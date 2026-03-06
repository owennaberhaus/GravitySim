#include <glew.h>
#include <GLFW/glfw3.h>
#include "gamestate.h"

int main(void)
{

    // In gamestate constructor, initialize all the things that need to be initialized for the game to run (window, opengl, etc)
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        std::exit(EXIT_FAILURE);
    }
    // set opengl to operate (ver 3.3)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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

    if (glewInit() != GLEW_OK) // init glew (must be done after creating window and context)
        std::cout << "Glew initialization failed" << '\n';
    
	GameState game(window); // initialize game state (window, shaders, etc)
    game.PrintTutorial();

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(game.GetWindow()))
    {
        /* Render here */
        game.update();
        game.render();
        game.Pause();

    }

    return 0;
}