#pragma once
#include "GLFW/glfw3.h"
#include <vector>
#include "object.h"
#include "menu.h"
#include "camera.h"

class Clicker {
public:
    void DeleteObjects(std::vector<std::unique_ptr<Object>>& objects, GLFWwindow* window, float worldX, float worldY, Camera& camera)
    {
        for (size_t i{ 0 }; i < objects.size(); ++i)
        {
            float distanceMTO = static_cast<float>(sqrt
            (pow((objects[i]->GetPosX() - (worldX + camera.GetPosition().x)), 2))
                + pow((objects[i]->GetPosY() - (worldY + camera.GetPosition().y)), 2)); //mouse to object
            if (distanceMTO < (objects[i]->GetMass() + 0.01) && glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            {
                objects.erase(objects.begin() + i);
                --i;
            }

        }
    }

    void MouseControl(GLFWwindow* window, float worldX, float worldY, std::vector<std::unique_ptr<Object>>& objects, Menu& menu, float& g_scrollDelta, Camera& camera) {
        /*frame timer*/
        ++m_framesSinceClick;
        /*catch where*/
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && m_framesSinceClick > 30)
        {
            objects.push_back(std::make_unique<Object>(worldX + camera.GetPosition().x, worldY + camera.GetPosition().y, 0.0f, 0.0f, 0.0f, 0.0f, menu.GetMass(), false, glm::vec3(0.5f, 0.5f, 0.5f)));
            m_framesSinceClick = 0;
        }
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS && m_framesSinceClick > 30)
        {
            if (menu.GetInitVelSwitch()) {
                objects.push_back(std::make_unique<Object>(worldX + camera.GetPosition().x, 
                    worldY + camera.GetPosition().y, 0.0f,
                    menu.GetXPositive() ? (menu.GetXVel() * 3) : -(menu.GetXVel() * 3),
                    menu.GetYPositive() ? (menu.GetYVel() * 3) : -(menu.GetYVel() * 3),
					menu.GetZPositive() ? (menu.GetZVel() * 3) : -(menu.GetZVel() * 3),
                    menu.GetMass(), true, glm::vec3(0.0f, 0.0f, 1.0f)));
            }
            else {
                objects.push_back(std::make_unique<Object>(worldX + camera.GetPosition().x,
                    worldY + camera.GetPosition().y, 0.0f,
                    0.0f, 0.0f, 0.0f, menu.GetMass(), true, glm::vec3(0.0f, 0.0f, 1.0f)));
            }
            m_framesSinceClick = 0;
        }
        // TODO
        /*scroll to zoom*/
        if (g_scrollDelta != 0.0)
        {
            // scrolling used to be a part of the menu - depricated code below
            //menu.selectedSlider->IncMass(g_scrollDelta / 500);
            //if (menu.selectedSlider->GetMass() < 0)
            //    menu.selectedSlider->SetMass(0);

			camera.IncZoom(g_scrollDelta); // zooming in and out with scroll
            g_scrollDelta = 0.0; // reset for next frame
        }

        DeleteObjects(objects, window, worldX, worldY, camera);
    }
private:
    int m_framesSinceClick{ 0 };


};