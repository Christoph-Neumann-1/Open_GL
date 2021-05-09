#pragma once
#include <GLFW/glfw3.h>
#include <Camera/Camera3D.hpp>

namespace GL
{
    class CameraControler
    {
    protected:
        Camera3D *cam;
        GLFWwindow *window;

    public:
        /**
         * @brief Get user input and update camera position
         * 
         * @param deltatime in seconds
         */
        virtual void Update(float deltatime)=0;

        CameraControler(Camera3D *_cam, GLFWwindow *_window = nullptr) : cam(_cam), window(_window) {}
        virtual ~CameraControler() {}
    };
}