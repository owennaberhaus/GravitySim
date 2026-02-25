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

//void Camera::UpdateViewMatrix()
//{
//	glm::mat4 x_view = glm::lookAt(m_position, m_target, up); // create view matrix based on position and target
//
//	glm::mat4 view = glm::translate(glm::mat4(1.0f), m_position); // translate view matrix by negative of position
//	// (we dont move the camera, we move the world in the opposite direction)
//	glUniformMatrix4fv(m_viewLoc, 1, GL_FALSE, glm::value_ptr(x_view)); // send updated view matrix to shader
//
//
//}

void Camera::UpdateViewMatrix()
{
	glm::mat4 view = glm::lookAt(m_position, m_target, cameraUp);
	glUniformMatrix4fv(m_viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}

void Camera::catchMouseMovement(GLFWwindow* window)
{
	
}