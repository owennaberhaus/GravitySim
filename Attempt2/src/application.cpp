#include "platform_gl.h"
#include <GLFW/glfw3.h>
#include "gamestate.h"

#ifdef __EMSCRIPTEN__
// The canvas is sized by CSS. Keep the drawing buffer matched to it, capping the
// device pixel ratio so a phone at 3x does not render nine times the pixels.
//
// This has to go through glfwSetWindowSize, not emscripten_set_canvas_element_size.
// The GLFW web shim keeps its own copy of the window size, and both
// glfwGetFramebufferSize and glfwGetWindowSize read that copy rather than asking
// the canvas. Resizing the canvas behind its back leaves the shim reporting
// whatever glfwCreateWindow was handed, so the viewport stays a small patch in
// the corner of a much larger buffer and every mouse ray is scaled against the
// wrong width.
static void MatchCanvasToCss(GLFWwindow* window)
{
    double cssW = 0.0, cssH = 0.0;
    emscripten_get_element_css_size("#canvas", &cssW, &cssH);

    double dpr = emscripten_get_device_pixel_ratio();
    if (dpr > 2.0)
        dpr = 2.0;

    int wantW = static_cast<int>(cssW * dpr);
    int wantH = static_cast<int>(cssH * dpr);
    if (wantW <= 0 || wantH <= 0)
        return;

    // The shim reads a size that exactly matches the screen as a request to go
    // fullscreen, which resizes the canvas again and oscillates. Give up one
    // pixel of height rather than trip it.
    int screenW = 0, screenH = 0;
    emscripten_get_screen_size(&screenW, &screenH);
    if (wantW == screenW && wantH == screenH)
        wantH -= 1;

    int haveW = 0, haveH = 0;
    glfwGetFramebufferSize(window, &haveW, &haveH);
    if (wantW != haveW || wantH != haveH)
        glfwSetWindowSize(window, wantW, wantH);
}
#endif

static void Frame(void* arg)
{
    GameState* game = static_cast<GameState*>(arg);

#ifdef __EMSCRIPTEN__
    MatchCanvasToCss(game->GetWindow());
#endif

    game->update();
    game->render();
    game->Pause();
}

int main(void)
{

    // initialize all the things that need to be initialized for the game to run
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        std::exit(EXIT_FAILURE);
    }
    // set opengl to operate (ver 3.3)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24); // asked for explicitly - the web shim decides per hint

    // Create a windowed mode window and its OpenGL context 
    GLFWwindow* window = glfwCreateWindow(600, 600, "3D Gravity Simulator", NULL, NULL);
    if (!window)
    {
        std::cerr << "Failed to initialize GLFW window\n";
        std::exit(EXIT_FAILURE);
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    glfwSwapInterval(1); // vsync (constnant 60 fps)

#ifndef __EMSCRIPTEN__
    if (glewInit() != GLEW_OK) // init glew (must be done after creating window and context)
        std::cout << "Glew initialization failed" << '\n';
#endif

    // Heap allocated because on the web main() returns immediately and the frame
    // callback keeps running afterwards, so the game cannot live on this stack.
    GameState* game = new GameState(window);
    game->PrintTutorial();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(Frame, game, 0, 1); // 0 fps = drive off requestAnimationFrame
#else
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(game->GetWindow()))
    {
        Frame(game);
    }
    delete game;
#endif

    return 0;
}
