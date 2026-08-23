// Runs the SHIPPED presets through the SHIPPED physics: real Object, real
// ApplyGravity2, real presets::Load. Nothing here reimplements the algorithm.
#include "platform_gl.h"
#include <GLFW/glfw3.h>
#include <cstdio>
#include <cmath>
#include <vector>
#include <limits>
#include <algorithm>

#include "object.h"
#include "camera.h"
#include "menu.h"
#include "shader.h"
#include "atomscene.h"
#include "presets.h"

int main()
{
    if (!glfwInit()) { printf("glfwInit failed\n"); return 1; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* w = glfwCreateWindow(64, 64, "harness", nullptr, nullptr);
    if (!w) { printf("window failed\n"); return 1; }
    glfwMakeContextCurrent(w);
    if (glewInit() != GLEW_OK) { printf("glew failed\n"); return 1; }

    Shader shader;
    Camera camera(shader.GetShader(), (int)shader.GetViewLoc(), (int)shader.GetProjLoc());
    Menu menu(camera);
    AtomScene atom(shader);

    const float dt = 1.0f / 60.0f;
    const float horizon = 180.0f;             // three minutes of wall clock
    const int steps = (int)(horizon / dt);

    printf("\n%-3s %-17s %3s %9s %8s %7s %7s %6s  %s\n",
           "key", "preset", "n", "clearance", "maxR", "dE", "band", "escape", "verdict");
    printf("%s\n", "--------------------------------------------------------------------------------");

    int bad = 0;
    for (int key = 1; key <= 9; ++key)
    {
        std::vector<std::unique_ptr<Object>> objects;
        presets::Load(key, false, objects, camera, atom);
        const int n = (int)objects.size();

        // Total energy of the system as the sim actually computes forces, i.e.
        // with the softened potential -M_G m1 m2 / sqrt(r^2 + s^2).
        auto energy = [&]() {
            const float s2 = 0.05f * 0.05f;
            double e = 0.0;
            for (size_t i = 0; i < objects.size(); ++i)
            {
                if (objects[i]->GetMovable())
                    e += 0.5 * objects[i]->GetMass() * glm::dot(objects[i]->GetVel(), objects[i]->GetVel());
                for (size_t k = i + 1; k < objects.size(); ++k)
                {
                    if (!objects[i]->GetExertsGravity() || !objects[k]->GetExertsGravity()) continue;
                    float d = glm::length(objects[k]->GetPos() - objects[i]->GetPos());
                    e -= M_G * objects[i]->GetMass() * objects[k]->GetMass() / std::sqrt(d * d + s2);
                }
            }
            return e;
        };

        double e0 = energy();
        float clearance = std::numeric_limits<float>::max();  // min (distance - contact)
        float maxR = 0.0f;

        // Energy only means anything when every body both moves and pulls. An
        // immovable star is an infinite reservoir, and a test particle
        // contributes kinetic energy with no matching potential term - either
        // one makes the total wander for reasons that are not integration error.
        bool closed = true;
        for (auto& o : objects)
            if (!o->GetMovable() || !o->GetExertsGravity()) closed = false;

        // So the general stability check is per body: the band of radii each one
        // sweeps early in the run against the band it sweeps at the end. A stable
        // orbit keeps the same band; a decaying or growing one does not.
        const int quarter = steps / 4;
        std::vector<float> loA(n, 1e30f), hiA(n, 0.0f), loB(n, 1e30f), hiB(n, 0.0f);

        for (int s = 0; s < steps; ++s)
        {
            ApplyGravity2(objects, dt, menu);
            for (auto& o : objects) o->Update(dt, w);

            for (size_t i = 0; i < objects.size(); ++i)
            {
                float r = glm::length(objects[i]->GetPos());
                if (r > maxR) maxR = r;

                if (s < quarter)            { loA[i] = std::min(loA[i], r); hiA[i] = std::max(hiA[i], r); }
                else if (s >= steps - quarter) { loB[i] = std::min(loB[i], r); hiB[i] = std::max(hiB[i], r); }

                for (size_t k = i + 1; k < objects.size(); ++k)
                {
                    float gap = glm::length(objects[k]->GetPos() - objects[i]->GetPos())
                              - (objects[i]->GetMass() + objects[k]->GetMass());
                    if (gap < clearance) clearance = gap;
                }
            }
        }

        float band = 0.0f;
        for (int i = 0; i < n; ++i)
        {
            float spanA = hiA[i] - loA[i], spanB = hiB[i] - loB[i];
            float scale = std::max(hiA[i], 0.05f);
            band = std::max(band, std::fabs(spanB - spanA) / scale);
            band = std::max(band, std::fabs(hiB[i] - hiA[i]) / scale);
        }

        double drift = closed
            ? ((std::fabs(e0) > 1e-9) ? std::fabs((energy() - e0) / e0) : 0.0)
            : -1.0;

        int escaped = 0;
        for (auto& o : objects)
            if (glm::length(o->GetPos()) > 40.0f) ++escaped;

        const char* verdict = "ok";
        if (clearance < 0.0f)   { verdict = "collides"; }
        if (escaped > 0)        { verdict = (clearance < 0.0f) ? "collides+ejects" : "ejects"; }
        // Preset 8 is meant to collide and scatter; everything else is not.
        bool expected = (key == 8);
        if (!expected && band > 0.35f && verdict[0] == 'o') { verdict = "FAIL drift"; ++bad; }
        else if (!expected && verdict[0] != 'o') { verdict = (clearance < 0.0f) ? "FAIL collide" : "FAIL eject"; ++bad; }

        char de[16];
        if (drift < 0.0) snprintf(de, sizeof(de), "%7s", "-");
        else             snprintf(de, sizeof(de), "%7.4f", drift);

        printf("%-3d %-17s %3d %9.3f %8.2f %s %7.3f %6d  %s\n",
               key, presets::CurrentName(), n, clearance, maxR, de, band, escaped, verdict);
        fflush(stdout);
    }

    printf("\n%d preset(s) failed\n", bad);
    glfwTerminate();
    return bad == 0 ? 0 : 1;
}
