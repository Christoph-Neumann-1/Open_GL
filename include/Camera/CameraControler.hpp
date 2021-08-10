/**
 * @file CameraController.hpp
 * @author Christoph Neumann
 * @copyright Copyright © 2021 Christoph Neumann - MIT License
 */
#pragma once
#include <GLFW/glfw3.h>
#include <Camera/Camera3D.hpp>

namespace GL
{
    /**
     * @brief Base for cameras.
     * 
     * Camera3D stores the values. Derive from this class if you want to add logic.
     */
    class CameraControler
    {
    protected:
        Camera3D &cam;
        GLFWwindow *window;

    public:
        /**
         * @brief Get user input and update camera position
         * This should be called every frame.
         * 
         * @param deltatime in seconds
         */
        virtual void Update(double deltatime)=0;

        CameraControler(Camera3D &_cam, GLFWwindow *_window = nullptr) : cam(_cam), window(_window) {}
        virtual ~CameraControler() {}
    };
}