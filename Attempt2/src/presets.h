#pragma once
#include "platform_gl.h"
#include <vector>
#include <memory>

#include "object.h"
#include "camera.h"
#include "atomscene.h"

// Canned scenes on the number row. 1-9 load scenes 0 resets
namespace presets
{
	bool Update(GLFWwindow* window, float delta, bool quantum,
		std::vector<std::unique_ptr<Object>>& objects,
		Camera& camera, AtomScene& atom);

	// Load scene index
	void Load(int index, bool quantum,
		std::vector<std::unique_ptr<Object>>& objects,
		Camera& camera, AtomScene& atom);

	// True when the page is being viewed on a touch device
	bool MobileMode();

	// The whole of mobile behaviour no spawning no sliders no camera keys
	void UpdateMobile(GLFWwindow* window, float delta, bool& quantum,
		std::vector<std::unique_ptr<Object>>& objects,
		Camera& camera, AtomScene& atom);

	// Name of whatever is loaded, empty once the user has touched the scene.
	const char* CurrentName();

	// The user spawned or deleted something, so the label no longer describes what is on screen.
	void Invalidate();
}
