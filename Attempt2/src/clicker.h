#pragma once
#include "GLFW/glfw3.h"
#include <vector>
#include "object.h"
#include "menu.h"

class Clicker {
public:
    void MouseControl(GLFWwindow* window, float worldX, float worldY, std::vector<std::unique_ptr<Object>>& objects, Menu& menu, float& g_scrollDelta) {
        /*frame timer*/
        ++m_framesSinceClick;
        /*catch where*/
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && m_framesSinceClick > 30)
        {
            objects.push_back(std::make_unique<Object>(worldX, worldY, 0.0f, 0.0f, menu.GetMass(), false, glm::vec3(0.5f, 0.5f, 0.5f)));
            m_framesSinceClick = 0;
        }
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS && m_framesSinceClick > 30)
        {
            if (menu.GetInitVelSwitch()) {
                objects.push_back(std::make_unique<Object>(worldX, worldY,
                    menu.GetXPositive() ? (menu.GetXVel() * 3) : -(menu.GetXVel() * 3),
                    menu.GetYPositive() ? (menu.GetYVel() * 3) : -(menu.GetYVel() * 3),
                    menu.GetMass(), true, glm::vec3(0.0f, 0.0f, 1.0f)));
            }
            else {
                objects.push_back(std::make_unique<Object>(worldX, worldY,
                    0, 0, menu.GetMass(), true, glm::vec3(0.0f, 0.0f, 1.0f)));
            }
            m_framesSinceClick = 0;
        }
        /*scroll*/
        if (g_scrollDelta != 0.0)
        {
            menu.selectedSlider->IncMass(g_scrollDelta / 500);
            if (menu.selectedSlider->GetMass() < 0)
                menu.selectedSlider->SetMass(0);
            g_scrollDelta = 0.0; // reset for next frame
        }
    }
private:
    int m_framesSinceClick{ 0 };


};