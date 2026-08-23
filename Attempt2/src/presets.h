#pragma once
#include "platform_gl.h"
#include <vector>
#include <memory>

#include "object.h"
#include "camera.h"
#include "atomscene.h"

// Canned scenes on the number row. 1-9 load a scene for whichever mode is
// showing, 0 clears back to an empty sandbox.
namespace presets
{
	// Call once a frame. Returns true if a preset was loaded, which only matters
	// to anything that caches per-frame state about the object list.
	bool Update(GLFWwindow* window, float delta, bool quantum,
		std::vector<std::unique_ptr<Object>>& objects,
		Camera& camera, AtomScene& atom);

	// Load scene `index` outright; 0 clears back to an empty sandbox. This is
	// the same entry point the number row uses.
	void Load(int index, bool quantum,
		std::vector<std::unique_ptr<Object>>& objects,
		Camera& camera, AtomScene& atom);

	// True when the page is being viewed on a touch device. Fixed at startup.
	bool MobileMode();

	// The whole of mobile behaviour: no spawning, no sliders, no camera keys.
	// The scene advances one step per tap and the camera turns on its own.
	void UpdateMobile(GLFWwindow* window, float delta, bool& quantum,
		std::vector<std::unique_ptr<Object>>& objects,
		Camera& camera, AtomScene& atom);

	// Name of whatever is loaded, empty once the user has touched the scene.
	const char* CurrentName();

	// The user spawned or deleted something, so the label no longer describes
	// what is on screen.
	void Invalidate();
}
