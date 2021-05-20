#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace GL
{
    /**
    * @brief Stores rotation and position of the camera and computes the projection matrix.
    * 
    */
    class Camera3D
    {
    public:
        glm::quat rotation;
        glm::vec3 position;

        Camera3D(glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f), glm::quat rot = glm::quat(1, 0, 0, 0)) : rotation(rot), position(pos) {}

        glm::vec3 Up() const
        {
            return glm::vec3(glm::vec4{0, 1, 0, 0} * glm::toMat4(rotation));
        }
        glm::vec3 Forward() const { return glm::vec3(glm::vec4{0, 0, -1, 0} * glm::toMat4(rotation)); }
        glm::vec3 Right() const { return glm::cross(Forward(), Up()); }

        glm::mat4 ComputeMatrix() const
        {
            return glm::lookAt(
                position,
                position + Forward(),
                Up());
        }
    };
}