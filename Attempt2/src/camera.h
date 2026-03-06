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
	void UpdateProjectionMatrix(int width, int height);


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

	const glm::vec3 GetPosition() { return m_position; }
	const glm::vec3 GetTarget() { return m_target; }
	const glm::vec3 GetUp() { return m_cameraUp; }
	const float GetRadius() { return m_radius; }
	const glm::vec3 GetDirection() { return m_direction; }
	const glm::mat4& GetProjectionMatrix() const { return m_projection; }
	const glm::mat4& GetViewMatrix() const { return m_view; }
	
	void IncRadius(float amount) { m_radius > 0.01f ? m_radius -= amount : m_radius = 0.011f; }

	void catchMouseMovement(GLFWwindow* window);
	glm::vec3 GetPosition2() { return m_position2; }

private:
	glm::vec3 m_position = glm::vec3(0.0f, 0.0f, 5.0f);
	glm::vec3 m_position2{ 3.0f, 0.0f, 3.0f }; 
	glm::vec3 m_target = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 m_direction = glm::normalize(m_position - m_target);

	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 m_cameraRight = glm::normalize(glm::cross(up, m_direction));
	glm::vec3 m_cameraUp = glm::cross(m_direction, m_cameraRight);
	
	glm::mat4 m_view{ 1.0f };
	glm::mat4 m_projection{ 1.0f };

	float m_radius = 3.0f;
	float m_zx{ 0.0f }; // theta (poorly named, ik)
	float m_yz{ 0.0f }; // phi


	float m_speed{ 0.05f };
    int m_viewLoc{ -1 }; // storing locally to avoid passing every frame (no clue if this is more preformatn lol)

};