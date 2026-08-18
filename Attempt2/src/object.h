#pragma once
extern float M_G; // gravitational constant that is changed with + / -
#define M_PI 3.1415926535897932384626433

#include <glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <matrix_transform.hpp>
#include <type_ptr.hpp>
#include <vector>
#include <memory>
#include <iostream>
#include "path.h"

class Menu;

// basically astral bodies
class Object
{
private:
    void GenSphereMesh(int segments);
    void TogglePaths(GLFWwindow* window, float timer);

public:

    GLuint VAO{};
    GLuint VBO{};
    GLuint EBO{};          // index buffer - a UV sphere can't be drawn without one
    GLsizei vertexCount{};
    GLsizei indexCount{};

    Object(const Object&) = delete;            // disable copy constructor
    Object& operator=(const Object&) = delete; // disable copy assignment
    Object(Object&& other) noexcept;
    Object(float x, float y, float z, float vx, float vy, float vz, float m, bool movable = true,
        glm::vec3 color = glm::vec3(0.0f, 0.0f, 1.0f), bool exertsGravity = true);
    ~Object();

    void DrawObject(int modelLoc, int colorLoc, bool highlighted = false);

    void Update(float delta, GLFWwindow* window);

    void UpdateSize();
    void Move(glm::vec3 distance) { SetPosition(GetPosX() + distance.x, GetPosY() + distance.y, GetPosZ() + distance.z); }

    const float GetPosX() { return m_posX; }
    const float GetPosY() { return m_posY; }
	const float GetPosZ() { return m_posZ; }
    const float GetMass() { return m_mass; }
    const float GetVelX() { return m_velX; }
    const float GetVelY() { return m_velY; }
	const float GetVelZ() { return m_velZ; }
    const bool GetMovable() { return m_movable; }
    const glm::vec3 GetColor() { return m_color; }
    void SetPosX(float val) { m_posX = val; }
    void SetPosY(float val) { m_posY = val; }
	void SetPosZ(float val) { m_posZ = val; }
    void SetPosition(float x, float y, float z = 0) { m_posX = x; m_posY = y; m_posZ = z; }
    void SetMass(float val) { m_mass = val; }
    void SetColor(glm::vec3 color) { m_color = color; }
    void IncVelX(float val, float delta) { m_velX += val * delta; }
    void IncVelY(float val, float delta) { m_velY += val * delta; }
	void IncVelZ(float val, float delta) { m_velZ += val * delta; }
    void SetVel(float x, float y, float z = 0) { m_velX = x; m_velY = y; m_velZ = z; }

    // vector conveniences - the physics below reads far better in vec3 form
    glm::vec3 GetPos() { return glm::vec3(m_posX, m_posY, m_posZ); }
    glm::vec3 GetVel() { return glm::vec3(m_velX, m_velY, m_velZ); }
    void SetPosition(const glm::vec3& p) { m_posX = p.x; m_posY = p.y; m_posZ = p.z; }
    void SetVel(const glm::vec3& v) { m_velX = v.x; m_velY = v.y; m_velZ = v.z; }
    void IncVel(const glm::vec3& accel, float delta) { m_velX += accel.x * delta; m_velY += accel.y * delta; m_velZ += accel.z * delta; }
    void IncMass(float val) { m_mass += val; }

    void UpdatePath(GLFWwindow* window, float delta);
    void DrawPath(int modelLoc, int colorLoc) {
        if (m_drawPaths) {
            glm::mat4 identity = glm::mat4(1.0f);
            m_path.DrawPath(modelLoc, colorLoc, identity);
        }
    }

private:
    float m_mass{}; // will impact how large the object looks and acts
    float m_posX{};
    float m_posY{};
    float m_posZ{};
    float m_velX{};
    float m_velY{};
    float m_velZ{};
    bool m_movable{};
	std::vector<float> m_vertices{};  // interleaved: px,py,pz, nx,ny,nz for each vertex
    std::vector<GLuint> m_indices{};
    const float m_GRAVITY = -0.000000f; // negative Y = down
    glm::mat4 m_model{ 1.0f };
    glm::vec3 m_color;
    bool m_exertsGravity{ true };

    Path m_path{ 0.005f, m_posX, m_posY, m_posZ, glm::vec3(1.0f, 1.0f, 1.0f) };
	float m_pathTimer{ 0.0f };
	bool m_drawPaths{ true };


};

// non member functs
void ApplyGravity(Object& one, Object& two, float delta, Menu& menu);

void ApplyGravity2(std::vector<std::unique_ptr<Object>>& objects, float delta, Menu& menu);

void IncrementGravity(GLFWwindow* window, float deltaTime);