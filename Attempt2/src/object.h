#pragma once

#define M_G 1 // gravitational constant - adjust as needed for visual effect
#define M_PI 3.1415926535897932384626433

#include <glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <matrix_transform.hpp>
#include <type_ptr.hpp>
#include <vector>
#include <iostream>
#include "path.h"

class Menu;

// basically astral bodies
class Object
{
private:
    void GenCircleVertices(int segments);
   
public:

    GLuint VAO{};
    GLuint VBO{};
    GLsizei vertexCount{};

    Object(const Object&) = delete;            // disable copy constructor
    Object& operator=(const Object&) = delete; // disable copy assignment
    Object(Object&& other) noexcept;
    Object(float x, float y, float z, float vx, float vy, float vz, float m, bool movable = true,
        glm::vec3 color = glm::vec3(0.0f, 0.0f, 1.0f), bool exertsGravity = true);
    ~Object();

    void DrawObject(int modelLoc, int colorLoc);

    void Update(float delta);

    void UpdateSize();

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
    void IncMass(float val) { m_mass += val; }

    void UpdatePath() { m_path.UpdateVertices(m_posX, m_posY); }
    void DrawPath(int modelLoc, int colorLoc) { m_path.DrawPath(modelLoc, colorLoc); }

private:
    float m_mass{}; // will impact how large the object looks and acts
    float m_posX{};
    float m_posY{};
    float m_posZ{};
    float m_velX{};
    float m_velY{};
    float m_velZ{};
    bool m_movable{};
    std::vector<float> m_vertices{};
    const float m_GRAVITY = -0.000000f; // negative Y = down
    glm::mat4 m_model{ 1.0f };
    glm::vec3 m_color;
    bool m_exertsGravity{ true };

    Path m_path{ 0.005f, m_posX, m_posY, glm::vec3(1.0f, 1.0f, 1.0f) };


};

// non member functs
void ApplyGravity(Object& one, Object& two, float delta, Menu& menu);

void ApplyGravity2(std::vector<std::unique_ptr<Object>>& objects, float delta, Menu& menu);