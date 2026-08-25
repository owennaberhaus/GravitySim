#pragma once
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <cmath>

// Mouse picking shared by both modes 
namespace picking
{
	// World-space direction of the ray under the cursor.
	inline glm::vec3 MouseRay(GLFWwindow* window, double mouseX, double mouseY,
		const glm::mat4& projection, const glm::mat4& view)
	{
		int winW{ 0 }, winH{ 0 };
		glfwGetWindowSize(window, &winW, &winH);
		if (winW <= 0 || winH <= 0)
			return glm::vec3(0.0f, 0.0f, -1.0f);

		// window pixels must be turned to normalized device coordinates
		float x = (2.0f * static_cast<float>(mouseX)) / static_cast<float>(winW) - 1.0f;
		float y = 1.0f - (2.0f * static_cast<float>(mouseY)) / static_cast<float>(winH);

		glm::vec4 rayClip(x, y, -1.0f, 1.0f);

		// undo the projection to get a direction in eye space, then the view to get it in world space
		glm::vec4 rayEye = glm::inverse(projection) * rayClip;
		rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

		return glm::normalize(glm::vec3(glm::inverse(view) * rayEye));
	}

	// Distance along the ray to the nearest intersection with the sphere, or a negative value if it never hits in front of the origin.
	inline float RaySphere(const glm::vec3& rayOrigin, const glm::vec3& rayDir,
		const glm::vec3& sphereCenter, float sphereRadius)
	{
		glm::vec3 oc = rayOrigin - sphereCenter; // origin to center

		// quadratic formula components for ray-sphere intersection
		float a = glm::dot(rayDir, rayDir);
		float b = 2.0f * glm::dot(oc, rayDir);
		float c = glm::dot(oc, oc) - sphereRadius * sphereRadius;

		float discriminant = b * b - 4.0f * a * c;
		if (discriminant < 0.0f)
			return -1.0f; // the line misses entirely

		float sqrtD = std::sqrt(discriminant);
		float t0 = (-b - sqrtD) / (2.0f * a); // near hit
		float t1 = (-b + sqrtD) / (2.0f * a); // far hit

		if (t0 > 0.0f) return t0; // normal case: entering the sphere
		if (t1 > 0.0f) return t1; // camera is inside the sphere
		return -1.0f; // wholly behind the camera
	}

	// Same contract as RaySphere for one triangle
	inline float RayTriangle(const glm::vec3& rayOrigin, const glm::vec3& rayDir,
		const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
	{
		const float kEpsilon = 1e-9f;

		glm::vec3 edge1 = b - a;
		glm::vec3 edge2 = c - a;
		glm::vec3 h = glm::cross(rayDir, edge2);
		float det = glm::dot(edge1, h);

		if (std::fabs(det) < kEpsilon)
			return -1.0f; // ray runs parallel to the triangle

		float invDet = 1.0f / det;
		glm::vec3 s = rayOrigin - a;

		float u = invDet * glm::dot(s, h);
		if (u < 0.0f || u > 1.0f)
			return -1.0f;

		glm::vec3 q = glm::cross(s, edge1);
		float v = invDet * glm::dot(rayDir, q);
		if (v < 0.0f || u + v > 1.0f)
			return -1.0f;

		float t = invDet * glm::dot(edge2, q);
		return (t > 0.0f) ? t : -1.0f;
	}
}
