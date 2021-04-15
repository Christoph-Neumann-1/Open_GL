#pragma once

#include <Camera3D.hpp>
#include <GLFW/glfw3.h>

class Flycam2
{
    Camera3D *cam;
    GLFWwindow *window;

    float movement;
    float rotation;
    float mouseSpeed;

    glm::vec3 up={0,1,0};

    double m_x;
    double m_y;


public:
    Flycam2(Camera3D *_cam, GLFWwindow *window, float move = 6, float rot = 4, float mouse = 0.2) : cam(_cam), window(window), movement(move), rotation(rot), mouseSpeed(mouse)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        glfwGetCursorPos(window, &m_x, &m_y);
    }
    ~Flycam2()
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    void Update(float deltatime)
    {
        double new_x, new_y;

        glfwGetCursorPos(window, &new_x, &new_y);

        float mousedeltax = new_x-m_x;
        float mousedeltay = new_y-m_y;

        m_x = new_x;
        m_y = new_y;

        float m_dt=movement*deltatime;
        float r_dt=rotation*deltatime;

        glm::vec3 forward = cam->Forward();
        glm::vec3 up_cam = cam->Up();
        glm::vec3 right = cam->Right();

        cam->position += m_dt * forward * (float)glfwGetKey(window, GLFW_KEY_W) - m_dt * forward * (float)glfwGetKey(window, GLFW_KEY_S);
        cam->position += m_dt * right * (float)glfwGetKey(window, GLFW_KEY_D) - m_dt * right * (float)glfwGetKey(window, GLFW_KEY_A);
        cam->position += m_dt * up_cam * (float)glfwGetKey(window, GLFW_KEY_E) - m_dt * up_cam * (float)glfwGetKey(window, GLFW_KEY_Q);


        cam->rotation = glm::rotate(cam->rotation, glm::radians(r_dt * mouseSpeed * mousedeltay), right);
        cam->rotation = glm::rotate(cam->rotation, glm::radians(r_dt * mouseSpeed * mousedeltax), up);

    }
};