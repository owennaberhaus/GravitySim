#define GLEW_STATIC


#include <glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <matrix_transform.hpp>
#include <type_ptr.hpp>
#include <iostream>
#include <vector>
#include <random>
#include <chrono>

#include "object.h"
#include "path.h"
#include "shader.h"
#include "menu.h"
#include "timer.h"
#include "clicker.h"
#include "camera.h"

int togglePathsTimer{ 0 };
int gravTimer{ 0 };

bool flag_drawPaths{ true }; // whether to draw the paths of the objects

float g_scrollDelta{ 0 };

static void TogglePaths(GLFWwindow* window, int& timer)
{
    ++timer;
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && timer > 10) {
        flag_drawPaths = !flag_drawPaths;
        timer = 0;
    }

}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    g_scrollDelta += static_cast<float>(yoffset);
}

int main(void)
{

    // Seed with current time
    unsigned int seed{ static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()) };
    std::mt19937 generator(seed);
    // Random distributions
    std::uniform_real_distribution<float> rPos(-0.9f, 0.9f);  // positions inside [-0.9, 0.9]
    std::uniform_real_distribution<float> rVel(-0.0001f, 0.0001f); // small velocities
    std::uniform_real_distribution<float> rMass(0.03f, 0.10f); // object sizes

    // set opengl to operate
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(600, 600, "schinitizel", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // vsync (60 fps)

    if (glewInit() != GLEW_OK)
        std::cout << "Glew initialization failed" << '\n';

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // lighten background slightly

    // initialize shaders (shader.h)
    unsigned int shader = CreateShader();
	unsigned int shader2D = CreateShader2D();
    // TODO :: create a genuniform function 
    // 3D uniforms
    int modelLoc = glGetUniformLocation(shader, "u_model"); // allows a translation matrix
    int colorLoc = glGetUniformLocation(shader, "u_color"); // accesses a glm::vec3 of rgb color
    int projLoc = glGetUniformLocation(shader, "u_projection"); // basically allows window scaling 
    int viewLoc = glGetUniformLocation(shader, "u_view"); // allows camera movement

    // 2D uniforms
    int modelLoc2D = glGetUniformLocation(shader2D, "u_model");
    int colorLoc2D = glGetUniformLocation(shader2D, "u_color");
    int projLoc2D = glGetUniformLocation(shader2D, "u_projection");


    int width, height; // used for projection matrix scaling
    float worldLeft, worldRight, worldBottom, worldTop; // absolute positions of window

    std::vector<std::unique_ptr<Object>> objects{};
    objects.reserve(20);

    int framesSinceClick{ 0 }; // a counter of how long since spawning in an item
                               // without it shit get wack :(
    glfwSetScrollCallback(window, scroll_callback); // prepare glfw for scroll input

    Timer timer;
    Clicker clicker;
    Camera camera(viewLoc);
    Menu menu(camera);
     
    std::cout << "right click to spawn in a movable object, left click for immovable\n" <<
        "Masses will be equal to the leftmost (blue) reference object\n" <<
        "second reference object is initial x velocity, then initial y velocity and finally initial z\n" <<
        "Green means positive velocity while red means negative\n" <<
        "Arrow keys to switch reference object, scroll to resize\n" << 
        "'B' to flip init vel sign, 'G' to turn off gravity, 'V' to toggle initial velocities\n" <<
        "WASD to move camera, scroll wheel to zoom in and out\n" << 
        "+ and - to increase and reduce the gravitational constant\n" <<
        "objects will spawn on the plane normal to the camera direciton, that crosses (0, 0, 0)";

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        TogglePaths(window, togglePathsTimer);

        float deltaTime = timer.delta();
        camera.Update(window);

        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        float halfHeight = SolveProjection(worldLeft, worldRight, worldBottom, worldTop, projLoc, width, height, camera);  
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        float worldX = worldLeft + (xpos / width) * (worldRight - worldLeft); // should move these to some other function
        float worldY = worldTop - (ypos / height) * (worldTop - worldBottom);
        clicker.MouseControl(window, worldX / camera.GetRadius(), worldY / camera.GetRadius(), objects, menu, g_scrollDelta, camera);
        menu.ToggleGravityAndInitVel(window);

        glUseProgram(shader2D);
        menu.UpdateAndDrawMenu(modelLoc2D, colorLoc2D, projLoc2D, window, deltaTime, halfHeight, width, height);
        
        ApplyGravity2(objects, deltaTime, menu);
        glUseProgram(shader);
        // update state for all rendered objects
        if (flag_drawPaths)
        {
            for (auto& obj : objects)
            {
                obj->UpdatePath();
                obj->DrawPath(modelLoc, colorLoc);
            }
        }
        for (auto& obj : objects)
        {
            obj->Update(deltaTime);
            obj->DrawObject(modelLoc, colorLoc);
        }
		

        IncrementGravity(window, gravTimer);
        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}