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
        static void setCallbackFunctions();
        static void processInput(GLFWwindow* window);

        static bool x_pressed;
        static uint32_t render_mode;

        static void key_press_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
        static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
        static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
        static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    };
}