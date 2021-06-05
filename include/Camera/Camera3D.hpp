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
        glm::dquat rotation;
        glm::dvec3 position;

        Camera3D(glm::dvec3 pos = glm::dvec3(0.0f, 0.0f, 0.0f), glm::dquat rot = glm::dquat(1, 0, 0, 0)) : rotation(rot), position(pos) {}

        glm::dvec3 Up() const
        {
            return glm::dvec3(glm::vec4{0, 1, 0, 0} * glm::toMat4(rotation));
        }
        glm::dvec3 Forward() const { return glm::dvec3(glm::vec4{0, 0, -1, 0} * glm::toMat4(rotation)); }
        glm::dvec3 Right() const { return glm::cross(Forward(), Up()); }

        ///Generate the view matrix
        glm::mat4 ComputeMatrix() const
        {
            return glm::lookAt(
                position,
                position + Forward(),
                Up());
        }
    };
}