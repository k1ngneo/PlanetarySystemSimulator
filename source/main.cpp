#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "StarSystemSim/app/event_manager.h"

#include "StarSystemSim/graphics/shader.h"
#include "StarSystemSim/graphics/camera.h"
#include "StarSystemSim/graphics/primitives/plane.h"
#include "StarSystemSim/graphics/primitives/cube.h"
#include "StarSystemSim/graphics/primitives/point_light.h"
#include "StarSystemSim/graphics/model.h"
#include "StarSystemSim/graphics/planet.h"
#include "StarSystemSim/graphics/star.h"
#include "StarSystemSim/graphics/skybox.h"

#include "StarSystemSim/physics/engine.h"

#include "StarSystemSim/utilities/timer.h"
#include "StarSystemSim/utilities/load_text_file.h"


#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_press_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

void lookAroundCam(glm::mat4& viewMat);
void lookAtCam(glm::mat4& viewMat, const glm::vec3& target);

unsigned int SCR_WIDTH = 1500;
unsigned int SCR_HEIGHT = 900;

int render_mode = 0;
bool x_pressed = false;

physics::Engine physicsEngine;

utils::Timer timer;
graphics::Camera mainCamera(glm::vec3(0.0f, 0.5f, 3.0f));

glm::vec2 mousePos;
bool isFirstMouseMovement = true;

bool isCursorVisible = false;

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 2);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "StarSystemSim", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_press_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetWindowPos(window, 20, 60);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    Shader lightingShader;
    lightingShader.compileShaders("shaders/lighting.shader", true);
    lightingShader.linkShaders();

    Shader lightSourceShader;
    lightSourceShader.compileShaders(utils::loadTextFile("shaders/light_source.shader").c_str());
    lightSourceShader.linkShaders();

    Shader starShader;
    starShader.compileShaders("shaders/star.shader", true);
    starShader.linkShaders();

    Shader skyboxShader;
    skyboxShader.compileShaders("shaders/skybox.shader", true);
    skyboxShader.linkShaders();

    Shader TBNShader;
    TBNShader.compileShaders("shaders/TBN.shader", true);
    TBNShader.linkShaders();

    //graphics::primitives::Plane floor;
    //floor.scale(glm::vec3(3.0f));

    graphics::Skybox skybox;

    glm::mat4 projMat4(1.0f), viewMat4(1.0f);

    graphics::Planet earth("earth");
    earth.translate(glm::vec3(-5.0f, 0.0f, 0.0f));
    earth.scale(glm::vec3(0.2f));
    earth.body.mass = 1.0f;
    earth.body.vel = { 0.0f, 0.0f, -10.0f };
    earth.activateBody(physicsEngine);

    graphics::Star sun("sun");
    sun.translate(glm::vec3(0.0f, 0.0f, 0.0f));
    sun.body.mass = 1000.0f;
    sun.body.vel = { 0.0f, 0.0f, 0.063245f };
    sun.activateBody(physicsEngine);

    physicsEngine.paused = true;

    mainCamera.target = &earth;

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


    bool show_demo_window = false;
    bool show_another_window = false;
    float clear_color[3] = { 0.5f, 0.5f, 0.5f };


    while (!glfwWindowShouldClose(window)) {
        timer.measureTime();
        physicsEngine.update();
        app::processInput(window);
        mainCamera.dir = earth.getPos() - mainCamera.pos;

        projMat4 = glm::perspective(glm::radians(mainCamera.fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.01f, 200.0f);

        if (!isCursorVisible) {
            //lookAroundCam(viewMat);
            lookAtCam(viewMat4, ((graphics::Planet*)(mainCamera.target))->getPos());
        }

        // ImGui preparing for a new frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();


        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        skyboxShader.use();
        skyboxShader.setUniformMat4("_viewMat", glm::mat4(glm::mat3(viewMat4)));
        skyboxShader.setUniformMat4("_projMat", projMat4);
        skyboxShader.unuse();

        starShader.use();
        starShader.setUniformMat4("_viewMat", viewMat4);
        starShader.setUniformMat4("_projMat", projMat4);
        starShader.unuse();

        skybox.draw(skyboxShader);



        lightingShader.use();
        
        lightingShader.setUniform3f("_viewPos", mainCamera.pos);

        lightingShader.setUniform3f("_light1.pos", sun.body.pos);
        lightingShader.setUniform3f("_light1.amb", sun.ambientColor);
        lightingShader.setUniform3f("_light1.diff", sun.diffuseColor);
        lightingShader.setUniform3f("_light1.spec", sun.specularColor);
        lightingShader.setUniform3f("_light1.attenuation", sun.attenuation);
        
        lightingShader.setUniformMat4("_projMat", projMat4);
        lightingShader.setUniformMat4("_viewMat", viewMat4);

        lightingShader.setUniform3f("_surfM.amb", glm::vec3(0.1f));
        lightingShader.setUniform3f("_surfM.diff", glm::vec3(1.0f));
        lightingShader.setUniform3f("_surfM.spec", glm::vec3(0.8f));
        lightingShader.setUniform1f("_surfM.shine", 32.0f);

        lightingShader.setUniform1f("_time", glfwGetTime());

        lightingShader.unuse();

        TBNShader.use();
        TBNShader.setUniformMat4("_projMat", projMat4);
        TBNShader.setUniformMat4("_viewMat", viewMat4);
        TBNShader.setUniform3f("_viewPos", mainCamera.pos);
        TBNShader.unuse();

        earth.rotate(0.02f, glm::vec3(0.0f, 1.0f, 0.0f));
        earth.draw(lightingShader);
        //earth.draw(TBNShader, GL_POINTS);
        sun.draw(starShader);

        //////////////////
        // ImGui rendering
        //////////////////

        // Control Panel Window
        {
            ImGui::Begin("Control Panel");

            ImGui::Checkbox("Pause Simulation", &physicsEngine.paused);
            ImGui::SliderFloat("Time Speed", &physicsEngine.timeMultiplier, 0.0f, 10.0f);

            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

            ImGui::Text("Camera\n");
            ImGui::Text("Yaw: %.1f\nPitch: %.1f", mainCamera.yaw, mainCamera.pitch);

            ImGui::End();
        }

        ImGui::Render();
        ImDrawData* drawData = ImGui::GetDrawData();
        if (drawData) {
            ImGui_ImplOpenGL3_RenderDrawData(drawData);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();

        lightingShader.reload();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void key_press_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    //if (glfwGetKey(window, GLFW_KEY_GRAVE_ACCENT) == GLFW_PRESS) {
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        if (isCursorVisible) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            isFirstMouseMovement = true;
            isCursorVisible = false;
        }
        else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            isCursorVisible = true;
        }
    }
}

void lookAroundCam(glm::mat4& viewMat) {
    mainCamera.dir.x = cos(glm::radians(mainCamera.yaw)) * cos(glm::radians(mainCamera.pitch));
    mainCamera.dir.y = sin(glm::radians(mainCamera.pitch));
    mainCamera.dir.z = sin(glm::radians(mainCamera.yaw)) * cos(glm::radians(mainCamera.pitch));
    mainCamera.front = glm::normalize(mainCamera.dir);

    viewMat = glm::lookAt(mainCamera.pos, mainCamera.front, mainCamera.up);
}

void lookAtCam(glm::mat4& viewMat, const glm::vec3& target) {
    mainCamera.pos.x = glm::cos(-mainCamera.yaw * 0.022222f);
    mainCamera.pos.y = glm::sin(mainCamera.pitch * 0.011111f);
    mainCamera.pos.z = glm::sin(-mainCamera.yaw * 0.022222f);
    
    mainCamera.dir = glm::normalize(-mainCamera.pos);
    
    mainCamera.pos = target + mainCamera.pos;

    viewMat = glm::lookAt(mainCamera.pos, target, mainCamera.up);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (isFirstMouseMovement) {
        mousePos = glm::vec2(xpos, ypos);
        isFirstMouseMovement = false;
    }

    glm::vec2 offset = glm::vec2(xpos - mousePos.x, mousePos.y - ypos);
    mousePos = glm::vec2(xpos, ypos);

    float sensitivity = 0.1f;
    offset *= sensitivity;

    mainCamera.yaw += offset.x;
    mainCamera.pitch += offset.y;

    if (mainCamera.pitch > 90.0f)
        mainCamera.pitch = 90.0f;
    if (mainCamera.pitch < -90.0f)
        mainCamera.pitch = -90.0f;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    mainCamera.fov -= (float)yoffset;
    if (mainCamera.fov < 1.0f)
        mainCamera.fov = 1.0f;
    if (mainCamera.fov > 45.0f)
        mainCamera.fov = 45.0f;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
}