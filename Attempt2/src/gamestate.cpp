#include "gamestate.h"
#include "presets.h"

GameState::GameState(GLFWwindow* window) :
    m_shader(),
    m_window(window),
	m_camera(m_shader.GetShader(), static_cast<int>(m_shader.GetViewLoc()), static_cast<int>(m_shader.GetProjLoc())),
    m_menu(m_camera),
    m_atom(m_shader),
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
        "WASD to orbit camera, Q/E to roll, 'R' to level out, scroll wheel to zoom\n" <<
        "hover an object (it brightens) and press SPACE to delete it\n" <<
        "+ and - to increase and reduce the gravitational constant\n" <<
        "objects will spawn on the plane normal to the camera direciton, that crosses (0, 0, 0)" <<
        "esc key to pause the whole simulation\n" <<
        "tab to switch between the gravity sim and quantum mode\n" << 
        "When in quantum mode, use '[' and ']' to adjust the statistical area of electron presence rendered\n";

    m_atom.PrintControls();
}

void GameState::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear the screen each frame to draw on a blank canvas

    if (m_mode == Mode::Quantum)
    {
        m_atom.Render(m_camera);
        DrawHud();
        glfwSwapBuffers(m_window);
        glfwPollEvents();
        return;
    }

    // The slider orbs are a control surface. With input off they are decoration
    // that covers part of the scene, so mobile skips them entirely.
    if (!presets::MobileMode())
    {
        glUseProgram(m_shader.GetShader2D());
        m_menu.UpdateAndDrawMenu(m_shader.GetModelLoc2D(), m_shader.GetColorLoc2D(), m_shader.GetProjLoc2D(), m_window, m_deltaTime, m_width, m_height);
    }

    ApplyGravity2(m_objects, m_deltaTime, m_menu);
    glUseProgram(m_shader.GetShader());
    // update state for all rendered objects


    for (auto& obj : m_objects)
    {
        obj->UpdatePath(m_window, m_deltaTime);
        obj->DrawPath(m_shader.GetModelLoc(), m_shader.GetColorLoc());
    }
    // The object under the cursor draws brighter, so it's obvious what SPACE
    // is about to delete.
    const int hovered = m_clicker.GetHoveredIndex();
    for (size_t i = 0; i < m_objects.size(); ++i)
    {
        m_objects[i]->Update(m_deltaTime, m_window);
        m_objects[i]->DrawObject(m_shader.GetModelLoc(), m_shader.GetColorLoc(),
            static_cast<int>(i) == hovered);
    }

    DrawHud();

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

    // A stall - alt-tab, a breakpoint, or the first frame after the wasm module
    // finishes loading - hands back a delta worth many frames. Integrating that
    // in one step throws everything off screen, so cap it.
    if (m_deltaTime > kMaxFrameStep)
        m_deltaTime = kMaxFrameStep;

    // Framebuffer size drives the viewport and the projection aspect ratio.
    glfwGetFramebufferSize(m_window, &m_width, &m_height); // snag new window dimensions
    glViewport(0, 0, m_width, m_height); // then update opengl viewport to avoid stretching

    // Camera now owns the projection matrix, so it needs the size first.
    m_camera.Update(m_window, m_width, m_height);

    bool tabPressed = glfwGetKey(m_window, GLFW_KEY_TAB) == GLFW_PRESS;
    if (tabPressed && !m_tabWasPressed)
        m_mode = (m_mode == Mode::Classical) ? Mode::Quantum : Mode::Classical;
    m_tabWasPressed = tabPressed;

    // On a touch device the sim runs itself: no spawning, no sliders, no camera
    // keys. One tap advances the scene, and that is the whole interface.
    if (presets::MobileMode())
    {
        bool quantum = (m_mode == Mode::Quantum);
        presets::UpdateMobile(m_window, m_deltaTime, quantum, m_objects, m_camera, m_atom);
        m_mode = quantum ? Mode::Quantum : Mode::Classical;

        // The camera already built its view matrix above, before the spin moved
        // it. Rebuild so the frame that renders is the frame that was aimed.
        m_camera.Update(m_window, m_width, m_height);

        // Refresh rather than Update: the atom still has to rebuild when the
        // element changes, but it must not read the mouse.
        if (m_mode == Mode::Quantum)
            m_atom.Refresh(m_camera);
        return;
    }

    // The number row loads canned scenes. Handled before either mode runs so a
    // quantum preset marks the atom dirty in time for this frame's rebuild.
    presets::Update(m_window, m_deltaTime, m_mode == Mode::Quantum, m_objects, m_camera, m_atom);

    if (m_mode == Mode::Quantum)
    {
        m_atom.Update(m_window, m_camera);
        glfwGetCursorPos(m_window, &m_xpos, &m_ypos);
        m_atom.UpdateHover(m_window, m_xpos, m_ypos, m_camera);
        return;
    }

    // world-space bounds of the spawn plane
    SolveProjection(m_worldLeft, m_worldRight, m_worldBottom, m_worldTop, static_cast<float>(m_width), static_cast<float>(m_height), m_camera);

    glfwGetCursorPos(m_window, &m_xpos, &m_ypos); // grab current mouse position

    // Cursor coordinates are in WINDOW space, which is not the same as the
    // framebuffer size on a scaled display - normalise against the right one.
    int winW{ 0 }, winH{ 0 };
    glfwGetWindowSize(m_window, &winW, &winH);
    if (winW > 0 && winH > 0)
    {
        m_worldX = m_worldLeft + static_cast<float>(m_xpos / winW) * (m_worldRight - m_worldLeft);
        m_worldY = m_worldTop - static_cast<float>(m_ypos / winH) * (m_worldTop - m_worldBottom);
    }

    // Spawning or deleting means the scene is no longer the preset that was
    // loaded, so the label stops claiming otherwise.
    const size_t countBefore = m_objects.size();
    m_clicker.UpdateHoverAndDelete(m_objects, m_window, m_xpos, m_ypos, m_camera);
    m_clicker.MouseControl(m_window, m_worldX / m_camera.GetRadius(), m_worldY / m_camera.GetRadius(), m_objects, m_menu, g_scrollDelta, m_camera);
    if (m_objects.size() != countBefore)
        presets::Invalidate();
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

void GameState::DrawHud()
{
    // Smoothed so the number is readable rather than flickering every frame
    if (m_deltaTime > 0.0f)
        m_fps = m_fps * 0.94f + (1.0f / m_deltaTime) * 0.06f;

    const float scale = 2.0f;
    const glm::vec3 label(0.62f, 0.72f, 0.90f);
    const glm::vec3 value(1.0f, 1.0f, 1.0f);

    m_text.Begin(m_width, m_height);

    if (m_mode == Mode::Quantum)
    {
        m_text.DrawAnchored("QUANTUM", TextRenderer::Anchor::TopRight, 0, scale, label);

        const std::vector<std::string>& lines = m_atom.HudLines();
        for (size_t i = 0; i < lines.size(); ++i)
            m_text.DrawAnchored(lines[i], TextRenderer::Anchor::TopRight,
                static_cast<int>(i) + 1, scale, value);

        const std::string preset = presets::CurrentName();
        if (!preset.empty())
            m_text.DrawAnchored(preset, TextRenderer::Anchor::TopRight,
                static_cast<int>(lines.size()) + 1, scale, glm::vec3(1.0f, 0.85f, 0.45f));

        const std::string& selected = m_atom.SelectedLabel();
        if (!selected.empty())
            m_text.DrawAnchored("selected: " + selected, TextRenderer::Anchor::TopLeft,
                0, scale, glm::vec3(1.0f, 0.85f, 0.45f));
    }
    else
    {
        char buffer[128];
        snprintf(buffer, sizeof(buffer), "objects %d   G=%.2f", static_cast<int>(m_objects.size()), M_G);

        if (!presets::MobileMode())
            DrawSliderLabels(label);
        m_text.DrawAnchored("GRAVITY", TextRenderer::Anchor::TopRight, 0, scale, label);
        m_text.DrawAnchored(buffer, TextRenderer::Anchor::TopRight, 1, scale, value);

        int line = 2;
        const std::string preset = presets::CurrentName();
        if (!preset.empty())
            m_text.DrawAnchored(preset, TextRenderer::Anchor::TopRight, line++, scale, glm::vec3(1.0f, 0.85f, 0.45f));
        if (m_paused)
            m_text.DrawAnchored("PAUSED", TextRenderer::Anchor::TopRight, line, scale, glm::vec3(1.0f, 0.7f, 0.3f));
    }

    char fps[32];
    snprintf(fps, sizeof(fps), "%.0f fps", m_fps);
    m_text.DrawAnchored(fps, TextRenderer::Anchor::BottomRight, 0, 1.5f, label);
    if (presets::MobileMode())
    {
        m_text.DrawAnchored("tap for the next scene", TextRenderer::Anchor::BottomLeft, 0, 1.5f, label);
    }
    else
    {
        m_text.DrawAnchored("tab switches mode", TextRenderer::Anchor::BottomLeft, 0, 1.5f, label);
        m_text.DrawAnchored("1-9 presets, 0 clears", TextRenderer::Anchor::BottomLeft, 1, 1.5f, label);
    }

    m_text.End();
}

void GameState::DrawSliderLabels(const glm::vec3& color)
{
    if (m_height <= 0)
        return;

    // The slider orbs are drawn through the menu's ortho projection, which spans -aspect to aspect across and -1..1 down
    const float unit = m_height * 0.5f;
    const float aspect = static_cast<float>(m_width) / static_cast<float>(m_height);
    const float labelScale = 1.5f;
    const glm::vec3 highlight(1.0f, 0.85f, 0.45f);

    std::vector<Menu::SliderLabel> sliders = m_menu.GetSliderLabels();
    for (size_t i = 0; i < sliders.size(); ++i)
    {
        const Menu::SliderLabel& s = sliders[i];

        float centreX = s.x * unit + m_width * 0.5f;
        float belowY = (1.0f - s.y) * unit + s.radius * unit + 8.0f;

        m_text.Draw(s.text, centreX - m_text.TextWidth(s.text, labelScale) * 0.5f, belowY,
            labelScale, s.selected ? highlight : color);
    }
}
