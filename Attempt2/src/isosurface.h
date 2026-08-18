#pragma once
#include <glm.hpp>
#include <vector>

// Turns a sampled scalar field into a triangle surface.
namespace iso
{
	// `grid` holds dim*dim*dim samples over the cube [-half, half]^3,
	// indexed as x + dim * (y + dim * z).

	// The level whose enclosed region holds `fraction` of the field's total.
	float LevelForFraction(const std::vector<float>& grid, float fraction);

	// Appends interleaved position + normal triangles, 6 floats per vertex.
	// Normals come from the field gradient rather than the triangle, so
	// neighbouring triangles agree at shared corners and the surface shades
	// smoothly with no vertex welding.
	void March(const std::vector<float>& grid, int dim, float half, float level, std::vector<float>& out);
}
