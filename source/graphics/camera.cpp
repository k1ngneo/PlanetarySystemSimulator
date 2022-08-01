#include "StarSystemSim/graphics/camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace graphics {

	Camera::Camera(const glm::vec3& pos) {
        this->pos = pos;
        this->front = glm::vec3(0.0f, 0.0f, -1.0f);
        this->up = glm::vec3(0.0f, 1.0f, 0.0f);

        this->dir = glm::vec3(0.0f, 0.0f, 0.0f);

        this->pitch = 0.0f;
        this->yaw = -90.0f;
        this->roll = 0.0f;
        this->fov = 45.0f;

        this->target = nullptr;

        this->viewMatrix = glm::mat4x4(1.0f);
        this->projMatrix = glm::mat4x4(1.0f);

        m_PosRelTarget = { 0.0f, 0.0f, 0.0f };
	}

    glm::mat4& Camera::lookAround() {
        this->dir.x = cos(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
        this->dir.y = sin(glm::radians(this->pitch));
        this->dir.z = sin(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
        this->front = glm::normalize(this->dir);

        this->viewMatrix = glm::lookAt(this->pos, this->front, this->up);
        return this->viewMatrix;
    }

    glm::mat4& Camera::lookAt(const glm::vec3& target, bool justFollow) {
        if (!justFollow) {
            float radPitch = glm::radians(this->pitch);
            float radYaw = glm::radians(this->yaw);

            m_PosRelTarget.x = glm::cos(-radYaw) * glm::cos(radPitch);
            m_PosRelTarget.y = glm::sin(radPitch);        
            m_PosRelTarget.z = glm::sin(-radYaw) * glm::cos(radPitch);

            this->dir = glm::normalize(-m_PosRelTarget);
        }

        this->pos = target + m_PosRelTarget;

        this->viewMatrix = glm::lookAt(this->pos, target, this->up);
        return this->viewMatrix;
    }

}