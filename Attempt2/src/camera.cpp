#include "camera.h"

// controls the camera's position and view matrix based on user input
void Camera::HandleInput(GLFWwindow* window)
{
	glm::vec2 dir(0.0f, 0.0f); // direction vector for movement (reset each frame)

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) dir.y += 1.0f;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) dir.y -= 1.0f;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) dir.x += 1.0f;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) dir.x -= 1.0f;

	if (dir.x != 0.0f || dir.y != 0.0f) // if the user input something
	{
		dir = glm::normalize(dir); // normalize so diagonals arent faster
		ShiftXY(dir.x * m_speed, dir.y * m_speed); // update position
	}

}

void Camera::UpdateViewMatrix()
{
	glm::mat4 view = glm::translate(glm::mat4(1.0f), -m_position); // translate view matrix by negative of position
	// (we dont move the camera, we move the world in the opposite direction)
	glUniformMatrix4fv(m_viewLoc, 1, GL_FALSE, glm::value_ptr(view)); // send updated view matrix to shader
}