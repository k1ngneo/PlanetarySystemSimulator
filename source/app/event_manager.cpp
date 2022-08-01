#include "StarSystemSim/app/event_manager.h"

#include "StarSystemSim/graphics/camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace app {
	bool EventManager::x_pressed = false;
	uint32_t EventManager::render_mode = 0;

	void EventManager::processInput(GLFWwindow* window) {
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

                switch ((RenderMode)(render_mode & 0x000000FF)) {
                case RenderMode::FACES:
                    render_mode = (0xFFFFFF00 & render_mode) | (uint32_t)RenderMode::EDGES;
                    break;
                case RenderMode::EDGES:
                    render_mode = (0xFFFFFF00 & render_mode) | (uint32_t)RenderMode::VERTICES;
                    break;
                case RenderMode::VERTICES:
                    render_mode = (0xFFFFFF00 & render_mode) | (uint32_t)RenderMode::FACES;
                    break;
                }
            }
        }
        else {
            x_pressed = false;
        }

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            glm::vec3 camRight = glm::normalize(glm::cross(camera.front, camera.up));
            camera.pos += glm::normalize(glm::cross(camera.up, camRight)) * camSpeed * App::mainTimer.deltaTime;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            glm::vec3 camRight = glm::normalize(glm::cross(camera.front, camera.up));
            camera.pos -= glm::normalize(glm::cross(camera.up, camRight)) * camSpeed * App::mainTimer.deltaTime;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            camera.pos -= glm::normalize(glm::cross(camera.front, camera.up)) * camSpeed * App::mainTimer.deltaTime;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            camera.pos += glm::normalize(glm::cross(camera.front, camera.up)) * camSpeed * App::mainTimer.deltaTime;
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            camera.pos += camSpeed * camera.up * App::mainTimer.deltaTime;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
            camera.pos -= camSpeed * camera.up * App::mainTimer.deltaTime;
        }
	}
}