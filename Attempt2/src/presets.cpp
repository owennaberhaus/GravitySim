#include "presets.h"

#include <cmath>
#include <gtc/quaternion.hpp>

namespace
{
	// Must match kSoftening in object.cpp 
	const float kSoft = 0.05f;
 
	float CircularSpeed(float mu, float r)
	{
		return std::sqrt(mu * r / (r * r + kSoft * kSoft));
	}

	// Mass doubles as radius 
	const float kG = 3.3333333f; // with a 0.3 star this makes mu exactly 1
	const float kStarMass = 0.3f;

	const char* s_name = "";
	int s_current = -1;
	float s_age = 0.0f;
	float s_resetAfter = 0.0f;
	bool s_down[10] = { false };

	// Mobile turntable
	float s_elevation = 0.0f;
	float s_yaw = 0.0f;

	int s_step = -1; // -1 means nothing shown yet
	float s_tapLock = 0.0f;
	const float kTapCooldown = 1.0f; // a fast double tap must not skip a scene
	const float kSpinDegrees = 9.0f; // a full turn every forty seconds
	const int kSteps = 18; // nine gravity scenes interleaved with nine atoms

	void Add(std::vector<std::unique_ptr<Object>>& v, glm::vec3 p, glm::vec3 vel,
		float mass, bool movable = true, bool gravitates = true)
	{
		v.push_back(std::make_unique<Object>(p.x, p.y, p.z, vel.x, vel.y, vel.z,
			mass, movable, glm::vec3(0.5f, 0.5f, 1.0f), gravitates));
	}

	// Park the camera at a distance
	void View(Camera& camera, float distance, float elevation)
	{
		s_elevation = elevation;
		s_yaw = 0.0f;
		camera.SetRadius(distance);
		camera.SetOrientation(glm::angleAxis(glm::radians(-elevation), glm::vec3(1.0f, 0.0f, 0.0f)));
	}

	// A body on a circular orbit at radius r, phase theta, in a plane tilted by
	// tilt degrees about the x axis.
	void Orbiter(std::vector<std::unique_ptr<Object>>& v, float mu, float r,
		float theta, float tilt, float mass, bool gravitates = false)
	{
		float speed = CircularSpeed(mu, r);
		glm::vec3 p(r * std::cos(theta), r * std::sin(theta), 0.0f);
		glm::vec3 vel(-speed * std::sin(theta), speed * std::cos(theta), 0.0f);

		glm::quat lean = glm::angleAxis(glm::radians(tilt), glm::vec3(1.0f, 0.0f, 0.0f));
		Add(v, lean * p, lean * vel, mass, true, gravitates);
	}

	typedef std::vector<std::unique_ptr<Object>> Bodies;

	// 1 - the Chenciner-Montgomery figure eight
	void FigureEight(Bodies& v, Camera& camera)
	{
		M_G = 20.0f;
		const float m = 0.05f;

		Add(v, glm::vec3(0.97000436f, -0.24308753f, 0.0f), glm::vec3(0.46620369f, 0.43236573f, 0.0f), m);
		Add(v, glm::vec3(-0.97000436f, 0.24308753f, 0.0f), glm::vec3(0.46620369f, 0.43236573f, 0.0f), m);
		Add(v, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-0.93240737f, -0.86473146f, 0.0f), m);

		View(camera, 3.2f, 0.0f);

		// The eight is an unstable periodic orbit
		s_resetAfter = 45.0f;
	}

	// 2 - a star and five planets on circular orbits
	void SolarSystem(Bodies& v, Camera& camera)
	{
		M_G = kG;
		Add(v, glm::vec3(0.0f), glm::vec3(0.0f), kStarMass, false);

		const float mu = M_G * kStarMass;
		Orbiter(v, mu, 1.0f, 0.0f, 0.0f, 0.05f);
		Orbiter(v, mu, 1.5f, 2.1f, 0.0f, 0.06f);
		Orbiter(v, mu, 2.1f, 4.0f, 14.0f, 0.05f);
		Orbiter(v, mu, 2.9f, 1.0f, 0.0f, 0.07f);
		Orbiter(v, mu, 3.8f, 3.4f, -19.0f, 0.05f);

		View(camera, 9.5f, 28.0f);
	}

	// 3 - two stars orbiting their common centre with a planet far enough out that it sees them as a single mass.
	void Binary(Bodies& v, Camera& camera)
	{
		M_G = kG;
		const float m = 0.2f;
		const float sep = 1.0f;
		const float mu = M_G * m;

		// Equal masses a distance d apart circle the barycentre at d/2
		float v0 = std::sqrt(mu * (sep * 0.5f) / (sep * sep + kSoft * kSoft));

		Add(v, glm::vec3(-sep * 0.5f, 0.0f, 0.0f), glm::vec3(0.0f, -v0, 0.0f), m);
		Add(v, glm::vec3(sep * 0.5f, 0.0f, 0.0f), glm::vec3(0.0f, v0, 0.0f), m);

		Orbiter(v, 2.0f * mu, 3.0f, 0.0f, 8.0f, 0.06f);

		View(camera, 8.0f, 26.0f);
	}

	// 4 - nested timescales. The moon laps the earth roughly nine times a year.
	void SunEarthMoon(Bodies& v, Camera& camera)
	{
		M_G = kG;
		Add(v, glm::vec3(0.0f), glm::vec3(0.0f), kStarMass, false);

		const float muSun = M_G * kStarMass;
		const float earthMass = 0.08f;
		const float earthR = 2.0f;
		float earthV = CircularSpeed(muSun, earthR);

		Add(v, glm::vec3(earthR, 0.0f, 0.0f), glm::vec3(0.0f, earthV, 0.0f), earthMass);

		// Well inside the Hill radius, which is earthR * cbrt(earthMass / 3*star) = 0.89 here
		const float moonR = 0.3f;
		float moonV = CircularSpeed(M_G * earthMass, moonR);
		Add(v, glm::vec3(earthR + moonR, 0.0f, 0.0f), glm::vec3(0.0f, earthV + moonV, 0.0f), 0.025f, true, false);

		View(camera, 6.5f, 24.0f);
	}

	// 5 - eccentric orbits
	void Comets(Bodies& v, Camera& camera)
	{
		M_G = kG;
		Add(v, glm::vec3(0.0f), glm::vec3(0.0f), kStarMass, false);

		const float mu = M_G * kStarMass;
		const float a[3] = { 2.5f, 1.8f, 3.2f };
		const float e[3] = { 0.70f, 0.55f, 0.80f };
		const float phase[3] = { 0.0f, 2.0f, 4.2f };
		const float tilt[3] = { 0.0f, 24.0f, -18.0f };

		for (int i = 0; i < 3; ++i)
		{
			float rA = a[i] * (1.0f + e[i]);
			float speed = std::sqrt(mu * (2.0f / rA - 1.0f / a[i]));

			glm::vec3 p(rA * std::cos(phase[i]), rA * std::sin(phase[i]), 0.0f);
			glm::vec3 vel(-speed * std::sin(phase[i]), speed * std::cos(phase[i]), 0.0f);

			glm::quat lean = glm::angleAxis(glm::radians(tilt[i]), glm::vec3(1.0f, 0.0f, 0.0f));
			Add(v, lean * p, lean * vel, 0.035f, true, false);
		}

		View(camera, 13.0f, 22.0f);
	}

	// 6 - an S-type planet
	void BinaryPlanet(Bodies& v, Camera& camera)
	{
		M_G = kG;
		const float m = 0.2f;
		const float sep = 4.0f;
		const float mu = M_G * m;

		float vOut = std::sqrt(mu * (sep * 0.5f) / (sep * sep + kSoft * kSoft));
		float cx = sep * 0.5f;

		Add(v, glm::vec3(-cx, 0.0f, 0.0f), glm::vec3(0.0f, -vOut, 0.0f), m);
		Add(v, glm::vec3(cx, 0.0f, 0.0f), glm::vec3(0.0f, vOut, 0.0f), m);

		const float rP = 0.6f;
		float vP = CircularSpeed(mu, rP);
		Add(v, glm::vec3(-cx + rP, 0.0f, 0.0f), glm::vec3(0.0f, vP - vOut, 0.0f), 0.05f, true, false);

		View(camera, 10.0f, 24.0f);
	}

	// 7 - two tight pairs orbiting each other.
	void DoubleBinary(Bodies& v, Camera& camera)
	{
		M_G = kG;
		const float m = 0.1f;
		const float inner = 0.5f;
		const float outer = 4.0f;

		float vIn = std::sqrt((M_G * m) * (inner * 0.5f) / (inner * inner + kSoft * kSoft));
		float vOut = std::sqrt((M_G * 2.0f * m) * (outer * 0.5f) / (outer * outer + kSoft * kSoft));

		const float side[2] = { -1.0f, 1.0f };
		for (int s = 0; s < 2; ++s)
		{
			float cx = side[s] * outer * 0.5f;
			float drift = side[s] * vOut;
			Add(v, glm::vec3(cx, inner * 0.5f, 0.0f), glm::vec3(-vIn, drift, 0.0f), m);
			Add(v, glm::vec3(cx, -inner * 0.5f, 0.0f), glm::vec3(vIn, drift, 0.0f), m);
		}

		View(camera, 11.0f, 25.0f);
	}

	// 8 - twelve bodies on a shell
	void Collapse(Bodies& v, Camera& camera)
	{
		M_G = kG;
		const int count = 10;
		const float R = 2.2f;

		// Circular speed at the shell edge is sqrt(mu_total / R) = 1.10 here
		const float spin = 0.75f;

		for (int i = 0; i < count; ++i)
		{
			// Fibonacci sphere
			float z = 1.0f - (2.0f * i + 1.0f) / count;
			float rho = std::sqrt(std::max(0.0f, 1.0f - z * z));
			float phi = i * 2.3999632f;   // golden angle in radians

			glm::vec3 dir(rho * std::cos(phi), rho * std::sin(phi), z);
			glm::vec3 p = dir * R;

			// Tangential kick about the y axis.
			glm::vec3 tangent = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), dir);
			float len = glm::length(tangent);
			glm::vec3 vel = (len > 1e-4f) ? (tangent / len) * spin : glm::vec3(0.0f);

			Add(v, p, vel, 0.08f);
		}

		View(camera, 8.5f, 18.0f);

		// same as earlier
		s_resetAfter = 45.0f;
	}

	// 9 - a disc of test particles
	void Ring(Bodies& v, Camera& camera)
	{
		M_G = kG;
		Add(v, glm::vec3(0.0f), glm::vec3(0.0f), kStarMass, false);

		const float mu = M_G * kStarMass;
		const int count = 12;

		for (int i = 0; i < count; ++i)
		{
			float t = static_cast<float>(i) / (count - 1);
			float r = 1.2f + t * 1.4f;
			float theta = i * 2.3999632f;
			float lift = ((i % 3) - 1) * 0.04f;

			float speed = CircularSpeed(mu, r);
			Add(v, glm::vec3(r * std::cos(theta), r * std::sin(theta), lift),
				glm::vec3(-speed * std::sin(theta), speed * std::cos(theta), 0.0f),
				0.03f, true, false);
		}

		View(camera, 7.0f, 30.0f);
	}

	const char* kGravityNames[10] = {
		"sandbox", "figure eight", "solar system", "binary star", "earth and moon",
		"comets", "planet in a binary", "double binary", "cluster collapse", "accretion ring"
	};

	// Chosen so each one shows a different shape
	const int kAtomZ[10] = { 1, 1, 6, 7, 10, 26, 29, 36, 64, 92 };
	const char* kAtomNames[10] = {
		"hydrogen", "hydrogen", "carbon", "nitrogen", "neon",
		"iron", "copper", "krypton", "gadolinium", "uranium"
	};

}

namespace presets
{

void Load(int index, bool quantum, std::vector<std::unique_ptr<Object>>& objects, Camera& camera, AtomScene& atom)
{
	s_age = 0.0f;
	s_resetAfter = 0.0f;
	s_current = index;

	if (quantum)
	{
		atom.SetElement(kAtomZ[index]);
		s_name = kAtomNames[index];
		// AtomScene refits the distance itself, so only the tilt is set here.
		s_elevation = 18.0f;
		s_yaw = 0.0f;
		return;
	}

	objects.clear();
	s_name = kGravityNames[index];

	switch (index)
	{
	case 1: FigureEight(objects, camera); break;
	case 2: SolarSystem(objects, camera); break;
	case 3: Binary(objects, camera); break;
	case 4: SunEarthMoon(objects, camera); break;
	case 5: Comets(objects, camera); break;
	case 6: BinaryPlanet(objects, camera); break;
	case 7: DoubleBinary(objects, camera); break;
	case 8: Collapse(objects, camera); break;
	case 9: Ring(objects, camera); break;
	default:
		M_G = 0.5f;                  // the value object.cpp starts at
		View(camera, 3.0f, 0.0f);
		break;
	}
}

bool Update(GLFWwindow* window, float delta, bool quantum,
	std::vector<std::unique_ptr<Object>>& objects, Camera& camera, AtomScene& atom)
{
	s_age += delta;

	for (int k = 0; k <= 9; ++k)
	{
		bool down = glfwGetKey(window, GLFW_KEY_0 + k) == GLFW_PRESS;
		bool edge = down && !s_down[k];
		s_down[k] = down;

		if (edge)
		{
			Load(k, quantum, objects, camera, atom);
			return true;
		}
	}

	if (!quantum && s_resetAfter > 0.0f && s_age >= s_resetAfter && s_current > 0)
	{
		Load(s_current, false, objects, camera, atom);
		return true;
	}

	return false;
}

bool MobileMode()
{
#ifdef __EMSCRIPTEN__
	// Asked once and cached
	static int cached = -1;
	if (cached < 0)
		cached = EM_ASM_INT({ return (Module.gsMobile | 0); }) ? 1 : 0;
	return cached != 0;
#else
	return false;
#endif
}

void UpdateMobile(GLFWwindow* window, float delta, bool& quantum,
	std::vector<std::unique_ptr<Object>>& objects, Camera& camera, AtomScene& atom)
{
	(void)window;
	s_age += delta;
	if (s_tapLock > 0.0f)
		s_tapLock -= delta;

	// Taps are counted in JS and drained
	int taps = 0;
#ifdef __EMSCRIPTEN__
	taps = EM_ASM_INT({
		var n = Module.gsTaps | 0;
		Module.gsTaps = 0;
		return n;
	});
#endif

	if (s_step < 0 || (taps > 0 && s_tapLock <= 0.0f))
	{
		s_step = (s_step + 1) % kSteps;

		// Even steps are a gravity scene
		quantum = (s_step % 2) != 0;
		Load(s_step / 2 + 1, quantum, objects, camera, atom);
		s_tapLock = kTapCooldown;
	}
	else if (!quantum && s_resetAfter > 0.0f && s_age >= s_resetAfter && s_current > 0)
	{
		// The figure eight and the cluster wear out
		float keep = s_yaw;
		Load(s_current, false, objects, camera, atom);
		s_yaw = keep;
	}

	s_yaw += glm::radians(kSpinDegrees) * delta;
	camera.SetOrientation(glm::angleAxis(s_yaw, glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::angleAxis(glm::radians(-s_elevation), glm::vec3(1.0f, 0.0f, 0.0f)));
}

const char* CurrentName()
{
	return s_name;
}

void Invalidate()
{
	s_name = "";
	s_resetAfter = 0.0f;
	s_current = -1;
}

}
