#pragma once
#include "glm.hpp"
#include "gtc/type_ptr.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/quaternion.hpp"
#include "glew.h"
#include "GLFW/glfw3.h"
#include "timer.h"

// Orbit camera whose orientation is stored as a quaternion. Storing orientation as a quaternion and rotating about the camera's OWN axes removes the special case completely
class Camera
{
public:
	Camera(unsigned int program, int viewLoc, int projLoc)
		: m_program(program), m_viewLoc(viewLoc), m_projLoc(projLoc)
	{
	}
	~Camera() = default;

	// called each frame (stuff everything the camera needs to do here)
	void Update(GLFWwindow* window, int width, int height);

	// Everything below is derived from the quaternion
	glm::vec3 GetPosition()  const { return m_target + m_orientation * glm::vec3(0.0f, 0.0f, m_radius); }
	glm::vec3 GetTarget()    const { return m_target; }
	glm::vec3 GetUp()        const { return m_orientation * glm::vec3(0.0f, 1.0f, 0.0f); }
	glm::vec3 GetRight()     const { return m_orientation * glm::vec3(1.0f, 0.0f, 0.0f); }
	glm::vec3 GetDirection() const { return m_orientation * glm::vec3(0.0f, 0.0f, 1.0f); } // target -> camera
	float GetRadius()        const { return m_radius; }

	const glm::mat4& GetViewMatrix()       const { return m_view; }
	const glm::mat4& GetProjectionMatrix() const { return m_projection; }

	// Scrolling in reduces the radius. Clamped so it can no longer go negative and turn the view inside out.
	void IncRadius(float amount) { m_radius = glm::clamp(m_radius - amount, kMinRadius, kMaxRadius); }

private:
	void HandleInput(GLFWwindow* window);
	void UpdateViewMatrix();
	void UpdateProjectionMatrix(int width, int height);

	static constexpr float kMinRadius{ 0.2f };
	static constexpr float kMaxRadius{ 500.0f };
	static constexpr float kTurnSpeed{ 1.5f };   // radians per second
	static constexpr float kMaxFrameStep{ 0.1f };  // swallow alt-tab / breakpoint hitches

	// Identity orientation = sitting on +Z looking back at the target down -Z.
	glm::quat m_orientation{ glm::vec3(0.0f) };
	glm::vec3 m_target{ 0.0f, 0.0f, 0.0f };
	float m_radius{ 3.0f };

	glm::mat4 m_view{ 1.0f };
	glm::mat4 m_projection{ 1.0f };

	// The camera keeps its own clock. It used to call glfwSetTime(0) every frame, quietly resetting GLFW's global timer for the whole program.
	Timer m_inputTimer{};

	unsigned int m_program{ 0 }; // bound before uploading, so the uniforms land on the right program
	int m_viewLoc{ -1 };
	int m_projLoc{ -1 };
};
