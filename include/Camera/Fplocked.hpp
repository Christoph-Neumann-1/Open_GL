#pragma once

#include <Camera/CameraControler.hpp>
#include <algorithm>
#include <Logger.hpp>

namespace GL
{

    class Fplocked : public CameraControler
    {
        float movement;
        float rotation;
        float mouseSpeed;

        glm::vec3 up = {0, 1, 0};

        double m_x;
        double m_y;

        float max_up = 89;
        float pitch = 0, yaw = 0;

    public:
        Fplocked(Camera3D *_cam, GLFWwindow *_window, float move = 4, float rot = 120, float mouse = 0.11) : CameraControler(_cam, _window), movement(move), rotation(rot), mouseSpeed(mouse)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
            glfwGetCursorPos(window, &m_x, &m_y);
        }
        ~Fplocked()
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

        void Update(float deltatime) override
        {
            double new_x, new_y;

            glfwGetCursorPos(window, &new_x, &new_y);

            float mousedeltax = new_x - m_x;
            float mousedeltay = new_y - m_y;

            m_x = new_x;
            m_y = new_y;

            float m_dt = movement * deltatime;
            float r_dt = rotation * deltatime;

            glm::vec3 forward = cam->Forward();
            glm::vec3 right = cam->Right();

            cam->position += m_dt * forward * (float)glfwGetKey(window, GLFW_KEY_W) - m_dt * forward * (float)glfwGetKey(window, GLFW_KEY_S);
            cam->position += m_dt * right * (float)glfwGetKey(window, GLFW_KEY_D) - m_dt * right * (float)glfwGetKey(window, GLFW_KEY_A);
            cam->position += m_dt * up * (float)glfwGetKey(window, GLFW_KEY_E) - m_dt * up * (float)glfwGetKey(window, GLFW_KEY_Q);

            pitch = std::clamp(pitch + r_dt * mouseSpeed * mousedeltay, -max_up, max_up);
            yaw += r_dt * mouseSpeed * mousedeltax;

            cam->rotation = glm::rotate(glm::quat(1,0,0,0), glm::radians(pitch), {1,0,0});
            cam->rotation = glm::rotate(cam->rotation, glm::radians(yaw), up);
        }
    };
}