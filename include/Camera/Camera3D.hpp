/**
 * @file Camera3D.hpp
 * @author Christoph Neumann
 * @copyright Copyright © 2021 Christoph Neumann - MIT License
 */
#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <GLFW/glfw3.h>

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
        static constexpr bool allow_mouse_lock=true;

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
        static void LockMouse(GLFWwindow *window)
        {
            if (!allow_mouse_lock)
                return;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        }
        static void UnlockMouse(GLFWwindow *window)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    };
}