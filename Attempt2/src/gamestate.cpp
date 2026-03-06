#include "gamestate.h"

GameState::GameState(GLFWwindow* window) :
    m_shader(),
    m_window(window),
	m_camera(m_shader.GetViewLoc()),
    m_menu(m_camera),
	m_width(0),
	m_height(0),
	m_deltaTime(0.0f),
	m_worldLeft(0.0f),
	m_worldTop(0.0f),
    m_worldBottom(0.0f),
	m_worldRight(0.0f),
	m_xpos(0.0),
	m_ypos(0.0),
	m_worldX(0.0f),
	m_worldY(0.0f)
{

    glEnable(GL_DEPTH_TEST); // depth testing for 3D rendering
    glDepthFunc(GL_LESS); // prevents drawing things that are behind other things (always draws closest object to camera)
    // TODO: add some sort of actual world rendering instead of the entire background being uniform grey
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // lighten background slightly - could be changed

	m_objects.reserve(20); // assume user makes less than 20 objects, but vector is dynamic

    glfwSetScrollCallback(m_window, scroll_callback); // prepare glfw for scroll input

	m_gameTimer.reset(); // initialize timer 

}

GameState::~GameState()
{
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void GameState::PrintTutorial()
{
    std::cout << "right click to spawn in a movable object, left click for immovable\n" <<
        "Masses will be equal to the leftmost (blue) reference object\n" <<
        "second reference object is initial x velocity, then initial y velocity and finally initial z\n" <<
        "Green means positive velocity while red means negative\n" <<
        "Arrow keys to switch reference object and to resize\n" <<
        "'B' to flip init vel sign, 'G' to turn off gravity, 'V' to toggle initial velocities\n" <<
        "WASD to move camera, scroll wheel to zoom in and out\n" <<
        "+ and - to increase and reduce the gravitational constant\n" <<
        "objects will spawn on the plane normal to the camera direciton, that crosses (0, 0, 0)" <<
        "esc key to pause the whole simulation\n";
}

void GameState::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear the screen each frame to draw on a blank canvas

    glUseProgram(m_shader.GetShader2D());
    m_menu.UpdateAndDrawMenu(m_shader.GetModelLoc2D(), m_shader.GetColorLoc2D(), m_shader.GetProjLoc2D(), m_window, m_deltaTime, m_width, m_height);

    ApplyGravity2(m_objects, m_deltaTime, m_menu);
    glUseProgram(m_shader.GetShader());
    // update state for all rendered objects


    for (auto& obj : m_objects)
    {
        obj->UpdatePath(m_window, m_deltaTime);
        obj->DrawPath(m_shader.GetModelLoc(), m_shader.GetColorLoc());
    }
    for (auto& obj : m_objects)
    {
        obj->Update(m_deltaTime, m_window);
        obj->DrawObject(m_shader.GetModelLoc(), m_shader.GetColorLoc());
    }

    /* Swap front and back buffers */
    glfwSwapBuffers(m_window);

    /* Poll for and process events */
    glfwPollEvents();
    
}

void GameState::update()
{
    if (!m_paused)
        m_deltaTime = m_gameTimer.delta(); // each frame pull the time since last frame
    else
        m_deltaTime = 0.0f;
    m_gameTimer.reset(); // then immidiately reset for next frame

    m_camera.Update(m_window);
    glfwGetFramebufferSize(m_window, &m_width, &m_height); // snag new window dimensions
    glViewport(0, 0, m_width, m_height); // then update opengl viewport to avoid stretching

    // updates projection matrix so that things look about right based on camera position
    SolveProjection(m_worldLeft, m_worldRight, m_worldBottom, m_worldTop, m_shader.GetProjLoc(), m_width, m_height, m_camera);

    glfwGetCursorPos(m_window, &m_xpos, &m_ypos); // grab current mouse position

    m_worldX = m_worldLeft + (m_xpos / m_width) * (m_worldRight - m_worldLeft); // should move these to some other function
    m_worldY = m_worldTop - (m_ypos / m_height) * (m_worldTop - m_worldBottom);
    m_clicker.DeleteObjects(m_objects, m_window, m_xpos, m_ypos, m_width, m_height, m_camera);
    m_clicker.MouseControl(m_window, m_worldX / m_camera.GetRadius(), m_worldY / m_camera.GetRadius(), m_objects, m_menu, g_scrollDelta, m_camera);
    m_menu.ToggleGravityAndInitVel(m_window);

    IncrementGravity(m_window, m_deltaTime);



}

void GameState::Pause()
{
    bool escapePressed = glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS;

    if (escapePressed && !m_escapeWasPressed)
    {
        m_paused = !m_paused;

        if (m_paused)
            std::cout << "paused\n";
        else
            std::cout << "play\n";
    }

    m_escapeWasPressed = escapePressed;
}