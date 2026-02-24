#include "menu.h"

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

void Menu::UpdateAndDrawMenu(int modelLoc, int colorLoc, GLFWwindow* window, float delta, float halfHeight)
{
	m_selectedIndicator.SetMass(selectedSlider->GetMass() + 0.01f);
	SwapSliderColors();

	float selectedX;
	if (CheckSelected() == 1)
		selectedX =  0.7f;
	else if (CheckSelected() == 2)
		selectedX =  0.0f;
	else if (CheckSelected() == 3)
		selectedX = -0.7f;
	else
		selectedX = -0.9f;

	m_selectedIndicator.SetPosition(m_camera->GetPosition().x - (selectedX * halfHeight), 
									m_camera->GetPosition().y + (0.9f * halfHeight));
	m_selectedIndicator.Update(delta);
	m_selectedIndicator.DrawObject(modelLoc, colorLoc);
	m_xVelSlider.SetPosition(m_camera->GetPosition().x - (0.7f * halfHeight), 
							 m_camera->GetPosition().y + (0.9f * halfHeight));
	m_xVelSlider.Update(delta);
	m_xVelSlider.DrawObject(modelLoc, colorLoc);
	//m_massSlider.SetPosition(m_camera->GetPosition().x,
	//						 m_camera->GetPosition().y + (0.9f * halfHeight));
	m_massSlider.SetPosition(m_camera->GetPosition().x,
		m_camera->GetPosition().y + (0.9f * halfHeight));
	m_massSlider.Update(delta);
	m_massSlider.DrawObject(modelLoc, colorLoc);
	m_yVelSlider.SetPosition(m_camera->GetPosition().x + (0.7f * halfHeight),
							 m_camera->GetPosition().y + (0.9f * halfHeight));
	m_yVelSlider.Update(delta);
	m_yVelSlider.DrawObject(modelLoc, colorLoc);
	m_zVelSlider.SetPosition(m_camera->GetPosition().x + (0.9f * halfHeight),
		m_camera->GetPosition().y + (0.9f * halfHeight));
	m_zVelSlider.Update(delta);
	m_zVelSlider.DrawObject(modelLoc, colorLoc);
	
	++m_timer;
	++m_timer2;
	++m_timer3;
	++m_timer4;
	UpdateSelected(window);
}

void Menu::IncSelectedRight()
{
	if (selectedSlider == &m_xVelSlider)
		selectedSlider = &m_massSlider;
	else if (selectedSlider == &m_massSlider)
		selectedSlider = &m_yVelSlider;
	else if (selectedSlider == &m_yVelSlider)
		selectedSlider = &m_zVelSlider;
	else
		selectedSlider = &m_xVelSlider;

	m_selectedIndicator.SetPosX(selectedSlider->GetPosX());
	m_selectedIndicator.SetPosY(selectedSlider->GetPosY());

}

void Menu::IncSelectedLeft()
{
	if (selectedSlider == &m_xVelSlider)
		selectedSlider = &m_zVelSlider;
	else if (selectedSlider == &m_massSlider)
		selectedSlider = &m_xVelSlider;
	else if (selectedSlider == &m_yVelSlider)
		selectedSlider = &m_massSlider;
	else
		selectedSlider = &m_yVelSlider;

	m_selectedIndicator.SetPosX(selectedSlider->GetPosX());
	m_selectedIndicator.SetPosY(selectedSlider->GetPosY());

}

void Menu::ToggleGravityAndInitVel(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS && m_timer3 > 20 && m_gravitySwitch)
	{
		m_gravitySwitch = false;
		m_timer3 = 0;
	}
	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS && m_timer3 > 20 && !m_gravitySwitch)
	{
		m_gravitySwitch = true;
		m_timer3 = 0;
	}
	// initial velocity
	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS && m_timer3 > 20 && m_initVelSwitch)
	{
		m_initVelSwitch = false;
		m_timer3 = 0;
	}
	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS && m_timer3 > 20 && !m_initVelSwitch)
	{
		m_initVelSwitch = true;
		m_timer3 = 0;
	}
}

void Menu::UpdateSelected(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS && m_timer > 20)
	{
		IncSelectedRight();
		m_timer = 0;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS && m_timer > 20)
	{
		IncSelectedLeft();
		m_timer = 0;
	}

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && m_timer4 > 5)
	{
		selectedSlider->IncMass( 0.005f);
		m_timer4 = 0;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && m_timer4 > 5 && selectedSlider->GetMass() > 0)
	{
		selectedSlider->IncMass(-0.005f);
		m_timer4 = 0;
	}

	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && m_timer2 > 20 && (selectedSlider == &m_xVelSlider || selectedSlider == &m_yVelSlider))
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

float SolveProjection(float& worldLeft, float& worldRight, float& worldBottom, float& worldTop, 
					 int projLoc, float width, float height, Camera& camera)
{
	float aspect = (float)width / (float)height;

	if (aspect >= 1.0f)
	{
		worldLeft = -aspect;
		worldRight = aspect;
		worldBottom = -1.0f;
		worldTop = 1.0f;
	}
	else
	{
		worldLeft = -1.0f;
		worldRight = 1.0f;
		worldBottom = -1.0f / aspect;
		worldTop = 1.0f / aspect;
	}

	float centerX = (worldLeft + worldRight) / 2.0f;
	float centerY = (worldBottom + worldTop) / 2.0f;
	float halfWidth = (worldRight - worldLeft) / 2.0f / camera.GetZoom();
	float halfHeight = (worldTop - worldBottom) / 2.0f / camera.GetZoom();

	worldLeft = centerX - halfWidth;
	worldRight = centerX + halfWidth;
	worldBottom = centerY - halfHeight;
	worldTop = centerY + halfHeight;

	glm::mat4 projection = glm::ortho(worldLeft, worldRight, worldBottom, worldTop);
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	return halfHeight;
}