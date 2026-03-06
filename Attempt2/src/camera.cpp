#include "camera.h"
#include <iostream>

// controls the camera's position and view matrix based on user input
void Camera::HandleInput(GLFWwindow* window)
{

	float speed = 0.5f * glfwGetTime();
	glfwSetTime(0);  // reset timer so movement is frame-rate independent

	// left right movement
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		m_zx += speed;

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		m_zx -= speed;

	//up down movement
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		m_yz += speed;

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		m_yz -= speed;

	float yaw = m_zx;
	float pitch = m_yz;

	// Orbit around Y axis
	m_position.x = m_target.x + m_radius * cos(pitch) * sin(yaw);
	m_position.y = m_target.y + m_radius * sin(pitch);
	m_position.z = m_target.z + m_radius * cos(pitch) * cos(yaw);

}

void Camera::UpdateProjectionMatrix(int width, int height)
{
	float aspect = static_cast<float>(width) / static_cast<float>(height);

	m_projection = glm::perspective(
		glm::radians(45.0f),  // FOV
		aspect,
		0.1f,                 // near plane
		1000.0f               // far plane
	);
}

void Camera::UpdateViewMatrix()
{
	glm::vec3 C = m_position;
	glm::vec3 T = m_target;

	glm::vec3 forward = glm::normalize(T - C);
	glm::vec3 worldUp = glm::vec3(0, 1, 0);

	m_cameraRight = glm::normalize(glm::cross(forward, worldUp));
	m_cameraUp = glm::normalize(glm::cross(m_cameraRight, forward));

	m_view = glm::lookAt(m_position, m_target, m_cameraUp);
	glUniformMatrix4fv(m_viewLoc, 1, GL_FALSE, glm::value_ptr(m_view));
}

// TODO move movement to mouse input
void Camera::catchMouseMovement(GLFWwindow* window)
{
	
}