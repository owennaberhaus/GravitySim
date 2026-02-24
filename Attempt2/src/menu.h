#pragma once

#include <vector>
#include <memory>
#include "object.h"

class Menu
{
private:
	void IncSelectedRight();
	void IncSelectedLeft();
	void UpdateSelected(GLFWwindow* window);
	void SwapSliderColors();

public:
	Menu();
	~Menu();

	void UpdateAndDrawMenu(int modelLoc, int colorLoc, GLFWwindow* window, float delta);
	void ToggleGravityAndInitVel(GLFWwindow* window);
	

	float GetMass() { return m_massSlider.GetMass(); }
	float GetXVel() { return m_xVelSlider.GetMass(); }
	float GetYVel() { return m_yVelSlider.GetMass(); }

	float GetXPositive() { return m_xPositive; }
	float GetYPositive() { return m_yPositive; }

	bool GetGravitySwitch() { return m_gravitySwitch; }
	bool GetInitVelSwitch() { return m_initVelSwitch; }


	Object* selectedSlider{ &m_massSlider };

private:

	Object m_massSlider       {  0.0f, 0.9f, 0.00000f, 0.00000f, 0.010f, false, glm::vec3(0.5f, 1.0f, 0.2f), false };
	Object m_xVelSlider       { -0.7f, 0.9f, 0.00000f, 0.00000f, 0.010f, false, glm::vec3(0.5f, 1.0f, 0.2f), false };
	Object m_yVelSlider       {  0.7f, 0.9f, 0.00000f, 0.00000f, 0.010f, false, glm::vec3(0.5f, 1.0f, 0.2f), false };
	Object m_selectedIndicator{  selectedSlider->GetPosX(), selectedSlider->GetPosY(), 0.00000f, 0.00000f, 0.013f, false, glm::vec3(1.0f, 1.0f, 1.0f), false};

	int m_timer{ 0 };
	int m_timer2{ 0 };
	int m_timer3{ 0 };
	bool m_xPositive{ true };
	bool m_yPositive{ true };
	bool m_gravitySwitch{ true };
	bool m_initVelSwitch{ true };

};

void DeleteObjects(std::vector<std::unique_ptr<Object>>& objects, GLFWwindow* window, float worldX, float worldY);

void SolveProjection(float& worldLeft, float& worldRight, float& worldBottom, float& worldTop, int projLoc, float width, float height);