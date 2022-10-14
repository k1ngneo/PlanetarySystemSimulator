#include "StarSystemSim/app/event_manager.h"

#include "StarSystemSim/graphics/camera.h"

#include "StarSystemSim/utilities/error.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace app {

    void EventManager::setCallbackFunctions() {
        glfwSetKeyCallback(App::s_Window, EventManager::key_press_callback);
        glfwSetMouseButtonCallback(App::s_Window, EventManager::mouse_button_callback);
        glfwSetCursorPosCallback(App::s_Window, EventManager::mouse_callback);
        glfwSetScrollCallback(App::s_Window, EventManager::scroll_callback);
    }

	void EventManager::processInput(GLFWwindow* window) {
        graphics::Camera& camera = App::s_Instance->mainCamera;
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        float camSpeed;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            camSpeed = 2.0f;
        else
            camSpeed = 1.0f;

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

    void EventManager::key_press_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
            for (size_t i = 0; i < App::s_Instance->camTargets.size(); ++i) {
                if (App::s_Instance->mainCamera.getTarget() == App::s_Instance->camTargets[i]) {
                    App::s_Instance->mainCamera.changeTarget(App::s_Instance->camTargets[(i + 1) % App::s_Instance->camTargets.size()]);
                    break;
                }
            }
        }

        graphics::RenderMode& renderMode = App::s_Instance->renderMode;
        if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
            switch (renderMode) {
            case graphics::RenderMode::FACES:
                renderMode = graphics::RenderMode::LINES;
                break;
            case graphics::RenderMode::LINES:
                renderMode = graphics::RenderMode::POINTS;
                break;
            case graphics::RenderMode::POINTS:
                renderMode = graphics::RenderMode::FACES;
                break;
            }
        }
    }

    void EventManager::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            App::s_Instance->isFirstMouseMovement = true;
            App::s_Instance->isCursorVisible = false;
        }
        else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_RELEASE) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            App::s_Instance->isCursorVisible = true;
        }

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
            // selecting celestial bodies
        }
    }

    void EventManager::mouse_callback(GLFWwindow* window, double xpos, double ypos) {
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
            camera.gYaw += offset.x;

            int yawQuotient = 360 * (int32_t)(camera.gYaw / 360.0f);

            if (camera.gYaw < 0.0f) {
                camera.gYaw = (yawQuotient + 1) * 360.0f + camera.gYaw;
            }
            else {
                camera.gYaw = camera.gYaw - (float)yawQuotient;
            }

            camera.gPitch += offset.y;

            if (camera.gPitch > 89.0f)
                camera.gPitch = 89.0f;
            if (camera.gPitch < -89.0f)
                camera.gPitch = -89.0f;
        }
    }

    void EventManager::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
        graphics::Camera& camera = App::s_Instance->mainCamera;

        if (camera.mode == graphics::Camera::Mode::LOOK_AROUND) {
            camera.fov -= 0.01f * (float)yoffset;
            if (camera.fov < 1.0f)
                camera.fov = 1.0f;
            if (camera.fov > 45.0f)
                camera.fov = 45.0f;
        }
        else if (camera.mode == graphics::Camera::Mode::LOOK_AT) {
            camera.radius -= 0.1f * (float)yoffset;
            if (camera.radius < 1.0f)
                camera.radius = 1.0f;
            if (camera.radius > 10.0f)
                camera.radius = 10.0f;
        }

        camera.projMatrix = glm::perspective(camera.fov, (float)App::getWindowWidth() / (float)App::getWindowHeight(), 0.1f, 200.0f);
    }

    void EventManager::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
        if (App::s_Instance) {
            App::s_Instance->resize(width, height);
            App::s_Instance->renderer.resize();
        }
    }
}