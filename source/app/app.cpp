#include "StarSystemSim/app/app.h"
#include "StarSystemSim/utilities/error.h"
#include "StarSystemSim/graphics/renderer.h"

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <glm/vec2.hpp>

#include <cstdlib>

App* App::s_Instance = nullptr;
extern graphics::Renderer* renderer;

void App::start(uint32_t width, uint32_t height) {
	if (s_Instance != nullptr) {
		delete s_Instance;
        utils::printError("Changing App Instance");
	}

	s_Instance = new App(width, height);
}

void App::clear() {
    delete s_Instance;
    s_Instance = nullptr;
    glfwTerminate();
}

App::App(uint32_t windowWidth, uint32_t windowHeight)
    : mainCamera(glm::vec3(0.0f, 0.0f, 0.0f)),
    mousePos(0.0f, 0.0f)
{
    m_ScrWidth = windowWidth;
    m_ScrHeight = windowHeight;

    isCursorVisible = false;
    isFirstMouseMovement = true;


    // setting up window and OpenGL
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    window = glfwCreateWindow(m_ScrWidth, m_ScrHeight, "StarSystemSim", NULL, NULL);
    if (window == NULL) {
        utils::printError("Failed to create GLFW window");
        glfwTerminate();
        exit(-1);
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, app::framebuffer_size_callback);
    glfwSetKeyCallback(window, app::key_press_callback);
    glfwSetCursorPosCallback(window, app::mouse_callback);
    glfwSetScrollCallback(window, app::scroll_callback);
    glfwSetWindowPos(window, 20, 60);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        utils::fatalError("Failed to initialize GLAD");
    }

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
}

App::~App() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(this->window);
}

void App::resize(uint32_t width, uint32_t height) {
    m_ScrWidth = width;
    m_ScrHeight = height;
}

namespace app {
    void key_press_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        //if (glfwGetKey(window, GLFW_KEY_GRAVE_ACCENT) == GLFW_PRESS) {
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
            if (App::s_Instance->isCursorVisible) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                App::s_Instance->isFirstMouseMovement = true;
                App::s_Instance->isCursorVisible = false;
            }
            else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                App::s_Instance->isCursorVisible = true;
            }
        }
    }

    void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
        if (App::s_Instance->isFirstMouseMovement) {
            App::s_Instance->mousePos = glm::vec2(xpos, ypos);
            App::s_Instance->isFirstMouseMovement = false;
        }
        
        glm::vec2& mousePos = App::s_Instance->mousePos;
        
        glm::vec2 offset = glm::vec2(xpos - mousePos.x, mousePos.y - ypos);
        mousePos = glm::vec2(xpos, ypos);
        
        float sensitivity = 0.1f;
        offset *= sensitivity;
        
        if (!App::s_Instance->isCursorVisible) {
            graphics::Camera& camera = App::s_Instance->mainCamera;
            camera.yaw += offset.x;
            camera.pitch += offset.y;
        
            if (camera.pitch > 89.0f)
                camera.pitch = 89.0f;
            if (camera.pitch < -89.0f)
                camera.pitch = -89.0f;
        }
    }

    void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
        App::s_Instance->mainCamera.fov -= (float)yoffset;
        if (App::s_Instance->mainCamera.fov < 1.0f)
            App::s_Instance->mainCamera.fov = 1.0f;
        if (App::s_Instance->mainCamera.fov > 45.0f)
            App::s_Instance->mainCamera.fov = 45.0f;
    }

    void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
        App::s_Instance->resize(width, height);
        renderer->resize();
    }
}