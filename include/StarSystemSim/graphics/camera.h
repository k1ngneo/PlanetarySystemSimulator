#pragma once

#include <glm/vec3.hpp>

namespace graphics {

    class Camera {
    public:
        glm::vec3 pos;
        glm::vec3 front;
        glm::vec3 up;

        glm::vec3 dir;

        float pitch, yaw, roll;
        float fov;

        Camera(const glm::vec3& pos);
    };

}