#include "StarSystemSim/graphics/shader.h"
#include "StarSystemSim/graphics/camera.h"
#include "StarSystemSim/utilities/timer.h"
#include "StarSystemSim/utilities/load_text_file.h"

#include "StarSystemSim/graphics/primitives/plane.h"
#include "StarSystemSim/graphics/primitives/cube.h"
#include "StarSystemSim/graphics/primitives/point_light.h"
#include "StarSystemSim/graphics/model.h"
#include "StarSystemSim/graphics/planet.h"
#include "StarSystemSim/graphics/star.h"
#include "StarSystemSim/graphics/skybox.h"

#include "StarSystemSim/physics/engine.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

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
void processInput(GLFWwindow* window);

const unsigned int SCR_WIDTH = 1500;
const unsigned int SCR_HEIGHT = 900;

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

    Shader basicShader;
    basicShader.compileShaders(utils::loadTextFile("shaders/basic.shader").c_str());
    basicShader.linkShaders();

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

    graphics::primitives::Cube::initVBO();

    //graphics::primitives::Plane floor;
    //floor.scale(glm::vec3(3.0f));

    graphics::Skybox skybox;

    glm::mat4 projMat4(1.0f), viewMat4(1.0f);

    graphics::Planet earth("earth");
    earth.translate(glm::vec3(-5.0f, 0.0f, 0.0f));
    earth.m_Body.mass = 1.0f;
    earth.m_Body.vel = { 0.0f, 0.0f, -1.0f };
    earth.activateBody(physicsEngine);

    graphics::Planet moon("earth");
    //moon.scale(glm::vec3(0.2f));
    moon.translate(glm::vec3(5.0f, 0.0f, 0.0f));
    moon.m_Body.vel = { 0.0f, 0.0f, 1.0f };
    moon.m_Body.mass = 1.0f;
    moon.activateBody(physicsEngine);

    graphics::Star sun("sun");
    sun.translate(glm::vec3(0.0f, 0.0f, 5.0f));
    sun.m_Body.mass = 1.0f;
    sun.m_Body.vel = { -1.0f, 0.0f, 0.0f };
    sun.activateBody(physicsEngine);

    graphics::Star sun2("sun");
    sun2.translate(glm::vec3(0.0f, 0.0f, -5.0f));
    sun2.m_Body.mass = 1.0f;
    sun2.m_Body.vel = { 1.0f, 0.0f, 0.0f };
    sun2.activateBody(physicsEngine);

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
        processInput(window);
        mainCamera.dir = earth.getPos() - mainCamera.pos;

        projMat4 = glm::perspective(glm::radians(mainCamera.fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.01f, 100.0f);

        if (!isCursorVisible) {
            viewMat4 = glm::lookAt(mainCamera.pos, mainCamera.pos + mainCamera.front, mainCamera.up);
            //viewMat4 = glm::lookAt(mainCamera.pos, mainCamera.target->getPos(), { 0.0f, 1.0f, 0.0f });
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

        basicShader.use();
        basicShader.setUniformMat4("_projMat", projMat4);
        basicShader.setUniformMat4("_viewMat", viewMat4);
        basicShader.unuse();



        lightingShader.use();
        
        lightingShader.setUniform3f("_viewPos", mainCamera.pos);

        lightingShader.setUniform3f("_light1.pos", sun.m_Body.pos);
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

        earth.rotate(0.02f, glm::vec3(0.0f, 1.0f, 0.0f));
        earth.draw(lightingShader);
        moon.draw(lightingShader);
        sun.draw(starShader);
        sun2.draw(starShader);

        //////////////////
        // ImGui rendering
        //////////////////

        // Control Panel Window
        {
            ImGui::Begin("Control Panel");

            ImGui::SliderFloat("float", &physicsEngine.timeMultiplier, 0.0f, 10.0f);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
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

    graphics::primitives::Cube::destroyVBO();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float camSpeed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camSpeed = 2.0f;
    else
        camSpeed = 1.0f;

    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
        if (!x_pressed) {
            x_pressed = true;
        
            switch (render_mode++) {
            case 0:
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                break;
            case 1:
                glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
                break;
            case 2:
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                break;
            }

            render_mode = render_mode % 3;
        }
    }
    else {
        x_pressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        glm::vec3 camRight = glm::normalize(glm::cross(mainCamera.front, mainCamera.up));
        mainCamera.pos += glm::normalize(glm::cross(mainCamera.up, camRight)) * camSpeed * timer.deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        glm::vec3 camRight = glm::normalize(glm::cross(mainCamera.front, mainCamera.up));
        mainCamera.pos -= glm::normalize(glm::cross(mainCamera.up, camRight)) * camSpeed * timer.deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        mainCamera.pos -= glm::normalize(glm::cross(mainCamera.front, mainCamera.up)) * camSpeed * timer.deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        mainCamera.pos += glm::normalize(glm::cross(mainCamera.front, mainCamera.up)) * camSpeed * timer.deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        mainCamera.pos += camSpeed * mainCamera.up * timer.deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        mainCamera.pos -= camSpeed * mainCamera.up * timer.deltaTime;
    }
}

void key_press_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (glfwGetKey(window, GLFW_KEY_GRAVE_ACCENT) == GLFW_PRESS) {
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

void lookAroundCam() {
    mainCamera.dir.x = cos(glm::radians(mainCamera.yaw)) * cos(glm::radians(mainCamera.pitch));
    mainCamera.dir.y = sin(glm::radians(mainCamera.pitch));
    mainCamera.dir.z = sin(glm::radians(mainCamera.yaw)) * cos(glm::radians(mainCamera.pitch));
    mainCamera.front = glm::normalize(mainCamera.dir);
}

void lookAtCam() {
    if (mainCamera.target) {
        mainCamera.pos.x = glm::cos(mainCamera.yaw + glm::pi<float>());
        mainCamera.pos.y = glm::sin(mainCamera.pitch);
        mainCamera.pos.z = glm::sin(mainCamera.yaw + glm::pi<float>());

        mainCamera.dir = -mainCamera.pos;

        mainCamera.pos = mainCamera.target->getPos() + mainCamera.pos;
    }
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

    if (mainCamera.pitch > 89.0f)
        mainCamera.pitch = 89.0f;
    if (mainCamera.pitch < -89.0f)
        mainCamera.pitch = -89.0f;

    lookAroundCam();
    //lookAtCam();
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
}