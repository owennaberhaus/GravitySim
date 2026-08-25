#include "object.h"
#include "menu.h"

#include <cmath>
#include <algorithm>

float M_G{ 0.5f }; // gravitational constant - adjust as needed for visual effect

static constexpr float kSoftening{ 0.05f };


void Object::GenSphereMesh(int segments)
{
    const int stacks = segments;
    const int slices = segments;

    m_vertices.clear();
    m_indices.clear();
    m_vertices.reserve(static_cast<size_t>(stacks + 1) * (slices + 1) * 6);
    m_indices.reserve(static_cast<size_t>(stacks) * slices * 6);

    for (int i = 0; i <= stacks; ++i)
    {
        float phi = static_cast<float>(M_PI) * i / stacks;
        float sinPhi = std::sin(phi);
        float cosPhi = std::cos(phi);

        for (int j = 0; j <= slices; ++j)
        {
            float theta = 2.0f * static_cast<float>(M_PI) * j / slices;

            float x = sinPhi * std::cos(theta);
            float y = sinPhi * std::sin(theta);
            float z = cosPhi;

            m_vertices.insert(m_vertices.end(), { x, y, z, x, y, z });
        }
    }

    for (int i = 0; i < stacks; ++i)
    {
        for (int j = 0; j < slices; ++j)
        {
            GLuint topLeft     = static_cast<GLuint>(i * (slices + 1) + j);
            GLuint topRight    = topLeft + 1;
            GLuint bottomLeft  = topLeft + (slices + 1);
            GLuint bottomRight = bottomLeft + 1;

            m_indices.insert(m_indices.end(), {
                topLeft,  bottomLeft, topRight,
                topRight, bottomLeft, bottomRight
                });
        }
    }

    vertexCount = static_cast<GLsizei>(m_vertices.size() / 6);
    indexCount = static_cast<GLsizei>(m_indices.size());

    // generate and bind VAO // VBO // EBO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(float), m_vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(GLuint), m_indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(
        0,                 // location in shader
        3,                 // 3 floats per position (x,y,z)
        GL_FLOAT,
        GL_FALSE,
        6 * sizeof(float), // stride
        (void*)0
    );
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        6 * sizeof(float),
        (void*)(3 * sizeof(float))
    );
    glEnableVertexAttribArray(1);

    glBindVertexArray(0); // unbind the vertex array to keep state clean
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Object::Object(Object&& other) noexcept             // move constructor
{
    VAO = other.VAO;
    VBO = other.VBO;
    EBO = other.EBO;
    vertexCount = other.vertexCount;
    indexCount = other.indexCount;
    m_posX = other.m_posX;
    m_posY = other.m_posY;
    m_posZ = other.m_posZ;
    m_velX = other.m_velX;
    m_velY = other.m_velY;
	m_velZ = other.m_velZ;
    m_mass = other.m_mass;
    m_color = other.m_color;
    m_exertsGravity = other.m_exertsGravity;
    m_vertices = std::move(other.m_vertices);
    m_indices = std::move(other.m_indices);
    m_model = other.m_model;

    // Null out the other's handles so destructor won't delete them
    other.VAO = 0;
    other.VBO = 0;
    other.EBO = 0;
}

Object::Object(float x, float y, float z, float vx, float vy, float vz, float m, bool movable,
    glm::vec3 color, bool exertsGravity) // constructor
    : m_posX(x), m_posY(y), m_posZ(z), m_velX(vx), m_velY(vy), m_velZ(vz), m_mass(m), m_movable(movable), m_exertsGravity(exertsGravity)
{
    SetColor(glm::vec3(0.5f, 0.5f, 1.0f)); // default color
    GenSphereMesh(32);
}

Object::~Object() // destructor
{
    glDeleteBuffers(1, &VBO); // cleanup!
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
    std::cout << "Object destroyed\n";
}

void Object::DrawObject(int modelLoc, int colorLoc, bool highlighted) {

    glm::vec3 drawColor = highlighted
        ? glm::min(m_color * 1.8f + glm::vec3(0.35f), glm::vec3(1.0f))
        : m_color;

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m_model));
    glUniform3fv(colorLoc, 1, glm::value_ptr(drawColor));

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (void*)0);
    glBindVertexArray(0);


}

void Object::TogglePaths(GLFWwindow* window, float delta)
{
    m_pathTimer += delta;
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && m_pathTimer > 0.2) {
        m_drawPaths = !m_drawPaths;
        m_pathTimer = 0;
    }

}

void Object::Update(float delta, GLFWwindow* window) {

    m_posX += m_velX * delta;
    m_posY += m_velY * delta;
	m_posZ += m_velZ * delta;

    // Update model matrix
    m_model = glm::translate(glm::mat4(1.0f), glm::vec3(m_posX, m_posY, m_posZ));
    UpdateSize();

    m_pathTimer += delta;
}

void Object::UpdateSize()
{
    m_model = glm::translate(glm::mat4(1.0f), glm::vec3(m_posX, m_posY, m_posZ));
    m_model = glm::scale(m_model, glm::vec3(m_mass, m_mass, m_mass));
}

void Object::UpdatePath(GLFWwindow* window, float delta)
{
    m_path.UpdateVertices(m_posX, m_posY, m_posZ);
    TogglePaths(window, delta);
}



// non member functions



void ApplyGravity(Object& one, Object& two, float delta, Menu& menu)
{
    glm::vec3 pos1 = one.GetPos();
    glm::vec3 pos2 = two.GetPos();

    glm::vec3 diff = pos2 - pos1;
    float dist2 = glm::dot(diff, diff);

    if (dist2 < 1e-12f)
        return;

    float dist = std::sqrt(dist2);
    glm::vec3 normal = diff / dist;

    float contact = one.GetMass() + two.GetMass();

    if (dist >= contact)
    {
        if (!menu.GetGravitySwitch())
            return;

        float fMag = M_G * one.GetMass() * two.GetMass() / (dist2 + kSoftening * kSoftening);

        // this is kind of a cop out i wont hold you but it makes things look reaaaaaal nice in those presets
        if (one.GetMovable() && two.GetExertsGravity())
            one.IncVel(normal * (fMag / one.GetMass()), delta);
        if (two.GetMovable() && one.GetExertsGravity())
            two.IncVel(-normal * (fMag / two.GetMass()), delta);

        return;
    }

    glm::vec3 vel1 = one.GetVel();
    glm::vec3 vel2 = two.GetVel();
    float m1 = one.GetMass();
    float m2 = two.GetMass();
    float overlap = contact - dist;

    // CASE 1: one movable, two immovable 
    if (one.GetMovable() && !two.GetMovable())
    {
        if (glm::dot(vel1, normal) > 0.0f)
            one.SetVel(vel1 - 2.0f * glm::dot(vel1, normal) * normal);

        one.SetPosition(pos1 - normal * overlap);
        return;
    }
    if (!one.GetMovable() && two.GetMovable())
    {
        if (glm::dot(vel2, -normal) > 0.0f)
            two.SetVel(vel2 - 2.0f * glm::dot(vel2, normal) * normal);

        two.SetPosition(pos2 + normal * overlap);
        return;
    }
    if (!one.GetMovable() && !two.GetMovable())
        return;

    // CASE 2: both movable (proper elastic collision)
    glm::vec3 relativeVel = vel1 - vel2;
    float velAlongNormal = glm::dot(relativeVel, normal);

    if (velAlongNormal > 0.0f)
    {
        const float restitution = 1.0f;

        float j = -(1.0f + restitution) * velAlongNormal;
        j /= (1.0f / m1 + 1.0f / m2);

        glm::vec3 impulse = j * normal;

        one.SetVel(vel1 + impulse / m1);
        two.SetVel(vel2 - impulse / m2);
    }

    // Proper position correction 
    if (overlap > 0.0f)
    {
        const float percent = 0.8f;
        const float slop = 0.01f;

        glm::vec3 correction = normal * (percent * std::max(overlap - slop, 0.0f) /
            (1.0f / m1 + 1.0f / m2));

        one.SetPosition(pos1 - correction / m1);
        two.SetPosition(pos2 + correction / m2);
    }
}

void ApplyGravity2(std::vector<std::unique_ptr<Object>>& objects, float delta, Menu& menu) {
    for (size_t i{ 0 }; i < objects.size(); ++i)
    {
        for (size_t k{ i + 1 }; k < objects.size(); ++k)
        {
            ApplyGravity(*objects[i], *objects[k], delta, menu);
        }
    }
}
float gravTimer{ 0 };
void IncrementGravity(GLFWwindow* window, float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS && GLFW_MOD_SHIFT && gravTimer > 0.2) {
        M_G += 0.1f;
        gravTimer = 0;
    }
    if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS && gravTimer > 0.2) {
        M_G -= 0.1f;
        gravTimer = 0;
    }
    gravTimer += deltaTime;
}
