#pragma once

#include <Camera/CameraControler.hpp>
#include <algorithm>
#include <Logger.hpp>

namespace GL
{

    class Fplocked : public CameraControler
    {
        double movement;
        double rotation;

        glm::dvec3 up = {0, 1, 0};

        double m_x;
        double m_y;

        double max_up = 89;
        double pitch = 0, yaw = 0;

    public:
        Fplocked(Camera3D *_cam, GLFWwindow *_window, double move = 4, double rot = 0.12) : CameraControler(_cam, _window), movement(move), rotation(rot)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
            glfwGetCursorPos(window, &m_x, &m_y);
        }
        ~Fplocked()
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

        void Update(double deltatime) override
        {
            double new_x, new_y;

            glfwGetCursorPos(window, &new_x, &new_y);

            double mousedeltax = new_x - m_x;
            double mousedeltay = new_y - m_y;

            m_x = new_x;
            m_y = new_y;

            double m_dt = movement * deltatime;

            glm::dvec3 forward = glm::normalize(glm::vec3{cam->Forward().x, 0, cam->Forward().z});
            glm::dvec3 right = cam->Right();

            cam->position += m_dt * forward * (double)glfwGetKey(window, GLFW_KEY_W) - m_dt * forward * (double)glfwGetKey(window, GLFW_KEY_S);
            cam->position += m_dt * right * (double)glfwGetKey(window, GLFW_KEY_D) - m_dt * right * (double)glfwGetKey(window, GLFW_KEY_A);
            cam->position += m_dt * up * (double)glfwGetKey(window, GLFW_KEY_E) - m_dt * up * (double)glfwGetKey(window, GLFW_KEY_Q);
            pitch = std::clamp(pitch + rotation * mousedeltay, -max_up, max_up);
            yaw += rotation * mousedeltax;

            cam->rotation = glm::rotate(glm::dquat(1, 0, 0, 0), glm::radians(pitch), {1, 0, 0});
            cam->rotation = glm::rotate(cam->rotation, glm::radians(yaw), up);
        }
    };
}