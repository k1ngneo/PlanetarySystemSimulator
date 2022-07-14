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

int render_mode = 0;
bool x_pressed = false;

physics::Engine physicsEngine;
utils::Timer timer;
graphics::Renderer* renderer;

int main() {
    App::start();
    graphics::Camera& camera = App::s_Instance->mainCamera;

    renderer = new graphics::Renderer;

    Shader lightingShader;
    lightingShader.compileShaders("shaders/celestial.shader", true);
    lightingShader.linkShaders();

    Shader atmosphereShader;
    atmosphereShader.compileShaders("shaders/atmosphere.shader", true);
    atmosphereShader.linkShaders();

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

    camera.target = &earth;
    //camera.target = &sun;

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glfwSetInputMode(App::s_Instance->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    bool show_demo_window = false;
    bool show_another_window = false;
    float clear_color[3] = { 0.5f, 0.5f, 0.5f };

    utils::Timer frameClock;

    while (!glfwWindowShouldClose(App::s_Instance->window)) {
        timer.measureTime();
        physicsEngine.update();
        app::processInput(App::s_Instance->window);
        camera.dir = camera.target->getPos() - camera.pos;

        projMat4 = glm::perspective(glm::radians(camera.fov), (float)App::s_Instance->getWindowWidth() / (float)App::s_Instance->getWindowHeight(), 0.01f, 200.0f);

        if (!App::s_Instance->isCursorVisible) {
            //lookAroundCam(viewMat);
            viewMat4 = camera.lookAt(((graphics::Planet*)(camera.target))->getPos());
        }
        else {
            viewMat4 = camera.lookAt(((graphics::Planet*)(camera.target))->getPos(), true);
        }

        // ImGui preparing for a new frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        renderer->bindFramebuffer();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

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
        
        lightingShader.setUniform3f("_viewPos", camera.pos);

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

        atmosphereShader.use();

        atmosphereShader.setUniform3f("_light1.pos", sun.body.pos);
        atmosphereShader.setUniform3f("_light1.amb", sun.ambientColor);
        atmosphereShader.setUniform3f("_light1.diff", sun.diffuseColor);
        atmosphereShader.setUniform3f("_light1.spec", sun.specularColor);
        atmosphereShader.setUniform3f("_light1.attenuation", sun.attenuation);

        atmosphereShader.setUniformMat4("_projMat", projMat4);
        atmosphereShader.setUniformMat4("_viewMat", viewMat4);

        atmosphereShader.setUniform3f("_viewPos", camera.pos);

        atmosphereShader.unuse();

        TBNShader.use();
        TBNShader.setUniformMat4("_projMat", projMat4);
        TBNShader.setUniformMat4("_viewMat", viewMat4);
        TBNShader.setUniform3f("_viewPos", camera.pos);
        TBNShader.unuse();

        switch (renderMode) {
        case RenderMode::FACES:
            glPolygonMode(GL_FRONT, GL_FILL);
            break;
        case RenderMode::EDGES:
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            break;
        case RenderMode::VERTICES:
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            break;
        }

        earth.rotate(0.02f * timer.deltaTime, glm::vec3(0.0f, 1.0f, 0.0f));
        earth.draw(lightingShader);
        sun.draw(starShader);

        glCullFace(GL_BACK);
        earth.draw(atmosphereShader);
        glCullFace(GL_FRONT);

        //earth.draw(TBNShader, GL_POINTS);
        renderer->unbindFramebuffer();
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

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

        lightingShader.reload();
        atmosphereShader.reload();

        frameClock.measureTime();
        std::chrono::microseconds sleepTime;
        sleepTime = std::chrono::microseconds((uint64_t)std::min(1000000.0f / 144.0f, frameClock.deltaTime * 1000000.0f));
        std::this_thread::sleep_for(sleepTime);
    }

    delete renderer;

    App::clear();

    return 0;
}