#pragma once
#include <glew.h>
#include <GLFW/glfw3.h>
#include <vector>

#include "object.h"
#include "camera.h"
#include "shader.h"
#include "orbital.h"

// Quantum mode: build one atom by clicking. Left click adds a proton,
// shift + left adds a neutron, right click adds an electron. Every occupied
// orbital is drawn as a solid isosurface of |psi|^2, all centred on the nucleus.
class AtomScene
{
public:
	AtomScene(Shader& shader);
	~AtomScene();

	AtomScene(const AtomScene&) = delete;
	AtomScene& operator=(const AtomScene&) = delete;

	void Update(GLFWwindow* window, Camera& camera);
	void Render(Camera& camera);
	void PrintControls();

private:
	void Rebuild(Camera& camera);
	void BuildSurfaces();
	void FrameCamera(Camera& camera, bool force);
	int ResolvedView() const;
	void Report();

	static const int kGridDim = 49;       // 48 cells per axis
	static const int kCoarseDim = 33;      // cheap first pass, only sizes the real one

	// m_view: which subshells get drawn.
	static const int kValence = -2;        // outermost only - the default
	static const int kAll = -1;            // every occupied subshell at once

	Shader* m_shader{ nullptr };
	Object m_nucleus;

	int m_protons{ 1 };
	int m_neutrons{ 0 };
	int m_electrons{ 1 };
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
