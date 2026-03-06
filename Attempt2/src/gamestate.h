#pragma once
#include <glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <matrix_transform.hpp>
#include <type_ptr.hpp>
#include <iostream>
#include <vector>
#include <random>
#include <chrono>

#include "camera.h"
#include "clicker.h"
#include "shader.h"
#include "timer.h"

class GameState
{
private:
	


public:
	GameState(GLFWwindow* window);
	virtual ~GameState();
	void update();
	void render();

	GLFWwindow* GetWindow() { return m_window; }
	void PrintTutorial();
	bool GetPaused() { return m_paused; }
	void SetDeltaTime(float delta) { m_deltaTime = delta; }	
	void Pause();

private: 
	Shader m_shader;
	Camera m_camera;
	Menu m_menu;
	Timer m_gameTimer;
	Clicker m_clicker;
	// vector that all objects are dynamically allocated to
	std::vector<std::unique_ptr<Object>> m_objects{};

	GLFWwindow* m_window;

	// window state || world dimension
	int m_width{ 0 };  // used for projection matrix scaling
	int m_height{ 0 };
	float m_worldLeft{ 0.0f }; // absolute positions of window - soon depricated
	float m_worldRight{ 0.0f };
	float m_worldBottom{ 0.0f };
	float m_worldTop{ 0.0f };
	float m_deltaTime{ 0.0f }; // time between frames

	double m_xpos{ 0.0f }; // for mouse position
	double m_ypos{ 0.0f };

	float m_worldX{ 0.0f };
	float m_worldY{ 0.0f };
	bool m_paused{ false };
	bool m_escapeWasPressed{ false };
	float m_pauseTimer{ 0.0f };
};