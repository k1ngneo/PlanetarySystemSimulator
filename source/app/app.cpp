#include "StarSystemSim/app/app.h"
#include "StarSystemSim/app/event_manager.h"
#include "StarSystemSim/utilities/error.h"
#include "StarSystemSim/graphics/renderer.h"

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <glm/vec2.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cstdlib>

utils::Timer App::mainTimer;
utils::Timer App::frameClock;
App* App::s_Instance = nullptr;
GLFWwindow* App::s_Window = nullptr;

void App::start(uint32_t width, uint32_t height) {
	if (s_Instance != nullptr) {
		delete s_Instance;
        utils::printError("Changing App Instance");
	}

    // setting up window and OpenGL
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    s_Window = glfwCreateWindow(width, height, "StarSystemSim", NULL, NULL);
    if (s_Window == NULL) {
        utils::printError("Failed to create GLFW window");
        glfwTerminate();
        exit(-1);
    }
    glfwMakeContextCurrent(s_Window);
    glfwSetFramebufferSizeCallback(s_Window, app::framebuffer_size_callback);

    app::EventManager::setCallbackFunctions();

    glfwSetWindowPos(s_Window, 20, 60);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        utils::fatalError("Failed to initialize GLAD");
    }

    ImGui_ImplGlfw_InitForOpenGL(s_Window, true);
    ImGui_ImplOpenGL3_Init("#version 430 core");

	s_Instance = new App(width, height);
}

void App::clear() {
    delete s_Instance;
    s_Instance = nullptr;
}

uint32_t App::getWindowWidth() {
    if (s_Instance) {
        return s_Instance->m_ScrWidth;
    }
    else {
        return 0;
    }
}

uint32_t App::getWindowHeight() {
    if (s_Instance) {
        return s_Instance->m_ScrHeight;
    }
    else {
        return 0;
    }
}

graphics::Planet* App::addToScene(graphics::Planet& planet) {
    if (s_Instance) {
        graphics::Planet* newPlanet = new graphics::Planet(planet);
        s_Instance->scene.planets.push_back(newPlanet);
        s_Instance->physicsEngine.addBody(&(newPlanet->body));
        return newPlanet;
    }

    return &planet;
}
graphics::Star* App::addToScene(graphics::Star& star) {
    if (s_Instance) {
        graphics::Star* newStar = new graphics::Star(star);
        s_Instance->scene.stars.push_back(newStar);
        s_Instance->physicsEngine.addBody(&(newStar->body));
        return newStar;
    }

    return &star;
}

void App::setSkybox(const graphics::Skybox& skybox) {
    if (s_Instance) {
        s_Instance->scene.skybox = skybox;
    }
}

App::App(uint32_t windowWidth, uint32_t windowHeight)
    : renderer(), renderMode(graphics::RenderMode::FACES), physicsEngine(), scene(),
    mainCamera(glm::vec3(0.0f, 0.0f, 10.0f)), camTargets(),
    mousePos(0.0f, 0.0f)
{
    m_ScrWidth = windowWidth;
    m_ScrHeight = windowHeight;
    
    renderer.resize(m_ScrWidth, m_ScrHeight);
    renderer.bindScene(&this->scene);

    this->isCursorVisible = true;
    this->isFirstMouseMovement = true;
    
    this->mainCamera.projMatrix = glm::perspective(mainCamera.fov, (float)m_ScrWidth / (float)m_ScrHeight, 0.1f, 200.0f);
}

App::~App() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(s_Window);
    glfwTerminate();
}

void App::resize(uint32_t width, uint32_t height) {
    m_ScrWidth = width;
    m_ScrHeight = height;

    mainCamera.projMatrix = glm::perspective(mainCamera.fov, (float)App::getWindowWidth() / (float)App::getWindowHeight(), 0.1f, 200.0f);
}

namespace app {
    void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
        if (App::s_Instance) {
            App::s_Instance->resize(width, height);
            App::s_Instance->renderer.resize();
        }
    }
}