#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace graphics {

    class Object;

    class Camera {
    public:
        enum class Mode {
            LOOK_AROUND, LOOK_AT
        } mode;

        void update(glm::mat4x4* viewMatrix = nullptr);

        glm::vec3 pos;
        glm::vec3 front;
        glm::vec3 up;

        glm::vec3 dir;

        float pitch, yaw, roll;

        float fov;
        float radius;

        Object* target;

        glm::mat4x4 viewMatrix;
        glm::mat4x4 projMatrix;

        Camera(const glm::vec3& pos);

    private:
        glm::mat4& lookAround();
        glm::mat4& lookAt(const glm::vec3& target, bool justFollow = false);
        glm::vec3 m_PosRelTarget;
    };

}