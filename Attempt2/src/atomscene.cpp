#include "atomscene.h"
#include "isosurface.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <limits>

extern float g_scrollDelta; // same scroll accumulator the gravity mode drains

namespace
{
	const glm::vec3 kColor(0.45f, 0.60f, 1.00f);

	// Mass number of the isotope to show. Measured values up to calcium past that, the most stable isobar from the semi-empirical mass formula, Z(A) = A / (1.98 + 0.0155 * A^(2/3))
	int MassNumber(int z)
	{
		static const int kMeasured[] = {
			0, 1, 4, 7, 9, 11, 12, 14, 16, 19, 20,
			23, 24, 27, 28, 31, 32, 35, 40, 39, 40
		};
		if (z <= 0)
			return 0;
		if (z < static_cast<int>(sizeof(kMeasured) / sizeof(kMeasured[0])))
			return kMeasured[z];

		double lo = z, hi = 3.0 * z + 4.0;
		for (int i = 0; i < 60; ++i)
		{
			double a = 0.5 * (lo + hi);
			if (a / (1.98 + 0.0155 * std::cbrt(a * a)) < z) lo = a; else hi = a;
		}
		return static_cast<int>(std::lround(0.5 * (lo + hi)));
	}

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
	// Same interleaved position and normal layout the object mesh uses
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
		<< "left click steps up the periodic table, right click steps back down\n"
		<< "hold shift to jump ten elements at a time\n"
		<< "  (neutral atom, filled in aufbau order with Pauli and Hund)\n"
		<< "'I' cycles the view: valence shell -> all shells -> one shell at a time\n"
		<< "'C' clears back to hydrogen\n"
		<< "'-' and '=' change how far the nucleus is exaggerated\n"
		<< "'[' and ']' thin or fatten the orbital surfaces\n"
		<< "scroll to zoom, 'F' refits the view to the atom\n"
		<< "tab returns to the gravity sim\n";
}

void AtomScene::Update(GLFWwindow* window, Camera& camera)
{
	// Scroll to zoom
	if (g_scrollDelta != 0.0f)
	{
		camera.IncRadius(g_scrollDelta);
		g_scrollDelta = 0.0f;
	}

	bool shift = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
		glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;

	int step = shift ? 10 : 1;

	bool leftDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
	if (leftDown && !m_leftWasDown)
	{
		m_element = std::min(m_element + step, 118);
		m_dirty = true;
	}
	m_leftWasDown = leftDown;

	bool rightDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
	if (rightDown && !m_rightWasDown)
	{
		m_element = std::max(m_element - step, 1);
		m_dirty = true;
	}
	m_rightWasDown = rightDown;

	// valence then all then first subshell ... then last then valence
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
		m_element = 1;
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

// Jump straight to an element. Same effect as the clear-then-click path, and
// it releases the one-shot auto-framing so the new cloud gets refitted.
void AtomScene::SetElement(int z)
{
	m_element = std::max(1, std::min(z, 118));
	m_view = kValence;
	m_framedOnce = false;
	m_dirty = true;
}

void AtomScene::Refresh(Camera& camera)
{
	if (m_dirty)
		Rebuild(camera);
}

void AtomScene::Rebuild(Camera& camera)
{
	m_config = orbital::FillAufbau(m_element);

	if (m_view >= static_cast<int>(m_config.size()))
		m_view = kValence;

	BuildSurfaces();
	FrameCamera(camera, false);
	Report();

	m_dirty = false;
}

void AtomScene::BuildSurfaces()
{
	std::vector<float>& verts = m_surfaceData;
	verts.clear();
	m_pieces.clear();
	m_hovered = -1;
	m_selectedLabel.clear();

	std::vector<float> grid;
	std::vector<float> coarse(kCoarseDim * kCoarseDim * kCoarseDim);

	m_cloudExtent = 0.0f;
	int view = ResolvedView();

	for (size_t i = 0; i < m_config.size(); ++i)
	{
		if (view != kAll && static_cast<int>(i) != view)
			continue;

		const orbital::Subshell& sh = m_config[i];
		float zeff = orbital::EffectiveCharge(m_config, m_element, sh.n, sh.l);

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

			// Pass one, coarse, over the full 99.5% box. Only used to find the isolevel and how far out the surface actually reaches.
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

			// Pass two, full resolution over just the part that holds the surface
			float tight = std::min(half, surfaceR * 1.12f);

			// An orbital with radial nodes is not one surface but several nested shells.
			int dim = std::min(GridDimFor(table, sh.l, m, level, tight), static_cast<int>(kMaxGridDim));
			float tightStep = 2.0f * tight / (dim - 1);

			grid.assign(static_cast<size_t>(dim) * dim * dim, 0.0f);
			for (int gz = 0; gz < dim; ++gz)
				for (int gy = 0; gy < dim; ++gy)
					for (int gx = 0; gx < dim; ++gx)
					{
						glm::vec3 p(-tight + gx * tightStep, -tight + gy * tightStep, -tight + gz * tightStep);
						grid[gx + dim * (gy + dim * gz)] = orbital::Density(table, sh.l, m, p);
					}

			int before = static_cast<int>(verts.size() / 6);
			iso::March(grid, dim, tight, level, verts);
			int after = static_cast<int>(verts.size() / 6);

			if (after > before)
			{
				Piece piece;
				piece.firstVertex = before;
				piece.vertexCount = after - before;
				piece.label = std::to_string(sh.n) + "spdf"[sh.l];
				m_pieces.push_back(piece);
			}
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

int AtomScene::Neutrons() const
{
	return std::max(MassNumber(m_element) - m_element, 0);
}

float AtomScene::NucleusRadius() const
{
	int total = m_element + Neutrons();
	if (total <= 0)
		return 0.0f;
	return m_cloudExtent * m_nucleusFraction * std::cbrt(static_cast<float>(total));
}

void AtomScene::UpdateHover(GLFWwindow* window, double mouseX, double mouseY, Camera& camera)
{
	m_hovered = -1;
	m_selectedLabel.clear();

	glm::vec3 origin = camera.GetPosition();
	glm::vec3 dir = picking::MouseRay(window, mouseX, mouseY,
		camera.GetProjectionMatrix(), camera.GetViewMatrix());

	// avoid hitting multiple surfaces at once 
	float nearest = std::numeric_limits<float>::max();

	// The nucleus really is a sphere, so it uses the sphere test unchanged.
	float nucleusR = NucleusRadius();
	if (nucleusR > 0.0f)
	{
		float t = picking::RaySphere(origin, dir, glm::vec3(0.0f), nucleusR);
		if (t >= 0.0f)
		{
			nearest = t;
			m_hovered = kNucleus;
		}
	}

	for (size_t p = 0; p < m_pieces.size(); ++p)
	{
		const Piece& piece = m_pieces[p];
		for (int v = 0; v + 2 < piece.vertexCount; v += 3)
		{
			const float* base = &m_surfaceData[(static_cast<size_t>(piece.firstVertex) + v) * 6];
			glm::vec3 a(base[0], base[1], base[2]);
			glm::vec3 b(base[6], base[7], base[8]);
			glm::vec3 c(base[12], base[13], base[14]);

			float t = picking::RayTriangle(origin, dir, a, b, c);
			if (t >= 0.0f && t < nearest)
			{
				nearest = t;
				m_hovered = static_cast<int>(p);
			}
		}
	}

	if (m_hovered == kNucleus)
		m_selectedLabel = "nucleus";
	else if (m_hovered >= 0)
		m_selectedLabel = m_pieces[m_hovered].label;
}

// How fine a grid this orbital needs
int AtomScene::GridDimFor(const orbital::RadialTable& table, int l, int m, float level, float tight) const
{
	float maxY2 = 0.0f;
	for (int a = 0; a <= 32; ++a)
	{
		float ct = -1.0f + 2.0f * a / 32.0f;
		float st = std::sqrt(std::max(0.0f, 1.0f - ct * ct));
		for (int b = 0; b < 64; ++b)
		{
			float ph = 6.2831853f * b / 64.0f;
			float y = orbital::RealHarmonic(l, m, glm::vec3(st * std::cos(ph), st * std::sin(ph), ct));
			maxY2 = std::max(maxY2, y * y);
		}
	}

	if (maxY2 <= 0.0f)
		return kGridDim;

	float radialThreshold = level / maxY2; // R^2 has to clear this to be inside

	const int kScan = 4096;
	float thinnest = 2.0f * tight;
	bool inside = false;
	float start = 0.0f;

	for (int i = 0; i <= kScan; ++i)
	{
		float r = tight * i / kScan;
		float rad = table.Lookup(r);
		bool now = rad * rad >= radialThreshold;

		if (now && !inside) { start = r; inside = true; }
		else if (!now && inside) { thinnest = std::min(thinnest, r - start); inside = false; }
	}
	if (inside)
		thinnest = std::min(thinnest, tight - start);

	if (thinnest <= 0.0f)
		return kMaxGridDim;

	// aim for three cells across the thinnest shell
	int wanted = static_cast<int>(std::ceil(2.0f * tight / (thinnest / 3.0f))) + 1;
	return std::max(wanted, static_cast<int>(kGridDim));
}

int AtomScene::ResolvedView() const
{
	if (m_view == kValence)
		return static_cast<int>(m_config.size()) - 1; // outermost occupied subshell
	return m_view;
}

void AtomScene::FrameCamera(Camera& camera, bool force)
{
	// Fit once on entry, then never again unless 'F' asks for it. 
	if (m_framedOnce && !force)
		return;

	float target = m_cloudExtent * 2.5f;
	camera.IncRadius(camera.GetRadius() - target); // no SetRadius, so subtract the difference
	m_framedOnce = true;
}

void AtomScene::Report()
{
	int neutrons = Neutrons();

	std::string head = std::string(orbital::ElementSymbol(m_element))
		+ "  Z=" + std::to_string(m_element)
		+ " N=" + std::to_string(neutrons)
		+ " A=" + std::to_string(m_element + neutrons);

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

	// R grows as A^(1/3), but measured against the cloud rather than in absolute units, so a contracting cloud can never be swallowed by its own nucleus.
	float nucleusR = NucleusRadius();
	if (nucleusR > 0.0f)
	{
		m_nucleus.SetPosition(glm::vec3(0.0f));
		m_nucleus.SetMass(nucleusR);
		m_nucleus.SetColor(m_hovered == kNucleus
			? glm::min(kColor * 1.8f + glm::vec3(0.35f), glm::vec3(1.0f))
			: kColor);
		m_nucleus.UpdateSize();
		m_nucleus.DrawObject(modelLoc, colorLoc);
	}

	if (m_surfaceVerts <= 0)
		return;

	glm::mat4 identity(1.0f);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(identity));
	glUniform3fv(colorLoc, 1, glm::value_ptr(kColor));

	glBindVertexArray(m_surfaceVAO);

	if (m_hovered < 0 || m_hovered >= static_cast<int>(m_pieces.size()))
	{
		glDrawArrays(GL_TRIANGLES, 0, m_surfaceVerts);
	}
	else
	{
		// Everything except the hovered orbital then that one brighter
		const Piece& piece = m_pieces[m_hovered];
		glm::vec3 lit = glm::min(kColor * 1.8f + glm::vec3(0.35f), glm::vec3(1.0f));

		if (piece.firstVertex > 0)
			glDrawArrays(GL_TRIANGLES, 0, piece.firstVertex);

		glUniform3fv(colorLoc, 1, glm::value_ptr(lit));
		glDrawArrays(GL_TRIANGLES, piece.firstVertex, piece.vertexCount);

		int after = piece.firstVertex + piece.vertexCount;
		if (after < m_surfaceVerts)
		{
			glUniform3fv(colorLoc, 1, glm::value_ptr(kColor));
			glDrawArrays(GL_TRIANGLES, after, m_surfaceVerts - after);
		}
	}

	glBindVertexArray(0);
}
