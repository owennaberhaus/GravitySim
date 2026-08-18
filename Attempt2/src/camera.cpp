#include "camera.h"
#include <iostream>

// controls the camera's orientation based on user input
void Camera::HandleInput(GLFWwindow* window)
{
	float dt = m_inputTimer.delta();
	if (dt > kMaxFrameStep)
		dt = kMaxFrameStep;

	const float step = kTurnSpeed * dt;

    // "up" is always whatever the camera currently calls up, so there is no pole to cross

	// left / right orbit, about the camera's local Y
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		m_orientation = m_orientation * glm::angleAxis(step, glm::vec3(0.0f, 1.0f, 0.0f));

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		m_orientation = m_orientation * glm::angleAxis(-step, glm::vec3(0.0f, 1.0f, 0.0f));

	// no flip
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		m_orientation = m_orientation * glm::angleAxis(-step, glm::vec3(1.0f, 0.0f, 0.0f));

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		m_orientation = m_orientation * glm::angleAxis(step, glm::vec3(1.0f, 0.0f, 0.0f));

	// roll about the view axis 
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		m_orientation = m_orientation * glm::angleAxis(step, glm::vec3(0.0f, 0.0f, 1.0f));

	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		m_orientation = m_orientation * glm::angleAxis(-step, glm::vec3(0.0f, 0.0f, 1.0f));

	// snap back to level - handy once you've rolled somewhere strange
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
		m_orientation = glm::quat(glm::vec3(0.0f));

	// renormalising quaternian each frame
	m_orientation = glm::normalize(m_orientation);
}

void Camera::UpdateProjectionMatrix(int width, int height)
{
	if (width <= 0 || height <= 0)
		return; // minimised window

	float aspect = static_cast<float>(width) / static_cast<float>(height);

	m_projection = glm::perspective(
		glm::radians(45.0f),  // FOV
		aspect,
		0.1f, // near plane
		1000.0f // far plane
	);

	glUseProgram(m_program);
	glUniformMatrix4fv(m_projLoc, 1, GL_FALSE, glm::value_ptr(m_projection));
}

void Camera::UpdateViewMatrix()
{
	// Built straight from the quaternion
	m_view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -m_radius))
		* glm::mat4_cast(glm::conjugate(m_orientation))
		* glm::translate(glm::mat4(1.0f), -m_target);

	glUseProgram(m_program);
	glUniformMatrix4fv(m_viewLoc, 1, GL_FALSE, glm::value_ptr(m_view));
}

void Camera::Update(GLFWwindow* window, int width, int height)
{
	HandleInput(window);
	UpdateProjectionMatrix(width, height);
	UpdateViewMatrix();
}
