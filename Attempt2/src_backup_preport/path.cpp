#include "path.h"

Path::Path(float w, float x, float y, float z, glm::vec3 color) : vertexCount(0)
{
    m_width = w;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    m_color = color;
}

void Path::DrawPath(int modelLoc, int colorLoc, const glm::mat4& model)
{
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(colorLoc, 1, glm::value_ptr(m_color));

    glBindVertexArray(VAO);
    glLineWidth(2.0f);
    glDrawArrays(GL_LINE_STRIP, 0, vertexCount);
    glBindVertexArray(0);
}

void Path::UpdateVertices(float x, float y, float z)
{

    m_vertices.push_back(x);
    m_vertices.push_back(y);
    m_vertices.push_back(z);

    vertexCount = m_vertices.size() / 3;

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
        m_vertices.size() * sizeof(float),
        m_vertices.data(),
        GL_DYNAMIC_DRAW);

    if (m_vertices.size() > m_maxLength * 3) { // maxLength in vertices, not floats
        m_vertices.erase(m_vertices.begin(), m_vertices.begin() + 3);
    }
}