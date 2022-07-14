#pragma once

#include "StarSystemSim/graphics/camera.h"
#include "StarSystemSim/utilities/timer.h"

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

extern bool x_pressed;
extern int render_mode;

extern utils::Timer timer;

enum class RenderMode {
    FACES, EDGES, VERTICES
};

RenderMode renderMode;

namespace app {

	void processInput(GLFWwindow* window) {
        graphics::Camera& camera = App::s_Instance->mainCamera;
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
                    renderMode = RenderMode::FACES;
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                    break;
                case 1:
                    renderMode = RenderMode::EDGES;
                    glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
                    break;
                case 2:
                    renderMode = RenderMode::VERTICES;
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
            glm::vec3 camRight = glm::normalize(glm::cross(camera.front, camera.up));
            camera.pos += glm::normalize(glm::cross(camera.up, camRight)) * camSpeed * timer.deltaTime;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            glm::vec3 camRight = glm::normalize(glm::cross(camera.front, camera.up));
            camera.pos -= glm::normalize(glm::cross(camera.up, camRight)) * camSpeed * timer.deltaTime;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            camera.pos -= glm::normalize(glm::cross(camera.front, camera.up)) * camSpeed * timer.deltaTime;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            camera.pos += glm::normalize(glm::cross(camera.front, camera.up)) * camSpeed * timer.deltaTime;
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            camera.pos += camSpeed * camera.up * timer.deltaTime;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
            camera.pos -= camSpeed * camera.up * timer.deltaTime;
        }
	}

}