#pragma once
#include "GLFW/glfw3.h"
#include <vector>
#include <limits>
#include <cmath>
#include "object.h"
#include "menu.h"
#include "camera.h"
extern float g_scrollDelta;

// just 1 class to control all mouse input
// TODO: actually move all input into the clicker class ... haha
class Clicker {
private:
    glm::vec3 GetSpawnPositionOnPlane(float worldX, float worldY, Camera& camera, float distanceScale = 1.0f)
    {
        // Get camera vectors
        glm::vec3 camPos = camera.GetPosition();
        glm::vec3 forward = glm::normalize(camera.GetTarget() - camPos);
        glm::vec3 up = camera.GetUp();
        glm::vec3 right = glm::normalize(glm::cross(forward, up));

        // Scale distance in front of camera by radius
        float spawnDistance = camera.GetRadius() * distanceScale;

        // Base spawn position directly in front of camera
        glm::vec3 spawnPos = camPos + forward * spawnDistance;

        // Offset by mouse world coordinates in camera's plane
        spawnPos += right * worldX;
        spawnPos += up * worldY;

        return spawnPos;
    }

    // Builds the world-space direction of the ray under the cursor.
    //
	// curser position is in window coordinates, with (0,0) at the top-left
    glm::vec3 GetMouseRay(GLFWwindow* window, double mouseX, double mouseY,
        const glm::mat4& projection, const glm::mat4& view)
    {
        int winW{ 0 }, winH{ 0 };
        glfwGetWindowSize(window, &winW, &winH);
        if (winW <= 0 || winH <= 0)
            return glm::vec3(0.0f, 0.0f, -1.0f);

        // window pixels must be turned to normalized device coordinates 
        float x = (2.0f * static_cast<float>(mouseX)) / static_cast<float>(winW) - 1.0f;
        float y = 1.0f - (2.0f * static_cast<float>(mouseY)) / static_cast<float>(winH);

        glm::vec4 rayClip(x, y, -1.0f, 1.0f);

        // undo the projection to get a direction in eye space, then the view to get it in world space
        glm::vec4 rayEye = glm::inverse(projection) * rayClip;
        rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

        return glm::normalize(glm::vec3(glm::inverse(view) * rayEye));
    }

    // Distance along the ray to the nearest intersection with the sphere, or a
    // negative value if it never hits in front of the origin.

    float RaySphereDistance(const glm::vec3& rayOrigin, const glm::vec3& rayDir,
        const glm::vec3& sphereCenter, float sphereRadius)
    {
        glm::vec3 oc = rayOrigin - sphereCenter; // origin to center

        // quadratic formula components for ray-sphere intersection
        float a = glm::dot(rayDir, rayDir);
        float b = 2.0f * glm::dot(oc, rayDir);
        float c = glm::dot(oc, oc) - sphereRadius * sphereRadius;

        float discriminant = b * b - 4.0f * a * c;
        if (discriminant < 0.0f)
            return -1.0f; // the line misses entirely

        float sqrtD = std::sqrt(discriminant);
        float t0 = (-b - sqrtD) / (2.0f * a); // near hit
        float t1 = (-b + sqrtD) / (2.0f * a); // far hit

        if (t0 > 0.0f) return t0; // normal case: entering the sphere
        if (t1 > 0.0f) return t1; // camera is inside the sphere
        return -1.0f;             // wholly behind the camera
    }

public:
    // Which object is the cursor over? Returns the index of the CLOSEST one the ray actually enters, or -1 for none.
    int FindHoveredObject(std::vector<std::unique_ptr<Object>>& objects, GLFWwindow* window,
        double xpos, double ypos, Camera& camera)
    {
        glm::vec3 rayOrigin = camera.GetPosition();
        glm::vec3 rayDir = GetMouseRay(window, xpos, ypos,
            camera.GetProjectionMatrix(), camera.GetViewMatrix());

        int nearest = -1;
        float nearestT = std::numeric_limits<float>::max();

        for (size_t i = 0; i < objects.size(); ++i)
        {
            float t = RaySphereDistance(rayOrigin, rayDir, objects[i]->GetPos(), objects[i]->GetMass());
            if (t >= 0.0f && t < nearestT)
            {
                nearestT = t;
                nearest = static_cast<int>(i);
            }
        }

        return nearest;
    }

    // Refresh what's under the cursor, and delete it on a space press.
    void UpdateHoverAndDelete(std::vector<std::unique_ptr<Object>>& objects, GLFWwindow* window,
        double xpos, double ypos, Camera& camera)
    {
        m_hoveredIndex = FindHoveredObject(objects, window, xpos, ypos, camera);

        // Rising edge only
        bool spaceDown = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
        bool spacePressed = spaceDown && !m_spaceWasDown;
        m_spaceWasDown = spaceDown;

        if (spacePressed && m_hoveredIndex >= 0 && m_hoveredIndex < static_cast<int>(objects.size()))
        {
            objects.erase(objects.begin() + m_hoveredIndex);
            m_hoveredIndex = -1; // whatever was under the cursor is gone
        }
    }

    // -1 when nothing is hovered. Used by the renderer to brighten the target
    int GetHoveredIndex() const { return m_hoveredIndex; }

    void MouseControl(GLFWwindow* window, float worldX, float worldY, std::vector<std::unique_ptr<Object>>& objects, Menu& menu, float& g_scrollDelta, Camera& camera) {
        /*frame timer*/
        ++m_framesSinceClick;

        // catch where
        glm::vec3 spawnPos = GetSpawnPositionOnPlane(worldX, worldY, camera, 1.0f) * static_cast<float>(pow(camera.GetRadius(), 3) * 1.25);
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && m_framesSinceClick > 30)
        {
            objects.push_back(std::make_unique<Object>(spawnPos.x, spawnPos.y, spawnPos.z, 0.0f, 0.0f, 0.0f, menu.GetMass(), false, glm::vec3(0.5f, 0.5f, 0.5f)));
            m_framesSinceClick = 0;
        }
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS && m_framesSinceClick > 30)
        {
            if (menu.GetInitVelSwitch()) {
                objects.push_back(std::make_unique<Object>(spawnPos.x, spawnPos.y, spawnPos.z,
                    menu.GetXPositive() ? (menu.GetXVel() * 3) : -(menu.GetXVel() * 3),
                    menu.GetYPositive() ? (menu.GetYVel() * 3) : -(menu.GetYVel() * 3),
					menu.GetZPositive() ? (menu.GetZVel() * 3) : -(menu.GetZVel() * 3),
                    menu.GetMass(), true, glm::vec3(0.0f, 0.0f, 1.0f)));
            }
            else {
                objects.push_back(std::make_unique<Object>(spawnPos.x, spawnPos.y, spawnPos.z,
                    0.0f, 0.0f, 0.0f, menu.GetMass(), true, glm::vec3(0.0f, 0.0f, 1.0f)));
            }
            m_framesSinceClick = 0;
        }
        // TODO
        /*scroll to zoom*/
        if (g_scrollDelta != 0.0)
        {
			camera.IncRadius(g_scrollDelta); // zooming in and out with scroll
            g_scrollDelta = 0.0; // reset for next frame
        }

    }
private:
    int m_framesSinceClick{ 0 };
    int m_hoveredIndex{ -1 };
    bool m_spaceWasDown{ false };

};

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    g_scrollDelta += static_cast<float>(yoffset);
}