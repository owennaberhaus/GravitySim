#pragma once
#include <string>
#include "glew.h"
#include "GLFW/glfw3.h"

class Shader
{
private:
    int CreateShader();
	int CreateShader2D();
	unsigned int CompileShader(unsigned int type, const std::string& source);
public:
    Shader();
	~Shader();

	const unsigned int GetShader() { return m_shader; }
	const unsigned int GetShader2D() { return m_shader2D; }
	const unsigned int GetViewLoc() { return m_viewLoc; }
	const unsigned int GetModelLoc() { return m_modelLoc; }
	const unsigned int GetColorLoc() { return m_colorLoc; }
	const unsigned int GetProjLoc() { return m_projLoc; }
	const unsigned int GetModelLoc2D() { return m_modelLoc2D; }
	const unsigned int GetColorLoc2D() { return m_colorLoc2D; }
	const unsigned int GetProjLoc2D() { return m_projLoc2D; }

private:
    const unsigned int m_shader{};
    const unsigned int m_shader2D{};
	const unsigned int m_modelLoc{}; // allows a translation matrix
	const unsigned int m_colorLoc{}; // accesses a glm::vec3 of rgb color
	const unsigned int m_projLoc{}; // basically allows window scaling
	const unsigned int m_viewLoc{}; // allows camera movement
	const unsigned int m_modelLoc2D{}; // allows a translation matrix
	const unsigned int m_colorLoc2D{}; // accesses a glm::vec3 of rgb color
	const unsigned int m_projLoc2D{}; // basically allows window scaling




};

