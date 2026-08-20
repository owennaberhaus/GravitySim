#pragma once
#include <glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <string>

#include "object.h"
#include "camera.h"
#include "shader.h"
#include "orbital.h"
#include "picking.h"

// Quantum mode: one neutral atom. Left click steps up the periodic table,
// right click steps back down, shift jumps ten at a time. Every occupied
// orbital is drawn as a solid isosurface of |psi|^2, all centred on the nucleus.
class AtomScene
{
public:
	AtomScene(Shader& shader);
	~AtomScene();

	AtomScene(const AtomScene&) = delete;
	AtomScene& operator=(const AtomScene&) = delete;

	void Update(GLFWwindow* window, Camera& camera);
	void UpdateHover(GLFWwindow* window, double mouseX, double mouseY, Camera& camera);
	void Render(Camera& camera);
	void PrintControls();

	// Same information Report() prints, for the on-screen HUD.
	const std::vector<std::string>& HudLines() const { return m_hudLines; }

	// "2p  (px orbital)" for whatever the cursor is over, empty if nothing.
	const std::string& SelectedLabel() const { return m_selectedLabel; }

private:
	void Rebuild(Camera& camera);
	void BuildSurfaces();
	void FrameCamera(Camera& camera, bool force);
	int GridDimFor(const orbital::RadialTable& table, int l, int m, float level, float tight) const;
	int ResolvedView() const;
	void Report();
	float NucleusRadius() const;
	int Neutrons() const;

	// One drawn orbital: which triangles are its, and what to call it.
	struct Piece
	{
		int firstVertex;
		int vertexCount;
		std::string label;
	};

	static const int kGridDim = 49;        // 48 cells per axis, the usual case
	static const int kMaxGridDim = 97;     // ceiling for orbitals with thin shells
	static const int kCoarseDim = 33;      // cheap first pass, only sizes the real one

	// m_view: which subshells get drawn.
	static const int kValence = -2;        // outermost only - the default
	static const int kAll = -1;            // every occupied subshell at once

	Shader* m_shader{ nullptr };
	Object m_nucleus;

	// One knob. The atom is always a neutral element: electrons match protons
	// and the neutron count comes from the isotope table below.
	int m_element{ 1 };
	int m_view{ kValence };

	// Nucleus size is a fraction of the cloud's own size, not an absolute
	// length. Real nuclei are ~1e-5 of the atom at every Z, so a fixed world
	// size would swell past the cloud as the cloud contracts.
	float m_nucleusFraction{ 0.03f };

	// Fraction of the probability each surface encloses. The textbook value is
	// 0.90, but that surface is fat enough that neighbouring lobes merge into a
	// ball - a filled p shell comes out spherical. Lower means slimmer, better
	// separated lobes.
	float m_isoFraction{ 0.30f };

	float m_cloudExtent{ 1.0f };
	bool m_framedOnce{ false };
	bool m_dirty{ true };

	std::vector<orbital::Subshell> m_config;
	std::vector<std::string> m_hudLines;

	// The surface is kept on the CPU as well so the cursor can be cast against
	// the actual triangles rather than a stand-in shape.
	std::vector<float> m_surfaceData;
	std::vector<Piece> m_pieces;
	static const int kNucleus = -2;   // m_hovered sentinel
	int m_hovered{ -1 };
	std::string m_selectedLabel;

	GLuint m_surfaceVAO{ 0 };
	GLuint m_surfaceVBO{ 0 };
	GLsizei m_surfaceVerts{ 0 };

	bool m_leftWasDown{ false };
	bool m_rightWasDown{ false };
	bool m_isolateWasDown{ false };
	bool m_clearWasDown{ false };
	bool m_growWasDown{ false };
	bool m_shrinkWasDown{ false };
	bool m_refitWasDown{ false };
	bool m_thinWasDown{ false };
	bool m_fatWasDown{ false };
};
