// How long does a rebuild actually take, and how big is the mesh? Real AtomScene.
#include "platform_gl.h"
#include <GLFW/glfw3.h>
#include <cstdio>
#include <chrono>
#include <vector>
#include "shader.h"
#include "camera.h"
#include "atomscene.h"
#include "presets.h"
#include "object.h"

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* w = glfwCreateWindow(64, 64, "c", nullptr, nullptr);
    glfwMakeContextCurrent(w);
    glewInit();

    Shader shader;
    Camera camera(shader.GetShader(), (int)shader.GetViewLoc(), (int)shader.GetProjLoc());
    AtomScene atom(shader);
    std::vector<std::unique_ptr<Object>> dummy;

    const int z[9]     = { 1, 6, 7, 10, 26, 29, 36, 64, 92 };
    const char* nm[9]  = { "H", "C", "N", "Ne", "Fe", "Cu", "Kr", "Gd", "U" };

    printf("\n%-4s %-4s %10s\n", "key", "el", "rebuild");
    printf("-----------------------------\n");
    double worst = 0.0; const char* worstName = "";
    for (int i = 0; i < 9; ++i)
    {
        auto t0 = std::chrono::steady_clock::now();
        presets::Load(i + 1, true, dummy, camera, atom);
        atom.Update(w, camera);          // triggers Rebuild via m_dirty
        auto t1 = std::chrono::steady_clock::now();
        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        if (ms > worst) { worst = ms; worstName = nm[i]; }
        printf("%-4d %-4s %8.1f ms\n", i + 1, nm[i], ms);
    }
    printf("\nworst: %s at %.0f ms\n", worstName, worst);
    glfwTerminate();
    return 0;
}
