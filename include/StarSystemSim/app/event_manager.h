#pragma once

#include "StarSystemSim/app/app.h"
#include "StarSystemSim/utilities/timer.h"

#include <GLFW/glfw3.h>

namespace app {
    enum class RenderMode {
        FACES, EDGES, VERTICES
    };

    class EventManager {
    public:
        static void processInput(GLFWwindow* window);

        static bool x_pressed;
        static uint32_t render_mode;
    };
}