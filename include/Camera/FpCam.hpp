#pragma once

#include <Camera/CameraControler.hpp>
namespace GL
{

    class FpCam : public CameraControler
    {
        double movement;
        double rotation;

        glm::dvec3 up = {0, 1, 0};

        double m_x;
        double m_y;

    public:
        FpCam(Camera3D *_cam, GLFWwindow *_window, double move = 4, double rot = 0.15) : CameraControler(_cam,_window), movement(move), rotation(rot)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
            glfwGetCursorPos(window, &m_x, &m_y);
        }
        ~FpCam()
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

            glm::dvec3 forward = cam->Forward();
            glm::dvec3 up_cam = cam->Up();
            glm::dvec3 right = cam->Right();

            cam->position += m_dt * forward * (double)glfwGetKey(window, GLFW_KEY_W) - m_dt * forward * (double)glfwGetKey(window, GLFW_KEY_S);
            cam->position += m_dt * right * (double)glfwGetKey(window, GLFW_KEY_D) - m_dt * right * (double)glfwGetKey(window, GLFW_KEY_A);
            cam->position += m_dt * up_cam * (double)glfwGetKey(window, GLFW_KEY_E) - m_dt * up_cam * (double)glfwGetKey(window, GLFW_KEY_Q);

            cam->rotation = glm::rotate(cam->rotation, glm::radians(rotation * mousedeltay), right);
            cam->rotation = glm::rotate(cam->rotation, glm::radians(rotation * mousedeltax), up);
        }
    };
}