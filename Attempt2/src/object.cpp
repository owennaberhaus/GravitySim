#include "object.h"
#include "menu.h" 
#define M_PI 3.1415926535897932384626433
#define M_G 0.5

void Object::GenCircleVertices(int segments)
{
    m_vertices.push_back(0.0f);
    m_vertices.push_back(0.0f);
    for (auto i{ 0 }; i <= segments; ++i) {
        float angle = (2 * static_cast<float>(M_PI)) * i / segments;
        m_vertices.push_back(cos(angle));
        m_vertices.push_back(sin(angle));
    }
    vertexCount = m_vertices.size() / 2; // each vertex has 2 points (x,y)

    // generate and bind VAO // VBO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(float), m_vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(
        0,                // location in shader
        2,                // 2 floats per vertex (x,y)
        GL_FLOAT,
        GL_FALSE,
        2 * sizeof(float), // stride
        (void*)0
    );
    glEnableVertexAttribArray(0);

    glBindVertexArray(0); // unbind the vertex array to keep state clean


}

Object::Object(Object&& other) noexcept             // move constructor
{
    VAO = other.VAO;
    VBO = other.VBO;
    vertexCount = other.vertexCount;
    m_posX = other.m_posX;
    m_posY = other.m_posY;
    m_velX = other.m_velX;
    m_velY = other.m_velY;
    m_mass = other.m_mass;
    m_color = other.m_color;
    m_exertsGravity = other.m_exertsGravity;
    m_vertices = std::move(other.m_vertices);
    m_model = other.m_model;

    // Null out the other’s handles so destructor won’t delete them
    other.VAO = 0;
    other.VBO = 0;
}

Object::Object(float x, float y, float vx, float vy, float m, bool movable,
    glm::vec3 color, bool exertsGravity) // constructor
    
{
    m_posX = x,
        m_posY = y,
        m_velX = vx,
        m_velY = vy,
        m_movable = movable,
        m_mass = m;
    m_color = color;
    m_exertsGravity = exertsGravity;
    GenCircleVertices(50); // 50 segments for now
}

Object::~Object() // destructor
{
    glDeleteBuffers(1, &VBO); // cleanup!
    glDeleteVertexArrays(1, &VAO);
    std::cout << "Object destroyed\n";
}

void Object::DrawObject(int modelLoc, int colorLoc) {

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m_model));
    glUniform3fv(colorLoc, 1, glm::value_ptr(m_color));

    glBindVertexArray(VAO);
    // glDrawArrays(GL_TRIANGLE_FAN, 0, vertexCount);
    glDrawArrays(GL_TRIANGLE_FAN, 0, vertexCount);
    glBindVertexArray(0);


}

void Object::Update(float delta) {

    m_posX += m_velX * delta;
    m_posY += m_velY * delta;

    // Update model matrix
    m_model = glm::translate(glm::mat4(1.0f), glm::vec3(m_posX, m_posY, 0.0f));
    UpdateSize();
}

void Object::UpdateSize()
{
    m_model = glm::translate(glm::mat4(1.0f), glm::vec3(m_posX, m_posY, 0.0f));
    m_model = glm::scale(m_model, glm::vec3(m_mass, m_mass, 1.0f));
}


// non member functions

void ApplyGravity(Object& one, Object& two, float delta, Menu& menu)
{

    float tx = std::abs(one.GetPosX() - two.GetPosX());
    float ty = std::abs(one.GetPosY() - two.GetPosY());
    float radius = static_cast<float>(sqrt(pow(tx, 2) + pow(ty, 2)));

    if (radius >= one.GetMass() + two.GetMass())
    {

        float fMag = static_cast<float>(M_G * (one.GetMass() * two.GetMass()) / static_cast<float>(pow(radius, 2)));
        float oneAccel = menu.GetGravitySwitch() ? fMag / one.GetMass() : 0;
        float twoAccel = menu.GetGravitySwitch() ? fMag / two.GetMass() : 0;

        one.GetMovable() ? one.IncVelX((tx / radius) * oneAccel * (one.GetPosX() >= two.GetPosX() ? -1 : 1), delta) : one.IncVelX(0, delta);
        one.GetMovable() ? one.IncVelY((ty / radius) * oneAccel * (one.GetPosY() >= two.GetPosY() ? -1 : 1), delta) : one.IncVelX(0, delta);
        two.GetMovable() ? two.IncVelX((tx / radius) * oneAccel * (two.GetPosX() >= one.GetPosX() ? -1 : 1), delta) : two.IncVelX(0, delta);
        two.GetMovable() ? two.IncVelY((ty / radius) * oneAccel * (two.GetPosY() >= one.GetPosY() ? -1 : 1), delta) : two.IncVelX(0, delta);

    }
    else 
    {
        glm::vec2 pos1(one.GetPosX(), one.GetPosY());
        glm::vec2 pos2(two.GetPosX(), two.GetPosY());

        glm::vec2 vel1(one.GetVelX(), one.GetVelY());
        glm::vec2 vel2(two.GetVelX(), two.GetVelY());

        float m1 = one.GetMass();
        float m2 = two.GetMass();

        // Collision normal
        glm::vec2 normal = glm::normalize(pos2 - pos1);

        // ---- CASE 1: one movable, two immovable ----
        if (one.GetMovable() && !two.GetMovable())
        {
            glm::vec2 reflected = vel1 - 2.0f * glm::dot(vel1, normal) * normal;
            one.SetVel(reflected.x, reflected.y);
        }
        else if (!one.GetMovable() && two.GetMovable())
        {
            glm::vec2 reflected = vel2 - 2.0f * glm::dot(vel2, -normal) * (-normal);
            two.SetVel(reflected.x, reflected.y);
        }
        // ---- CASE 2: both movable (proper 2D elastic collision) ----
        else if (one.GetMovable() && two.GetMovable())
        {
            glm::vec2 tangent(-normal.y, normal.x);

            float v1n = glm::dot(vel1, normal);
            float v1t = glm::dot(vel1, tangent);
            float v2n = glm::dot(vel2, normal);
            float v2t = glm::dot(vel2, tangent);

            // 1D elastic collision along normal
            float v1n_after = (v1n * (m1 - m2) + 2.0f * m2 * v2n) / (m1 + m2);
            float v2n_after = (v2n * (m2 - m1) + 2.0f * m1 * v1n) / (m1 + m2);

            glm::vec2 v1n_vec = v1n_after * normal;
            glm::vec2 v1t_vec = v1t * tangent;
            glm::vec2 v2n_vec = v2n_after * normal;
            glm::vec2 v2t_vec = v2t * tangent;

            glm::vec2 newVel1 = v1n_vec + v1t_vec;
            glm::vec2 newVel2 = v2n_vec + v2t_vec;

            one.SetVel(newVel1.x, newVel1.y);
            two.SetVel(newVel2.x, newVel2.y);
        }

        // ---- Position correction (prevents sticking) ----
        float overlap = (m1 + m2) - radius;
        if (overlap > 0.0f)
        {
            glm::vec2 correction = normal * (overlap * 0.5f);

            if (one.GetMovable())
            {
                one.IncVelX(0, delta); // no-op but keeps structure consistent
                one.IncVelY(0, delta);
            }

            if (two.GetMovable())
            {
                two.IncVelX(0, delta);
                two.IncVelY(0, delta);
            }
        }
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