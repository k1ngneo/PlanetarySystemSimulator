#include "StarSystemSim/graphics/camera.h"

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
	}

}