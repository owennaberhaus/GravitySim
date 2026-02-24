#pragma once
#include "glm.hpp"
#include "gtc/type_ptr.hpp"
#include "glew.h"
#include "GLFW/glfw3.h"

class Camera
{
private:
	void ShiftXY(float x, float y)
	{
		m_position.x += x;
		m_position.y += y;
	}

	void HandleInput(GLFWwindow* window);

	void UpdateViewMatrix();


public:

	Camera(int viewLocA)
		: m_viewLoc(viewLocA)
	{
	}
	~Camera() = default;

	void Update(GLFWwindow* window) // called each frame (stuff everything the camera needs to do here)
	{
		HandleInput(window);
		UpdateViewMatrix();
	}

	glm::vec3 GetPosition() { return m_position; }
	const float GetZoom() { return m_zoom; }

	void IncZoom(float amount) {
		m_zoom += amount / 25;
		if (m_zoom < 0.1f) m_zoom = 0.1f; // prevent zooming in too much
		if (m_zoom > 10.0f) m_zoom = 10.0f; // prevent zooming out too much
	}

private:
	glm::vec3 m_position{ 0.0f, 0.0f, 0.0f };
	// TODO
	float m_zoom{ 1.0f };
	float m_rotation{ 0.0f };

	float m_speed{ 0.05f };
    int m_viewLoc{ -1 }; // storing locally to avoid passing every frame (no clue if this is more preformatn lol)

};