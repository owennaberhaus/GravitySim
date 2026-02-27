#include "object.h"
#include "menu.h" 

float M_G{ 0.5f }; // gravitational constant - adjust as needed for visual effect


void Object::GenCircleVertices(int segments)
{
    m_vertices.push_back(0.0f);
    m_vertices.push_back(0.0f);
	m_vertices.push_back(0.0f);
    for (auto i{ 0 }; i <= segments; ++i) {
        float phi = M_PI * i / segments;
        for (auto j{ 0 }; j < segments; ++j) { 
            float theta = (2 * static_cast<float>(M_PI)) * j / segments;

            // SPHEREICALNCOORDINATEC CALLSSC # CALC ##333!!!N!111!!
            float x = sin(phi) * cos(theta);
            float y = sin(phi) * sin(theta);
            float z = cos(phi);

            m_vertices.push_back(x);
            m_vertices.push_back(y);
			m_vertices.push_back(z);
        }
    }
    vertexCount = m_vertices.size() / 3; // each vertex has 3 points (x,y,z)

    // generate and bind VAO // VBO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(float), m_vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(
        0,                // location in shader
        3,                // 2 floats per vertex (x,y)
        GL_FLOAT,
        GL_FALSE,
        3 * sizeof(float), // stride
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
    m_posZ = other.m_posZ;
    m_velX = other.m_velX;
    m_velY = other.m_velY;
	m_velZ = other.m_velZ;
    m_mass = other.m_mass;
    m_color = other.m_color;
    m_exertsGravity = other.m_exertsGravity;
    m_vertices = std::move(other.m_vertices);
    m_model = other.m_model;

    // Null out the other’s handles so destructor won’t delete them
    other.VAO = 0;
    other.VBO = 0;
}

Object::Object(float x, float y, float z, float vx, float vy, float vz, float m, bool movable,
    glm::vec3 color, bool exertsGravity) // constructor
    : m_posX(x), m_posY(y), m_posZ(z), m_velX(vx), m_velY(vy), m_velZ(vz), m_mass(m), m_movable(movable), m_exertsGravity(exertsGravity)
{
    SetColor(glm::vec3(0.5f, 0.5f, 1.0f)); // default color
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
    glDrawArrays(GL_POINTS, 0, vertexCount);
    glBindVertexArray(0);


}

void Object::Update(float delta) {

    m_posX += m_velX * delta;
    m_posY += m_velY * delta;
	m_posZ += m_velZ * delta;

    // Update model matrix
    m_model = glm::translate(glm::mat4(1.0f), glm::vec3(m_posX, m_posY, m_posZ));
    UpdateSize();
}

void Object::UpdateSize()
{
    m_model = glm::translate(glm::mat4(1.0f), glm::vec3(m_posX, m_posY, m_posZ));
    m_model = glm::scale(m_model, glm::vec3(m_mass, m_mass, m_mass));
}


// non member functions



void ApplyGravity(Object& one, Object& two, float delta, Menu& menu)
{
    float tx = std::abs(one.GetPosX() - two.GetPosX());
    float ty = std::abs(one.GetPosY() - two.GetPosY());
	float tz = std::abs(one.GetPosZ() - two.GetPosZ());
    float radius = static_cast<float>(sqrt(pow(tx, 2) + pow(ty, 2) + pow(tz, 2)));

    if (radius >= one.GetMass() + two.GetMass())
    {

        float fMag = static_cast<float>(M_G * (one.GetMass() * two.GetMass()) / static_cast<float>(pow(radius, 2)));
        float oneAccel = menu.GetGravitySwitch() ? fMag / one.GetMass() : 0;
        float twoAccel = menu.GetGravitySwitch() ? fMag / two.GetMass() : 0;

        one.GetMovable() ? one.IncVelX((tx / radius) * oneAccel * (one.GetPosX() >= two.GetPosX() ? -1 : 1), delta) : one.IncVelX(0, delta);
        one.GetMovable() ? one.IncVelY((ty / radius) * oneAccel * (one.GetPosY() >= two.GetPosY() ? -1 : 1), delta) : one.IncVelY(0, delta);
        one.GetMovable() ? one.IncVelZ((tz / radius) * oneAccel * (one.GetPosZ() >= two.GetPosZ() ? -1 : 1), delta) : one.IncVelZ(0, delta);
        two.GetMovable() ? two.IncVelX((tx / radius) * oneAccel * (two.GetPosX() >= one.GetPosX() ? -1 : 1), delta) : two.IncVelX(0, delta);
        two.GetMovable() ? two.IncVelY((ty / radius) * oneAccel * (two.GetPosY() >= one.GetPosY() ? -1 : 1), delta) : two.IncVelY(0, delta);
        two.GetMovable() ? two.IncVelZ((tz / radius) * oneAccel * (two.GetPosZ() >= one.GetPosZ() ? -1 : 1), delta) : two.IncVelZ(0, delta);

    }
    else 
    {
        glm::vec3 pos1(one.GetPosX(), one.GetPosY(), one.GetPosZ());
        glm::vec3 pos2(two.GetPosX(), two.GetPosY(), two.GetPosZ());

        glm::vec3 vel1(one.GetVelX(), one.GetVelY(), one.GetVelZ());
        glm::vec3 vel2(two.GetVelX(), two.GetVelY(), two.GetVelZ());

        float m1 = one.GetMass();
        float m2 = two.GetMass();

        // Collision normal
        glm::vec3 normal = glm::normalize(pos2 - pos1);

        // ---- CASE 1: one movable, two immovable ----
        if (one.GetMovable() && !two.GetMovable())
        {
            glm::vec3 reflected = vel1 - 2.0f * glm::dot(vel1, normal) * normal;
            one.SetVel(reflected.x, reflected.y, reflected.z);
        }
        else if (!one.GetMovable() && two.GetMovable())
        {
            glm::vec3 reflected = vel2 - 2.0f * glm::dot(vel2, -normal) * (-normal);
            two.SetVel(reflected.x, reflected.y, reflected.z);
        }
        // ---- CASE 2: both movable (proper 2D elastic collision) ----
        else if (one.GetMovable() && two.GetMovable())
        {
            glm::vec3 diff = pos2 - pos1;
            float dist = glm::length(diff);
            if (dist == 0.0f) return;

            glm::vec3 normal = diff / dist;

            glm::vec3 relativeVel = vel1 - vel2;
            float velAlongNormal = glm::dot(relativeVel, normal);

            if (velAlongNormal > 0.0f)
                return;

            float restitution = 1.0f;

            float j = -(1.0f + restitution) * velAlongNormal;
            j /= (1.0f / m1 + 1.0f / m2);

            glm::vec3 impulse = j * normal;

            one.SetVel(
                vel1.x + impulse.x / m1,
                vel1.y + impulse.y / m1,
                vel1.z + impulse.z / m1
            );

            two.SetVel(
                vel2.x - impulse.x / m2,
                vel2.y - impulse.y / m2,
                vel2.z - impulse.z / m2
            );

            // ---- Proper position correction ----
            float overlap = (one.GetMass() + two.GetMass()) - dist;
            if (overlap > 0.0f)
            {
                float percent = 0.8f;
                float slop = 0.01f;

                glm::vec3 correction = normal * percent *
                    std::max(overlap - slop, 0.0f) /
                    (1.0f / m1 + 1.0f / m2);

                pos1 -= correction / m1;
                pos2 += correction / m2;

                one.SetPosition(pos1.x, pos1.y, pos1.z);
                two.SetPosition(pos2.x, pos2.y, pos2.z);
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

void IncrementGravity(GLFWwindow* window, int& timer) {
    if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS && GLFW_MOD_SHIFT && timer > 10) {
        M_G += 0.1f;
        timer = 0;
    }
    if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS && timer > 10) {
        M_G -= 0.1f;
        timer = 0;
    }
    timer++;
}