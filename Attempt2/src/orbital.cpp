#include "orbital.h"
#include <cmath>
#include <algorithm>

namespace
{
	// Madelung order: sort by n+l, ties broken by n
	const int kOrder[][2] = {
		{1,0},{2,0},{2,1},{3,0},{3,1},{4,0},{3,2},{4,1},{5,0},{4,2},
		{5,1},{6,0},{4,3},{5,2},{6,1},{7,0},{5,3},{6,2},{7,1}
	};
	const int kOrderCount = sizeof(kOrder) / sizeof(kOrder[0]);

	double Factorial(int k)
	{
		double f = 1.0;
		for (int i = 2; i <= k; ++i)
			f *= i;
		return f;
	}

	// Generalized Laguerre L_k^alpha(x) by recurrence
	double Laguerre(int k, double alpha, double x)
	{
		double prev = 0.0;
		double cur = 1.0;
		for (int i = 0; i < k; ++i)
		{
			double next = ((2 * i + 1 + alpha - x) * cur - (i + alpha) * prev) / (i + 1);
			prev = cur;
			cur = next;
		}
		return cur;
	}

	// Slater group ordering: 1s | 2s2p | 3s3p | 3d | 4s4p | 4d | 4f |     Key is (n, 0) for s/p and (n, l) for d/f, compared lexicographically
	int GroupClass(int l) { return l <= 1 ? 0 : l; }

	bool SameGroup(int nA, int lA, int nB, int lB)
	{
		return nA == nB && GroupClass(lA) == GroupClass(lB);
	}

	bool LowerGroup(int nA, int lA, int nB, int lB)
	{
		if (nA != nB) return nA < nB;
		return GroupClass(lA) < GroupClass(lB);
	}

	const char* kSymbols[] = {
		"n", "H","He","Li","Be","B","C","N","O","F","Ne",
		"Na","Mg","Al","Si","P","S","Cl","Ar","K","Ca",
		"Sc","Ti","V","Cr","Mn","Fe","Co","Ni","Cu","Zn",
		"Ga","Ge","As","Se","Br","Kr","Rb","Sr","Y","Zr",
		"Nb","Mo","Tc","Ru","Rh","Pd","Ag","Cd","In","Sn",
		"Sb","Te","I","Xe","Cs","Ba","La","Ce","Pr","Nd",
		"Pm","Sm","Eu","Gd","Tb","Dy","Ho","Er","Tm","Yb",
		"Lu","Hf","Ta","W","Re","Os","Ir","Pt","Au","Hg",
		"Tl","Pb","Bi","Po","At","Rn","Fr","Ra","Ac","Th",
		"Pa","U","Np","Pu","Am","Cm","Bk","Cf","Es","Fm",
		"Md","No","Lr","Rf","Db","Sg","Bh","Hs","Mt","Ds",
		"Rg","Cn","Nh","Fl","Mc","Lv","Ts","Og"
	};
	const int kSymbolCount = sizeof(kSymbols) / sizeof(kSymbols[0]);

	const char* SubshellLetter(int l)
	{
		switch (l)
		{
		case 0: return "s";
		case 1: return "p";
		case 2: return "d";
		default: return "f";
		}
	}
}

namespace orbital
{

std::vector<Subshell> FillAufbau(int electrons)
{
	std::vector<Subshell> config;
	int left = electrons;

	for (int i = 0; i < kOrderCount && left > 0; ++i)
	{
		int n = kOrder[i][0];
		int l = kOrder[i][1];
		int capacity = 2 * (2 * l + 1);
		int put = std::min(left, capacity);

		config.push_back({ n, l, put });
		left -= put;
	}

	return config;
}

std::vector<int> HundOccupancy(int l, int count)
{
	int slots = 2 * l + 1;
	std::vector<int> occ(slots, 0);

	int singles = std::min(count, slots);
	int pairs = std::max(count - slots, 0);

	for (int i = 0; i < singles; ++i)
		occ[i] += 1;
	for (int i = 0; i < pairs; ++i)
		occ[i] += 1;

	return occ;
}

float EffectiveCharge(const std::vector<Subshell>& config, int protons, int n, int l)
{
	double s = 0.0;

	// A Slater group can span two subshells (2s2p, 3s3p, ...)
	int groupCount = 0;
	for (size_t i = 0; i < config.size(); ++i)
		if (SameGroup(config[i].n, config[i].l, n, l))
			groupCount += config[i].count;

	if (groupCount > 0)
		s += (groupCount - 1) * (n == 1 ? 0.30 : 0.35);

	for (size_t i = 0; i < config.size(); ++i)
	{
		const Subshell& sh = config[i];

		if (SameGroup(sh.n, sh.l, n, l))
		{
			continue;
		}
		else if (l <= 1)
		{
			if (sh.n == n - 1)
				s += sh.count * 0.85;
			else if (sh.n < n - 1)
				s += sh.count * 1.00;
		}
		else
		{
			if (LowerGroup(sh.n, sh.l, n, l))
				s += sh.count * 1.00;
		}
	}

	double zeff = protons - s;
	return static_cast<float>(std::max(zeff, 0.3));
}

float RadialWave(int n, int l, float zeff, float r)
{
	double z = zeff;
	double rho = 2.0 * z * r / n;

	double norm = std::sqrt(std::pow(2.0 * z / n, 3.0) * Factorial(n - l - 1) /
		(2.0 * n * Factorial(n + l)));

	double value = norm * std::exp(-rho / 2.0) * std::pow(rho, l) *
		Laguerre(n - l - 1, 2 * l + 1, rho);

	return static_cast<float>(value);
}

float RealHarmonic(int l, int m, const glm::vec3& d)
{
	float x = d.x, y = d.y, z = d.z;

	switch (l)
	{
	case 0:
		return 0.28209479f; // 1/2 sqrt(1/pi)

	case 1:
		switch (m)
		{
		case -1: return 0.48860251f * y;
		case  0: return 0.48860251f * z;
		default: return 0.48860251f * x;
		}

	case 2:
		switch (m)
		{
		case -2: return 1.09254843f * x * y;
		case -1: return 1.09254843f * y * z;
		case  0: return 0.31539157f * (3.0f * z * z - 1.0f);
		case  1: return 1.09254843f * x * z;
		default: return 0.54627421f * (x * x - y * y);
		}

	default:
		switch (m)
		{
		case -3: return 0.59004358f * y * (3.0f * x * x - y * y);
		case -2: return 2.89061144f * x * y * z;
		case -1: return 0.45704579f * y * (5.0f * z * z - 1.0f);
		case  0: return 0.37317633f * z * (5.0f * z * z - 3.0f);
		case  1: return 0.45704579f * x * (5.0f * z * z - 1.0f);
		case  2: return 1.44530572f * (x * x - y * y) * z;
		default: return 0.59004358f * x * (x * x - 3.0f * y * y);
		}
	}
}

float MeanRadius(int n, int l, float zeff)
{
	return (0.5f / zeff) * (3.0f * n * n - l * (l + 1));
}

float RadialTable::Lookup(float r) const
{
	if (r >= rMax || value.size() < 2)
		return 0.0f;

	float x = r / rMax * (value.size() - 1);
	int i = static_cast<int>(x);
	if (i >= static_cast<int>(value.size()) - 1)
		return value.back();

	float f = x - i;
	return value[i] + (value[i + 1] - value[i]) * f;
}

RadialTable MakeRadialTable(int n, int l, float zeff)
{
	const int kBins = 1024;
	RadialTable table;

	float rHi = 50.0f * n * n / zeff;
	float dr = rHi / kBins;

	std::vector<float> weight(kBins + 1, 0.0f);
	double total = 0.0;
	for (int i = 1; i <= kBins; ++i)
	{
		float r = dr * i;
		float rad = RadialWave(n, l, zeff, r);
		weight[i] = r * r * rad * rad;
		total += weight[i];
	}

	table.rMax = rHi;
	if (total > 0.0)
	{
		double want = total * 0.995;
		double acc = 0.0;
		for (int i = 1; i <= kBins; ++i)
		{
			acc += weight[i];
			if (acc >= want)
			{
				table.rMax = dr * i * 1.05f;
				break;
			}
		}
	}

	const int kSamples = 512;
	table.value.resize(kSamples);
	for (int i = 0; i < kSamples; ++i)
		table.value[i] = RadialWave(n, l, zeff, table.rMax * i / (kSamples - 1));

	return table;
}

float Density(const RadialTable& table, int l, int m, const glm::vec3& p)
{
	float r = glm::length(p);
	if (r >= table.rMax)
		return 0.0f;

	glm::vec3 dir = (r > 1e-6f) ? (p / r) : glm::vec3(0.0f, 0.0f, 1.0f);
	float psi = table.Lookup(r) * RealHarmonic(l, m, dir);
	return psi * psi;
}

const char* ElementSymbol(int protons)
{
	if (protons < 0 || protons >= kSymbolCount)
		return "?";
	return kSymbols[protons];
}

std::string ConfigString(const std::vector<Subshell>& config)
{
	std::string out;
	for (size_t i = 0; i < config.size(); ++i)
	{
		if (i > 0)
			out += " ";
		out += std::to_string(config[i].n);
		out += SubshellLetter(config[i].l);
		out += std::to_string(config[i].count);
	}
	return out;
}

}
