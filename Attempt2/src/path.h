#pragma once
#include <glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <matrix_transform.hpp>
#include <type_ptr.hpp>
#include <vector>
#include <iostream>
#include "path.h"

class Path
{
public:
    GLuint VAO;
    GLuint VBO;
    GLsizei vertexCount{};

    Path(float w = 0.005f, float x = 0.0f, float y = 0.0f, glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f));
    ~Path() {}
    void DrawPath(int modelLoc, int colorLoc);
    void UpdateVertices(float x, float y);

private:
    std::vector<float> m_vertices;
    float m_width{ 0.005f };
    glm::mat4 m_model{ 1.0f };
    glm::vec3 m_color;
    size_t m_maxLength{ 50000 };

};