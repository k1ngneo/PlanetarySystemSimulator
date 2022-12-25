#include "StarSystemSim/app/app.h"
#include "StarSystemSim/app/event_manager.h"

#include "StarSystemSim/graphics/shader.h"
#include "StarSystemSim/graphics/renderer.h"
#include "StarSystemSim/graphics/camera.h"
#include "StarSystemSim/graphics/primitives/plane.h"
#include "StarSystemSim/graphics/primitives/cube.h"
#include "StarSystemSim/graphics/primitives/point_light.h"
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
#include <algorithm>

int main() {
    App::start();
    graphics::Camera& camera = App::s_Instance->mainCamera;
    glm::mat4x4* viewMat4 = &camera.viewMatrix;
    
    graphics::Skybox skybox;
    skybox.loadTexture("models/skybox/");
    App::setSkybox(skybox);

    graphics::Object* camTarget = nullptr;
    
    graphics::Planet* earth = nullptr;

    {
        graphics::Planet e("earth");
        e.translate(glm::vec3(-5.0f, 0.0f, 0.0f));
        e.scale(glm::vec3(0.2f));
        e.body.mass = 1.0f;
        e.body.vel = { 0.0f, 0.0f, -2.445f };
        camTarget = App::addToScene(e);
        earth = (graphics::Planet*)camTarget;
        App::s_Instance->camTargets.push_back(camTarget);
    
        graphics::Star sun("sun");
        sun.translate(glm::vec3(0.0f, 0.0f, 0.0f));
        sun.body.mass = 1000.0f;
        sun.body.vel = { 0.0f, 0.0f, 0.063245f };
        camTarget = App::addToScene(sun);
        App::s_Instance->camTargets.push_back(camTarget);
    }

    camera.mode = graphics::Camera::Mode::LOOK_AT;
    camera.pos = glm::vec3(-5.0f, 0.0f, 0.0f);
    camera.radius = 3.14f;
    camera.setTarget(camTarget);


    bool show_demo_window = false;
    bool show_another_window = false;
    float clear_color[3] = { 0.5f, 0.5f, 0.5f };

    physics::Engine& physicsEngine = App::s_Instance->physicsEngine;
    graphics::Renderer& renderer = App::s_Instance->renderer;

    std::vector<glm::vec3> lines;
    renderer.lines = &lines;

    while (!glfwWindowShouldClose(App::s_Window)) {
        App::mainTimer.measureTime();
        physicsEngine.update();
        app::EventManager::processInput(App::s_Window);

        camera.update();

        // ImGui preparing for a new frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        physicsEngine.getPredictedPos(lines);

        renderer.drawFrame((uint32_t)App::s_Instance->renderMode);

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

            ImGui::End();
        }

        // Selected Celestial Body
        {
            //ImGui::Begin("Celestial Body", (bool*)0, ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus);
            //
            //ImGui::DragFloat3("Position", (float*)&((graphics::Planet*)(camera.getTarget()))->body.pos);
            //ImGui::DragFloat3("Velocity", (float*)&((graphics::Planet*)(camera.getTarget()))->body.vel);
            //ImGui::End();
        }

        ImGui::Render();
        ImDrawData* drawData = ImGui::GetDrawData();
        if (drawData) {
            ImGui_ImplOpenGL3_RenderDrawData(drawData);
        }

        glfwSwapBuffers(App::s_Window);
        glfwPollEvents();

        App::frameClock.measureTime();
        std::chrono::microseconds sleepTime;
        double maxTime = 1000000.0f / 144.0f;
        sleepTime = std::chrono::microseconds((uint64_t)std::max(0.0f, (float)std::min(maxTime, maxTime - (double)App::frameClock.deltaTime * 1000000.0f)));
        std::this_thread::sleep_for(sleepTime);
    }

    App::clear();

    return 0;
}