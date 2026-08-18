#include "isosurface.h"
#include <algorithm>
#include <cmath>

namespace
{
	// A cube split into six tetrahedra sharing the 0-6 diagonal. Tetrahedra
	// have no ambiguous cases, which is why this is used instead of marching
	// cubes - the whole case table fits on a screen and can be checked by hand.
	const int kCorner[8][3] = {
		{0,0,0},{1,0,0},{1,1,0},{0,1,0},{0,0,1},{1,0,1},{1,1,1},{0,1,1}
	};

	const int kTet[6][4] = {
		{0,1,2,6},{0,2,3,6},{0,3,7,6},{0,7,4,6},{0,4,5,6},{0,5,1,6}
	};

	// Tetrahedron edges, as pairs of local vertex indices.
	const int kEdge[6][2] = { {0,1},{1,2},{2,0},{0,3},{1,3},{2,3} };

	// Which edges the surface crosses, per inside-mask. Bit v means vertex v is
	// inside. Entries come in groups of three, -1 ends the list.
	const int kCase[16][7] = {
		{-1,-1,-1,-1,-1,-1,-1},
		{ 0, 2, 3,-1,-1,-1,-1},
		{ 0, 4, 1,-1,-1,-1,-1},
		{ 2, 3, 4,  2, 4, 1,-1},
		{ 1, 5, 2,-1,-1,-1,-1},
		{ 0, 3, 5,  0, 5, 1,-1},
		{ 0, 4, 5,  0, 5, 2,-1},
		{ 3, 4, 5,-1,-1,-1,-1},
		{ 3, 5, 4,-1,-1,-1,-1},
		{ 0, 2, 5,  0, 5, 4,-1},
		{ 0, 3, 5,  0, 5, 1,-1},
		{ 1, 2, 5,-1,-1,-1,-1},
		{ 2, 4, 1,  2, 3, 4,-1},
		{ 0, 1, 4,-1,-1,-1,-1},
		{ 0, 3, 2,-1,-1,-1,-1},
		{-1,-1,-1,-1,-1,-1,-1}
	};
}

namespace iso
{

float LevelForFraction(const std::vector<float>& grid, float fraction)
{
	if (grid.empty())
		return 0.0f;

	std::vector<float> sorted(grid);
	std::sort(sorted.begin(), sorted.end(), std::greater<float>());

	double total = 0.0;
	for (size_t i = 0; i < sorted.size(); ++i)
		total += sorted[i];

	if (total <= 0.0)
		return 0.0f;

	// Cell volume is constant so it cancels out of the ratio.
	double want = total * fraction;
	double acc = 0.0;
	for (size_t i = 0; i < sorted.size(); ++i)
	{
		acc += sorted[i];
		if (acc < want)
			continue;

		// Never return a value that appears in the grid. A spherically
		// symmetric field puts thousands of samples at the identical value, and
		// landing exactly on one splits that tied set on floating point noise -
		// the surface comes out shredded into rings and fragments. Drop the
		// level into the gap below the crossing sample so the inside test is
		// unambiguous for every one of them.
		for (size_t j = i + 1; j < sorted.size(); ++j)
			if (sorted[j] < sorted[i])
				return 0.5f * (sorted[i] + sorted[j]);

		return sorted[i] * 0.999f;
	}

	return sorted.back() * 0.999f;
}

void March(const std::vector<float>& grid, int dim, float half, float level, std::vector<float>& out)
{
	if (dim < 2 || static_cast<int>(grid.size()) < dim * dim * dim)
		return;

	float step = 2.0f * half / (dim - 1);

	float val[8];
	int index[8];
	glm::vec3 pos[8];
	glm::vec3 grad[8];

	for (int k = 0; k < dim - 1; ++k)
	{
		for (int j = 0; j < dim - 1; ++j)
		{
			for (int i = 0; i < dim - 1; ++i)
			{
				for (int c = 0; c < 8; ++c)
				{
					int ci = i + kCorner[c][0];
					int cj = j + kCorner[c][1];
					int ck = k + kCorner[c][2];

					index[c] = ci + dim * (cj + dim * ck);
					val[c] = grid[index[c]];
					pos[c] = glm::vec3(-half + ci * step, -half + cj * step, -half + ck * step);

					int ip = std::min(ci + 1, dim - 1), im = std::max(ci - 1, 0);
					int jp = std::min(cj + 1, dim - 1), jm = std::max(cj - 1, 0);
					int kp = std::min(ck + 1, dim - 1), km = std::max(ck - 1, 0);

					grad[c] = glm::vec3(
						grid[ip + dim * (cj + dim * ck)] - grid[im + dim * (cj + dim * ck)],
						grid[ci + dim * (jp + dim * ck)] - grid[ci + dim * (jm + dim * ck)],
						grid[ci + dim * (cj + dim * kp)] - grid[ci + dim * (cj + dim * km)]);
				}

				for (int t = 0; t < 6; ++t)
				{
					int mask = 0;
					for (int v = 0; v < 4; ++v)
						if (val[kTet[t][v]] >= level)
							mask |= 1 << v;

					const int* list = kCase[mask];
					for (int e = 0; list[e] >= 0; ++e)
					{
						int a = kTet[t][kEdge[list[e]][0]];
						int b = kTet[t][kEdge[list[e]][1]];

						// Always walk an edge from its lower grid index to its
						// higher one. Neighbouring cells share edges but reach
						// them in opposite order, and the two interpolations
						// differ in the last bits - enough to leave hairline
						// seams. Fixing the direction makes them bit-identical.
						if (index[a] > index[b])
							std::swap(a, b);

						float fa = val[a];
						float fb = val[b];
						float f = (fb != fa) ? (level - fa) / (fb - fa) : 0.5f;
						f = std::min(std::max(f, 0.0f), 1.0f);

						glm::vec3 p = pos[a] + (pos[b] - pos[a]) * f;
						glm::vec3 g = grad[a] + (grad[b] - grad[a]) * f;

						float len = glm::length(g);
						// density falls off outwards, so the outward normal is -grad
						glm::vec3 nrm = (len > 1e-20f) ? (-g / len) : glm::vec3(0.0f, 0.0f, 1.0f);

						out.push_back(p.x); out.push_back(p.y); out.push_back(p.z);
						out.push_back(nrm.x); out.push_back(nrm.y); out.push_back(nrm.z);
					}
				}
			}
		}
	}
}

}
