/**
 * @file Flycam.hpp
 * @author Christoph Neumann
 * @copyright Copyright © 2021 Christoph Neumann - MIT License
 */
#pragma once

#include <Camera/CameraControler.hpp>
namespace GL
{
    /**
     * @brief Can rotate in all directions freely.
     * 
     */
    class Flycam : public CameraControler
    {

        double movement;
        double rotation;
        double mouseSpeed;

        double m_x;
        double m_y;

    public:
        Flycam(Camera3D &_cam, GLFWwindow *_window, double move = 4, double rot = 80, double mouse = 0.11) : CameraControler(_cam, _window), movement(move), rotation(rot), mouseSpeed(mouse)
        {
            Camera3D::LockMouse(window);
            glfwGetCursorPos(window, &m_x, &m_y);
        }
        ~Flycam()
        {
            Camera3D::UnlockMouse(window);
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

            glm::dvec3 forward = cam.Forward();
            glm::dvec3 up_cam = cam.Up();
            glm::dvec3 right = cam.Right();

            cam.position += m_dt * forward * (double)glfwGetKey(window, GLFW_KEY_W) - m_dt * forward * (double)glfwGetKey(window, GLFW_KEY_S);
            cam.position += m_dt * right * (double)glfwGetKey(window, GLFW_KEY_D) - m_dt * right * (double)glfwGetKey(window, GLFW_KEY_A);
            cam.position += m_dt * up_cam * (double)glfwGetKey(window, GLFW_KEY_R) - m_dt * up_cam * (double)glfwGetKey(window, GLFW_KEY_F);

            cam.rotation = glm::rotate(cam.rotation, glm::radians(mouseSpeed * mousedeltay), right);
            cam.rotation = glm::rotate(cam.rotation, glm::radians(mouseSpeed * mousedeltax), up_cam);
            cam.rotation = glm::rotate(cam.rotation, glm::radians(rotation * deltatime * (double)(glfwGetKey(window, GLFW_KEY_Q) - glfwGetKey(window, GLFW_KEY_E))), forward);
        }
    };
}