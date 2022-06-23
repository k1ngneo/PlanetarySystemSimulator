#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace graphics {

    class Object;

    class Camera {
    public:
        glm::mat4 lookAround();
        glm::mat4 lookAt(const glm::vec3& target, bool justFollow = false);

        void update();


        glm::vec3 pos;
        glm::vec3 front;
        glm::vec3 up;

        glm::vec3 dir;

        float pitch, yaw, roll;
        float fov;

        Object* target;

        Camera(const glm::vec3& pos);

    private:
        glm::vec3 m_PosRelTarget;
    };

}