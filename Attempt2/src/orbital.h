#pragma once
#include <glm.hpp>
#include <vector>
#include <string>

// Hydrogenic orbital math. Everything is in Bohr radii (a0 = 1).
namespace orbital
{
	struct Subshell
	{
		int n;
		int l;
		int count;
	};

	// R_nl tabulated on [0, rMax], so the density field can be evaluated with a lookup instead of running the Laguerre recurrence at every grid point.
	struct RadialTable
	{
		std::vector<float> value;
		float rMax{ 1.0f };
		float Lookup(float r) const;
	};

	// Fills `electrons` into subshells in Madelung (n+l, then n) order.
	std::vector<Subshell> FillAufbau(int electrons);

	// Hund's rule: singly occupy each m before pairing. Returns 2l+1 entries for m = -l .. +l.
	std::vector<int> HundOccupancy(int l, int count);

	// Slater's rules.
	float EffectiveCharge(const std::vector<Subshell>& config, int protons, int n, int l);

	float RadialWave(int n, int l, float zeff, float r);
	float RealHarmonic(int l, int m, const glm::vec3& dir);

	// <r> = (1 / 2Zeff) * (3n^2 - l(l+1))
	float MeanRadius(int n, int l, float zeff);

	// rMax is the radius holding 99.5% of the radial probability.
	RadialTable MakeRadialTable(int n, int l, float zeff);

	// |psi|^2 at a point, for the isosurface grid.
	float Density(const RadialTable& table, int l, int m, const glm::vec3& p);

	const char* ElementSymbol(int protons);
	std::string ConfigString(const std::vector<Subshell>& config);
}
