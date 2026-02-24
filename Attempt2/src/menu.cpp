#include "menu.h"

Menu::Menu()
{

}
Menu::~Menu()
{

}

void Menu::UpdateAndDrawMenu(int modelLoc, int colorLoc, GLFWwindow* window, float delta)
{
	m_selectedIndicator.SetMass(selectedSlider->GetMass() + 0.01f);
	SwapSliderColors();

	m_selectedIndicator.Update(delta);
	m_selectedIndicator.DrawObject(modelLoc, colorLoc);
	m_xVelSlider.Update(delta);
	m_xVelSlider.DrawObject(modelLoc, colorLoc);
	m_massSlider.Update(delta);
	m_massSlider.DrawObject(modelLoc, colorLoc);
	m_yVelSlider.Update(delta);
	m_yVelSlider.DrawObject(modelLoc, colorLoc);
	
	++m_timer;
	++m_timer2;
	++m_timer3;
	UpdateSelected(window);
}

void Menu::IncSelectedRight()
{
	if (selectedSlider == &m_xVelSlider)
		selectedSlider = &m_massSlider;
	else if (selectedSlider == &m_massSlider)
		selectedSlider = &m_yVelSlider;
	else
		selectedSlider = &m_xVelSlider;

	m_selectedIndicator.SetPosX(selectedSlider->GetPosX());
	m_selectedIndicator.SetPosY(selectedSlider->GetPosY());

}

void Menu::IncSelectedLeft()
{
	if (selectedSlider == &m_xVelSlider)
		selectedSlider = &m_yVelSlider;
	else if (selectedSlider == &m_massSlider)
		selectedSlider = &m_xVelSlider;
	else
		selectedSlider = &m_massSlider;

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

}

void DeleteObjects(std::vector<std::unique_ptr<Object>>& objects, GLFWwindow* window, float worldX, float worldY)
{
	for (size_t i{ 0 }; i < objects.size(); ++i)
	{
		float distanceMTO = static_cast<float>(sqrt(pow((objects[i]->GetPosX() - worldX), 2) + pow((objects[i]->GetPosY() - worldY), 2))); //mouse to object
		if (distanceMTO < (objects[i]->GetMass() + 0.01) && glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		{
			objects.erase(objects.begin() + i);
			--i;
		}

	}
}

void SolveProjection(float& worldLeft, float& worldRight, float& worldBottom, float& worldTop, int projLoc, float width, float height)
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

	glm::mat4 projection;
	if (aspect >= 1.0f)
	{
		// Wide screen case
		projection = glm::ortho(-aspect, aspect, -1.0f, 1.0f);
	}
	else
	{
		// Tall screen case
		projection = glm::ortho(-1.0f, 1.0f, -1.0f / aspect, 1.0f / aspect);
	}

	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
}