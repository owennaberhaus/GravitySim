#include "atomscene.h"
#include "isosurface.h"
#include <iostream>
#include <cmath>
#include <algorithm>

extern float g_scrollDelta; // same scroll accumulator the gravity mode drains

namespace
{
	const glm::vec3 kColor(0.45f, 0.60f, 1.00f);

	bool KeyEdge(GLFWwindow* window, int key, bool& wasDown)
	{
		bool down = glfwGetKey(window, key) == GLFW_PRESS;
		bool edge = down && !wasDown;
		wasDown = down;
		return edge;
	}
}

AtomScene::AtomScene(Shader& shader)
	: m_shader(&shader),
	m_nucleus(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, false, kColor, false)
{
	// Same interleaved position + normal layout the Object mesh uses, so the
	// existing lit shader draws these with no changes at all.
	glGenVertexArrays(1, &m_surfaceVAO);
	glBindVertexArray(m_surfaceVAO);
	glGenBuffers(1, &m_surfaceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_surfaceVBO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

AtomScene::~AtomScene()
{
	glDeleteBuffers(1, &m_surfaceVBO);
	glDeleteVertexArrays(1, &m_surfaceVAO);
}

void AtomScene::PrintControls()
{
	std::cout << "\n-- quantum mode --\n"
		<< "left click adds a proton, shift + left click adds a neutron\n"
		<< "right click adds an electron, shift + right click adds ten\n"
		<< "  (aufbau order, Pauli and Hund respected)\n"
		<< "'I' cycles the view: valence shell -> all shells -> one shell at a time\n"
		<< "'C' clears back to hydrogen\n"
		<< "'-' and '=' change how far the nucleus is exaggerated\n"
		<< "'[' and ']' thin or fatten the orbital surfaces\n"
		<< "scroll to zoom, 'F' refits the view to the atom\n"
		<< "tab returns to the gravity sim\n";
}

void AtomScene::Update(GLFWwindow* window, Camera& camera)
{
	// Scroll to zoom, exactly as the gravity mode does it. This used to be
	// drained only by Clicker::MouseControl, which quantum mode never reaches,
	// so the wheel did nothing here and then dumped all at once on tab back.
	if (g_scrollDelta != 0.0f)
	{
		camera.IncRadius(g_scrollDelta);
		g_scrollDelta = 0.0f;
	}

	bool shift = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
		glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;

	bool leftDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
	if (leftDown && !m_leftWasDown)
	{
		if (shift)
		{
			if (m_neutrons < 200)
				++m_neutrons;
		}
		else if (m_protons < 118)
			++m_protons;
		m_dirty = true;
	}
	m_leftWasDown = leftDown;

	bool rightDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
	if (rightDown && !m_rightWasDown)
	{
		// 21 electrons before the first d orbital appears, so shift jumps ten
		int add = shift ? 10 : 1;
		m_electrons = std::min(m_electrons + add, 118);
		m_dirty = true;
	}
	m_rightWasDown = rightDown;

	// valence -> all -> first subshell -> ... -> last -> valence
	if (KeyEdge(window, GLFW_KEY_I, m_isolateWasDown))
	{
		if (m_view == kValence)       m_view = kAll;
		else if (m_view == kAll)      m_view = 0;
		else                          ++m_view;

		if (m_view >= static_cast<int>(m_config.size()))
			m_view = kValence;
		m_dirty = true;
	}

	if (KeyEdge(window, GLFW_KEY_C, m_clearWasDown))
	{
		m_protons = 1;
		m_neutrons = 0;
		m_electrons = 1;
		m_view = kValence;
		m_framedOnce = false;
		m_dirty = true;
	}

	// Refit the view to the atom, and hand framing back to the auto-fit.
	if (KeyEdge(window, GLFW_KEY_F, m_refitWasDown))
		FrameCamera(camera, true);

	if (KeyEdge(window, GLFW_KEY_LEFT_BRACKET, m_thinWasDown))
	{
		m_isoFraction = std::max(m_isoFraction - 0.05f, 0.10f);
		m_dirty = true;
	}
	if (KeyEdge(window, GLFW_KEY_RIGHT_BRACKET, m_fatWasDown))
	{
		m_isoFraction = std::min(m_isoFraction + 0.05f, 0.95f);
		m_dirty = true;
	}

	if (KeyEdge(window, GLFW_KEY_EQUAL, m_growWasDown))
		m_nucleusFraction = std::min(m_nucleusFraction * 1.5f, 0.5f);
	if (KeyEdge(window, GLFW_KEY_MINUS, m_shrinkWasDown))
		m_nucleusFraction = std::max(m_nucleusFraction / 1.5f, 0.002f);

	if (m_dirty)
		Rebuild(camera);
}

void AtomScene::Rebuild(Camera& camera)
{
	m_config = orbital::FillAufbau(m_electrons);

	if (m_view >= static_cast<int>(m_config.size()))
		m_view = kValence;

	BuildSurfaces();
	FrameCamera(camera, false);
	Report();

	m_dirty = false;
}

void AtomScene::BuildSurfaces()
{
	std::vector<float> verts;
	std::vector<float> grid(kGridDim * kGridDim * kGridDim);
	std::vector<float> coarse(kCoarseDim * kCoarseDim * kCoarseDim);

	m_cloudExtent = 0.0f;
	int view = ResolvedView();

	for (size_t i = 0; i < m_config.size(); ++i)
	{
		if (view != kAll && static_cast<int>(i) != view)
			continue;

		const orbital::Subshell& sh = m_config[i];
		float zeff = orbital::EffectiveCharge(m_config, m_protons, sh.n, sh.l);

		// One table per subshell - every m in it shares the same radial part.
		orbital::RadialTable table = orbital::MakeRadialTable(sh.n, sh.l, zeff);
		std::vector<int> occ = orbital::HundOccupancy(sh.l, sh.count);

		float half = table.rMax;
		m_cloudExtent = std::max(m_cloudExtent, half);

		for (int k = 0; k < static_cast<int>(occ.size()); ++k)
		{
			if (occ[k] <= 0)
				continue;

			int m = k - sh.l;

			// Pass one, coarse, over the full 99.5% box. Only used to find the
			// isolevel and how far out the surface actually reaches.
			float coarseStep = 2.0f * half / (kCoarseDim - 1);
			for (int gz = 0; gz < kCoarseDim; ++gz)
				for (int gy = 0; gy < kCoarseDim; ++gy)
					for (int gx = 0; gx < kCoarseDim; ++gx)
					{
						glm::vec3 p(-half + gx * coarseStep, -half + gy * coarseStep, -half + gz * coarseStep);
						coarse[gx + kCoarseDim * (gy + kCoarseDim * gz)] = orbital::Density(table, sh.l, m, p);
					}

			float level = iso::LevelForFraction(coarse, m_isoFraction);

			float surfaceR = 0.0f;
			for (int gz = 0; gz < kCoarseDim; ++gz)
				for (int gy = 0; gy < kCoarseDim; ++gy)
					for (int gx = 0; gx < kCoarseDim; ++gx)
						if (coarse[gx + kCoarseDim * (gy + kCoarseDim * gz)] >= level)
						{
							glm::vec3 p(-half + gx * coarseStep, -half + gy * coarseStep, -half + gz * coarseStep);
							surfaceR = std::max(surfaceR, glm::length(p));
						}

			if (surfaceR <= 0.0f)
				continue;

			// Pass two, full resolution over just the part that holds the
			// surface. The 99.5% box spends half its radius on empty tail, so
			// this buys 2-3x finer cells for free. The level is an absolute
			// density, so it carries straight over.
			float tight = std::min(half, surfaceR * 1.12f);
			float tightStep = 2.0f * tight / (kGridDim - 1);

			for (int gz = 0; gz < kGridDim; ++gz)
				for (int gy = 0; gy < kGridDim; ++gy)
					for (int gx = 0; gx < kGridDim; ++gx)
					{
						glm::vec3 p(-tight + gx * tightStep, -tight + gy * tightStep, -tight + gz * tightStep);
						grid[gx + kGridDim * (gy + kGridDim * gz)] = orbital::Density(table, sh.l, m, p);
					}

			iso::March(grid, kGridDim, tight, level, verts);
		}
	}

	if (m_cloudExtent <= 0.0f)
		m_cloudExtent = 1.0f; // no electrons, so nothing sets the scale

	m_surfaceVerts = static_cast<GLsizei>(verts.size() / 6);

	glBindBuffer(GL_ARRAY_BUFFER, m_surfaceVBO);
	glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float),
		verts.empty() ? nullptr : verts.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

int AtomScene::ResolvedView() const
{
	if (m_view == kValence)
		return static_cast<int>(m_config.size()) - 1; // outermost occupied subshell
	return m_view;
}

void AtomScene::FrameCamera(Camera& camera, bool force)
{
	// Fit once on entry, then never again unless 'F' asks for it. Re-fitting on
	// every rebuild rescaled the camera by exactly the factor the cloud
	// contracted, so adding a proton looked like it did nothing at all.
	if (m_framedOnce && !force)
		return;

	float target = m_cloudExtent * 2.5f;
	camera.IncRadius(camera.GetRadius() - target); // no SetRadius, so subtract the difference
	m_framedOnce = true;
}

void AtomScene::Report()
{
	int charge = m_protons - m_electrons;

	std::string head = std::string(orbital::ElementSymbol(m_protons))
		+ "  Z=" + std::to_string(m_protons)
		+ " N=" + std::to_string(m_neutrons)
		+ " A=" + std::to_string(m_protons + m_neutrons)
		+ "  charge=" + (charge > 0 ? "+" : "") + std::to_string(charge);

	std::string config = orbital::ConfigString(m_config);
	if (config.empty())
		config = "(no electrons)";

	std::string detail = "iso " + std::to_string(static_cast<int>(m_isoFraction * 100.0f)) + "%";
	int view = ResolvedView();
	if (view == kAll)
		detail += "   all shells";
	else if (view >= 0 && view < static_cast<int>(m_config.size()))
	{
		detail += "   showing ";
		detail += std::to_string(m_config[view].n);
		detail += "spdf"[m_config[view].l];
		detail += " only";
	}
	detail += "   " + std::to_string(m_surfaceVerts / 3) + " tris";

	m_hudLines.clear();
	m_hudLines.push_back(head);
	m_hudLines.push_back(config);
	m_hudLines.push_back(detail);

	std::cout << head << "  " << config << "  " << detail << '\n';
}

void AtomScene::Render(Camera& camera)
{
	glUseProgram(m_shader->GetShader());
	int modelLoc = m_shader->GetModelLoc();
	int colorLoc = m_shader->GetColorLoc();

	// R grows as A^(1/3), but measured against the cloud rather than in absolute
	// units, so a contracting cloud can never be swallowed by its own nucleus.
	int total = m_protons + m_neutrons;
	if (total > 0)
	{
		m_nucleus.SetPosition(glm::vec3(0.0f));
		m_nucleus.SetMass(m_cloudExtent * m_nucleusFraction * std::cbrt(static_cast<float>(total)));
		m_nucleus.SetColor(kColor);
		m_nucleus.UpdateSize();
		m_nucleus.DrawObject(modelLoc, colorLoc);
	}

	if (m_surfaceVerts <= 0)
		return;

	glm::mat4 identity(1.0f);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(identity));
	glUniform3fv(colorLoc, 1, glm::value_ptr(kColor));

	glBindVertexArray(m_surfaceVAO);
	glDrawArrays(GL_TRIANGLES, 0, m_surfaceVerts);
	glBindVertexArray(0);
}
