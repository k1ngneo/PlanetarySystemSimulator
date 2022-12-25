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
        void changeTarget(Object* newTarget);
        void setTarget(Object* newTarget);

        glm::vec3 pos;
        glm::vec3 front;
        glm::vec3 up;

        glm::vec3 dir;

        // current pitch, yaw, roll in degrees
        float pitch, yaw, roll;
        // goal pitch, yaw, roll for linear interpolation
        float gPitch, gYaw, gRoll;

        float fov;
        float radius;


        glm::mat4x4 viewMatrix;
        glm::mat4x4 projMatrix;

        Camera(const glm::vec3& pos);

        inline const Object* getTarget() { return m_Target; }

    private:
        glm::mat4& lookAround();
        glm::mat4& lookAt(const glm::vec3& target);
        Object* m_Target;
        glm::vec3 m_PosRelTarget;
        bool m_CameraLockedOnTarget;
    };

}