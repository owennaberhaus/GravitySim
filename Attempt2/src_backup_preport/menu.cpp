#include "menu.h"

extern float g_scrollDelta{ 0 }; // used for scroll input, made global so 
// evrything will be aware of how much scroll since last frame

Menu::Menu(Camera& camera) : 
	m_camera(&camera)
{
	m_selectedIndicator.SetPosX(selectedSlider->GetPosX());
	m_selectedIndicator.SetPosY(selectedSlider->GetPosY());
	m_massSlider.SetColor(glm::vec3(0.7f, 8.0f, 0.2f));
	m_selectedIndicator.SetColor(glm::vec3(1.0f, 1.0f, 1.0f));
}
Menu::~Menu()
{

}

int Menu::CheckSelected()
{
	if (selectedSlider == &m_xVelSlider)
		return 1;
	else if (selectedSlider == &m_massSlider)
		return 2;
	else if (selectedSlider == &m_yVelSlider)
		return 3;
	else 
		return 4;
}

void Menu::UpdateAndDrawMenu(int modelLoc, int colorLoc, int projLoc2D,  GLFWwindow* window, float delta, float width, float height)
{
	glDisable(GL_DEPTH_TEST); // disable depth testing for 2d rendering

	m_selectedIndicator.SetMass(selectedSlider->GetMass() + 0.01f);
	SwapSliderColors();

	float selectedX;
	if (CheckSelected() == 1)
		selectedX = m_yVelSlider.GetPosX();
	else if (CheckSelected() == 2)
		selectedX = m_xVelSlider.GetPosX();
	else if (CheckSelected() == 3)
		selectedX = m_zVelSlider.GetPosX();
	else
		selectedX = m_massSlider.GetPosX();

	float aspect = width / height;
	glm::mat4 hudProjection = glm::ortho(
		-aspect, aspect,
		-1.0f, 1.0f,
		-1.0f, 1.0f
	);

	glUniformMatrix4fv(projLoc2D, 1, GL_FALSE, glm::value_ptr(hudProjection));

	m_xVelSlider.Update(delta, window);
	m_xVelSlider.DrawObject(modelLoc, colorLoc);
	m_massSlider.Update(delta, window);
	m_massSlider.DrawObject(modelLoc, colorLoc);
	m_yVelSlider.Update(delta, window);
	m_yVelSlider.DrawObject(modelLoc, colorLoc);
	m_zVelSlider.Update(delta, window);
	m_zVelSlider.DrawObject(modelLoc, colorLoc);
	m_selectedIndicator.Update(delta, window);
	m_selectedIndicator.DrawObject(modelLoc, colorLoc);
	m_timer += delta;
	m_timer2 += delta;
	m_timer3 += delta;
	m_timer4 += delta;
	UpdateSelected(window);

	glEnable(GL_DEPTH_TEST); // re-enable depth testing for 3d rendering next loop
}

std::vector<Menu::SliderLabel> Menu::GetSliderLabels()
{
	std::vector<SliderLabel> out;
	Object* sliders[4] = { &m_massSlider, &m_xVelSlider, &m_yVelSlider, &m_zVelSlider };
	const char* names[4] = { "mass", "x vel", "y vel", "z vel" };

	for (int i = 0; i < 4; ++i)
		out.push_back({ names[i], sliders[i]->GetPosX(), sliders[i]->GetPosY(),
			sliders[i]->GetMass(), selectedSlider == sliders[i] });

	return out;
}

void Menu::IncSelectedRight()
{
	if (selectedSlider == &m_zVelSlider)
		selectedSlider = &m_massSlider;
	else if (selectedSlider == &m_massSlider)
		selectedSlider = &m_xVelSlider;
	else if (selectedSlider == &m_yVelSlider)
		selectedSlider = &m_zVelSlider;
	else
		selectedSlider = &m_yVelSlider;

	m_selectedIndicator.SetPosX(selectedSlider->GetPosX());
	m_selectedIndicator.SetPosY(selectedSlider->GetPosY());

}

void Menu::IncSelectedLeft()
{
	if (selectedSlider == &m_xVelSlider)
		selectedSlider = &m_massSlider;
	else if (selectedSlider == &m_massSlider)
		selectedSlider = &m_zVelSlider;
	else if (selectedSlider == &m_yVelSlider)
		selectedSlider = &m_xVelSlider;
	else
		selectedSlider = &m_yVelSlider;

	m_selectedIndicator.SetPosX(selectedSlider->GetPosX());
	m_selectedIndicator.SetPosY(selectedSlider->GetPosY());

}

void Menu::ToggleGravityAndInitVel(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS && m_timer3 > 0.2 && m_gravitySwitch)
	{
		m_gravitySwitch = false;
		m_timer3 = 0;
	}
	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS && m_timer3 > 0.2 && !m_gravitySwitch)
	{
		m_gravitySwitch = true;
		m_timer3 = 0;
	}
	// initial velocity
	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS && m_timer3 > 0.2 && m_initVelSwitch)
	{
		m_initVelSwitch = false;
		m_timer3 = 0;
	}
	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS && m_timer3 > 0.2 && !m_initVelSwitch)
	{
		m_initVelSwitch = true;
		m_timer3 = 0;
	}
}

void Menu::UpdateSelected(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS && m_timer > 0.2)
	{
		IncSelectedRight();
		m_timer = 0;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS && m_timer > 0.2)
	{
		IncSelectedLeft();
		m_timer = 0;
	}

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && m_timer4 > 0.1)
	{
		selectedSlider->IncMass( 0.005f);
		m_timer4 = 0;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && m_timer4 > 0.1 && selectedSlider->GetMass() > 0)
	{
		selectedSlider->IncMass(-0.005f);
		m_timer4 = 0;
	}

	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && m_timer2 > 0.2 && 
		(selectedSlider == &m_xVelSlider || selectedSlider == &m_yVelSlider || selectedSlider == &m_zVelSlider))
	{
		if (m_xPositive && selectedSlider == &m_xVelSlider)
			m_xPositive = false;
		else if (!m_xPositive && selectedSlider == &m_xVelSlider)
			m_xPositive = true;

		if (m_yPositive && selectedSlider == &m_yVelSlider)
			m_yPositive = false;
		else if (!m_yPositive && selectedSlider == &m_yVelSlider)
			m_yPositive = true;

		if (m_zPositive && selectedSlider == &m_zVelSlider)
			m_zPositive = false;
		else if (!m_zPositive && selectedSlider == &m_zVelSlider)
			m_zPositive = true;

		m_timer2 = 0;
	}
}

void Menu::SwapSliderColors()
{
	if (m_xPositive) 
		m_xVelSlider.SetColor(glm::vec3(glm::vec3(0.5f, 1.0f, 0.2f)));
	else
		m_xVelSlider.SetColor(glm::vec3(glm::vec3(1.0f, 0.2f, 0.5f)));

	if (m_yPositive)
		m_yVelSlider.SetColor(glm::vec3(glm::vec3(0.5f, 1.0f, 0.2f)));
	else
		m_yVelSlider.SetColor(glm::vec3(glm::vec3(1.0f, 0.2f, 0.5f)));

	if (m_zPositive)
		m_zVelSlider.SetColor(glm::vec3(glm::vec3(0.5f, 1.0f, 0.2f)));
	else
		m_zVelSlider.SetColor(glm::vec3(glm::vec3(1.0f, 0.2f, 0.5f)));

}

void SolveProjection(float& worldLeft, float& worldRight, float& worldBottom, float& worldTop,
					 float width, float height, Camera& camera)
{
	float aspect = width / height;

	// scared to update this since adding 3D, things may hinge on finding a correct half height?
	// I think the menu, thats the next thing to fix.
	if (aspect >= 1.0f)
	{
		worldLeft = -aspect; worldRight = aspect; worldBottom = -1.0f; worldTop = 1.0f;
	}
	else
	{
		worldLeft = -1.0f; worldRight = 1.0f; worldBottom = -1.0f / aspect; worldTop = 1.0f / aspect;
	}

	float centerX = (worldLeft + worldRight) / 2.0f;
	float centerY = (worldBottom + worldTop) / 2.0f;
	float halfWidth = (worldRight - worldLeft) / 2.0f / camera.GetRadius() / 3;
	float halfHeight = (worldTop - worldBottom) / 2.0f / camera.GetRadius() / 3;

	worldLeft = centerX - halfWidth;
	worldRight = centerX + halfWidth;
	worldBottom = centerY - halfHeight;
	worldTop = centerY + halfHeight;

}