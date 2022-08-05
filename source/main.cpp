#include "StarSystemSim/app/app.h"
#include "StarSystemSim/app/event_manager.h"

#include "StarSystemSim/graphics/shader.h"
#include "StarSystemSim/graphics/renderer.h"
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
#include "StarSystemSim/utilities/error.h"


#include <GLFW/glfw3.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>

#include <iostream>
#include <chrono>
#include <thread>

physics::Engine physicsEngine;
graphics::Renderer* renderer;

int main() {
    App::start();
    graphics::Camera& camera = App::s_Instance->mainCamera;
    glm::mat4x4* viewMat4 = &camera.viewMatrix;

    renderer = new graphics::Renderer;
    
    graphics::Planet earth("earth");
    renderer->addObject(&earth);
    earth.translate(glm::vec3(-5.0f, 0.0f, 0.0f));
    earth.scale(glm::vec3(0.2f));
    earth.body.mass = 1.0f;
    earth.body.vel = { 0.0f, 0.0f, -10.0f };
    earth.activateBody(physicsEngine);

    graphics::Star sun("sun");
    renderer->addStar(&sun);
    sun.translate(glm::vec3(0.0f, 0.0f, 0.0f));
    sun.body.mass = 1000.0f;
    sun.body.vel = { 0.0f, 0.0f, 0.063245f };
    sun.activateBody(physicsEngine);

    physicsEngine.paused = true;

    camera.mode = graphics::Camera::Mode::LOOK_AT;
    camera.radius = 3.14f;
    camera.target = &earth;
    //camera.target = &sun;

    glfwSetInputMode(App::s_Instance->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    bool show_demo_window = false;
    bool show_another_window = false;
    float clear_color[3] = { 0.5f, 0.5f, 0.5f };

    camera.yaw = 190.0f;

    while (!glfwWindowShouldClose(App::s_Instance->window)) {
        App::mainTimer.measureTime();
        physicsEngine.update();
        app::EventManager::processInput(App::s_Instance->window);
        camera.dir = camera.target->getPos() - camera.pos;

        if (!App::s_Instance->isCursorVisible) {
            camera.update(viewMat4);
        }

        // ImGui preparing for a new frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        renderer->drawFrame();

        /////////////////////
        // ImGui rendering //
        /////////////////////

        // Control Panel Window
        {
            ImGui::Begin("Control Panel");

            ImGui::Checkbox("Pause Simulation", &physicsEngine.paused);
            ImGui::SliderFloat("Time Speed", &physicsEngine.timeMultiplier, 0.0f, 10.0f);

            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

            ImGui::Text("Camera\n");
            ImGui::Text("Yaw: %.1f\nPitch: %.1f", camera.yaw, camera.pitch);

            ImGui::SliderInt("Blur Strength", &renderer->blurStr, 0, 20);

            ImGui::End();
        }

        ImGui::Render();
        ImDrawData* drawData = ImGui::GetDrawData();
        if (drawData) {
            ImGui_ImplOpenGL3_RenderDrawData(drawData);
        }

        glfwSwapBuffers(App::s_Instance->window);
        glfwPollEvents();

        App::frameClock.measureTime();
        std::chrono::microseconds sleepTime;
        sleepTime = std::chrono::microseconds((uint64_t)std::min(1000000.0f / 144.0f, App::frameClock.deltaTime * 1000000.0f));
        std::this_thread::sleep_for(sleepTime);
    }

    delete renderer;

    App::clear();

    return 0;
}