#pragma once
#include "GLFW/glfw3.h"
#include <vector>
#include "object.h"
#include "menu.h"
#include "camera.h"
extern float g_scrollDelta;

// just 1 class to control all mouse input
// TODO: actually move all input into the clicker class 
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

    glm::vec3 GetMouseRay(double mouseX, double mouseY, int width, int height, const glm::mat4& projection, const glm::mat4& view)
    {
        float x = (2.0f * mouseX) / width - 1.0f;
        float y = 1.0f - (2.0f * mouseY) / height;

        glm::vec4 rayClip(x, y, -1.0f, 1.0f);

        glm::vec4 rayEye = glm::inverse(projection) * rayClip;
        rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

        glm::vec3 rayWorld = glm::normalize(glm::vec3(glm::inverse(view) * rayEye));

        return rayWorld;
    }

    bool RayIntersectsSphere(glm::vec3 rayOrigin, glm::vec3 rayDir, glm::vec3 sphereCenter, float sphereRadius)
    {
        glm::vec3 oc = rayOrigin - sphereCenter; // origin to center

		// quadratic formula components for ray-sphere intersection (reddit glazed this)
        float a = glm::dot(rayDir, rayDir);
        float b = 2.0f * glm::dot(oc, rayDir);
        float c = glm::dot(oc, oc) - sphereRadius * sphereRadius;

        float discriminant = b * b - 4.0f * a * c;
        return discriminant >= 0.0f;

        float sqrtD = glm::sqrt(discriminant);
        float t0 = (-b - sqrtD) / (2.0f * a); // near hit
        float t1 = (-b + sqrtD) / (2.0f * a); // far hit

        // at least one intersection must be in front of the ray origin
        return t0 > 0.0f || t1 > 0.0f;
    }

public:
    void DeleteObjects(std::vector<std::unique_ptr<Object>>& objects, GLFWwindow* window, float xpos, float ypos, int width, int height, Camera& camera)
    {
        glm::vec3 rayOrigin = camera.GetPosition();
        glm::vec3 rayDir = GetMouseRay(xpos, ypos, width, height, camera.GetProjectionMatrix(), camera.GetViewMatrix());

		for (auto i = objects.begin(); i != objects.end(); ) // dynamically erase objects while iterating, only increment if not erasing
        {
            glm::vec3 center((*i)->GetPosX(), (*i)->GetPosY(), (*i)->GetPosZ());
            if (RayIntersectsSphere(rayOrigin, rayDir, center, (*i)->GetMass() * 2) && glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            {
                i = objects.erase(i);  // remove and get next valid iterator - erase returns next element
                std::cout << "true" << '\n';
            }
            else
            {
                ++i;
            }
        }
    }

    void MouseControl(GLFWwindow* window, float worldX, float worldY, std::vector<std::unique_ptr<Object>>& objects, Menu& menu, float& g_scrollDelta, Camera& camera) {
        /*frame timer*/
        ++m_framesSinceClick;

        /*catch where*/
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

};

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    g_scrollDelta += static_cast<float>(yoffset);
}