#pragma once

#include "StarSystemSim/app/app.h"
#include "StarSystemSim/utilities/timer.h"

#include <GLFW/glfw3.h>

namespace app {
    class EventManager {
    public:
        static void setCallbackFunctions();
        static void processInput(GLFWwindow* window);

        static void key_press_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
        static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
        static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
        static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    };
}