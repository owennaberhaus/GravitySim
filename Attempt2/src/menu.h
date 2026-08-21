#pragma once

#include <vector>
#include <memory>
#include "object.h"
#include "camera.h"

class Menu
{
private:
	void IncSelectedRight();
	void IncSelectedLeft();
	void UpdateSelected(GLFWwindow* window);
	void SwapSliderColors();
	int CheckSelected();

public:
	Menu(Camera& camera);
	~Menu();

	void UpdateAndDrawMenu(int modelLoc, int colorLoc, int projLoc2D, GLFWwindow* window, float delta, float width, float height);
	void ToggleGravityAndInitVel(GLFWwindow* window);
	

	float GetMass() { return m_massSlider.GetMass(); }
	float GetXVel() { return m_xVelSlider.GetMass(); }
	float GetYVel() { return m_yVelSlider.GetMass(); }
	float GetZVel() { return m_zVelSlider.GetMass(); } 

	float GetXPositive() { return m_xPositive; }
	float GetYPositive() { return m_yPositive; }
	float GetZPositive() { return m_zPositive; } 

	bool GetGravitySwitch() { return m_gravitySwitch; }
	bool GetInitVelSwitch() { return m_initVelSwitch; }

	Object* GetSelectedSlider() { return selectedSlider; }

	// What each slider orb means, so the HUD can caption them. Position and
	// radius are in the menu's own ortho space, not pixels.
	struct SliderLabel
	{
		const char* text;
		float x;
		float y;
		float radius;
		bool selected;
	};
	std::vector<SliderLabel> GetSliderLabels();


private:

	Object m_massSlider       { -0.9f, 0.9f, 0.0f, 0.00000f, 0.00000f, 0.0f, 0.010f, false, glm::vec3(0.5f, 0.8f, 1.0f), false };
	Object m_xVelSlider       { -0.6f, 0.9f, 0.0f, 0.00000f, 0.00000f, 0.0f, 0.010f, false, glm::vec3(0.7f, 0.8f, 0.2f), false };
	Object m_yVelSlider       { -0.3f, 0.9f, 0.0f, 0.00000f, 0.00000f, 0.0f, 0.010f, false, glm::vec3(0.7f, 0.8f, 0.2f), false };
	Object m_zVelSlider       {  0.0f, 0.9f, 0.0f, 0.00000f, 0.00000f, 0.0f, 0.010f, false, glm::vec3(0.7f, 0.8f, 0.2f), false };
	Object* selectedSlider{ &m_massSlider };
	Object m_selectedIndicator{  0.0f, 0.0f, 0.0f, 0.00000f, 0.00000f, 0.0f, 0.013f, false, glm::vec3(1.0f, 1.0f, 1.0f), false};

	glm::vec3 m_menuPosition{ 0, 0, 0 };

	float m_timer{ 0 };
	float m_timer2{ 0 };
	float m_timer3{ 0 };
	float m_timer4{ 0 };
	bool m_xPositive{ true };
	bool m_yPositive{ true };
	bool m_zPositive{ true };
	bool m_gravitySwitch{ true };
	bool m_initVelSwitch{ true };
	Camera* m_camera{ nullptr }; // for knowing the camera's position so the menu stays stuck to the camera

};

// Solves the world-space bounds used to map the cursor onto the spawn plane
void SolveProjection(float& worldLeft, float& worldRight, float& worldBottom, float& worldTop,
					 float width, float height, Camera& camera);