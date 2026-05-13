#include "PlanetarySystemSim/graphics/camera.h"

#include "PlanetarySystemSim/app/app.h"
#include "PlanetarySystemSim/graphics/object.h"
#include "PlanetarySystemSim/utilities/error.h"
#include "PlanetarySystemSim/utilities/lerp.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace graphics {

	Camera::Camera(const glm::vec3& pos) {
        this->mode = Mode::LOOK_AROUND;

        this->pos = pos;
        this->front = glm::vec3(0.0f, 0.0f, -1.0f);
        this->up = glm::vec3(0.0f, 1.0f, 0.0f);

        this->pitch = 0.0f;
        this->yaw = -90.0f;
        this->roll = 0.0f;

        this->fov = 45.0f;
        this->radius = 1.0f;

        this->target = nullptr;

        this->viewMatrix = glm::mat4x4(1.0f);
        this->projMatrix = glm::mat4x4(1.0f);

        m_PosRelTarget = { 0.0f, 0.0f, 0.0f };
        m_TargetSwitchProgress = 1.0f;
	}

    void Camera::update(glm::mat4x4* viewMatrix) {
        switch (this->mode) {
            case Mode::LOOK_AROUND:
                if (viewMatrix)
                    *viewMatrix = this->lookAround();
                else
                    this->lookAround();
                break;
            case Mode::LOOK_AT:
                if (this->target == nullptr) {
                    utils::printError("Camera tried to look at a not set target");
                    break;
                }
                if (viewMatrix)
                    *viewMatrix = this->lookAt(this->target->getPos());
                else
                    this->lookAt(this->target->getPos());
                break;
        }
    }

    void Camera::setTarget(Object* newTarget) {
        this->target = newTarget;
        m_TargetSwitchProgress = 0.0f;
    }

    glm::mat4& Camera::lookAround() {
        glm::vec3 dir;

        dir.x = cos(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
        dir.y = sin(glm::radians(this->pitch));
        dir.z = sin(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
        this->front = glm::normalize(dir);

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

            m_PosRelTarget *= this->radius;
        }
        
        m_TargetSwitchProgress += App::s_Instance->mainTimer.deltaTime / m_TargetSwitchDuration;
        m_TargetSwitchProgress = std::min(m_TargetSwitchProgress, 1.0f);

        this->pos = utils::lerp(this->pos, target + m_PosRelTarget, m_TargetSwitchProgress);
        this->front = utils::lerp(this->front, target, m_TargetSwitchProgress);

        this->viewMatrix = glm::lookAt(this->pos, this->front, this->up);
        return this->viewMatrix;
    }

}