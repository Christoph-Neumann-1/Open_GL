#pragma once

#include <Camera3D.hpp>
#include <GLFW/glfw3.h>

class SpaceCam
{
    Camera3D *cam;
    GLFWwindow *window;

    float acceleration;
    float rotation;
    float mouseSpeed;

    glm::vec3 velocity = {0, 0, 0};
    double m_x;
    double m_y;

public:
    SpaceCam(Camera3D *_cam, GLFWwindow *window, float move = 6, float rot = 4, float mouse = 0.2) : cam(_cam), window(window), acceleration(move), rotation(rot), mouseSpeed(mouse)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        glfwGetCursorPos(window, &m_x, &m_y);
    }
    ~SpaceCam()
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    void Update(float deltatime)
    {
        double new_x, new_y;

        glfwGetCursorPos(window, &new_x, &new_y);

        float mousedeltax = new_x - m_x;
        float mousedeltay = new_y - m_y;

        m_x = new_x;
        m_y = new_y;

        float m_dt = acceleration * deltatime;
        float r_dt = rotation * deltatime;

        glm::vec3 forward = cam->Forward();
        glm::vec3 up_cam = cam->Up();
        glm::vec3 right = cam->Right();

        velocity += m_dt * forward * (float)glfwGetKey(window, GLFW_KEY_W) - m_dt * forward * (float)glfwGetKey(window, GLFW_KEY_S);
        velocity += m_dt * right * (float)glfwGetKey(window, GLFW_KEY_D) - m_dt * right * (float)glfwGetKey(window, GLFW_KEY_A);
        velocity += m_dt * up_cam * (float)glfwGetKey(window, GLFW_KEY_R) - m_dt * up_cam * (float)glfwGetKey(window, GLFW_KEY_F);

        cam->position += deltatime*velocity;

        cam->rotation = glm::rotate(cam->rotation, glm::radians(r_dt * mouseSpeed * mousedeltay), right);
        cam->rotation = glm::rotate(cam->rotation, glm::radians(r_dt * mouseSpeed * mousedeltax), up_cam);
        cam->rotation = glm::rotate(cam->rotation, glm::radians(r_dt * (float)(glfwGetKey(window, GLFW_KEY_Q) - glfwGetKey(window, GLFW_KEY_E))), forward);
    }
};