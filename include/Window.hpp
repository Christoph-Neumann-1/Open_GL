#pragma once
#include <atomic>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <Callback.hpp>

namespace GL
{
    ///@brief Simple wrapper. Manages window size.
    ///
    ///@attention Not thread safe. Should only be used in render thread anyways.
    class Window
    {
        int size_x;
        int size_y; 
        GLFWwindow *window;
        CallbackList &ResizeCallback;

        static void Resize(GLFWwindow *window, int x, int y)
        {
            auto _this = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
            _this->size_x = x;
            _this->size_y = y;
            glViewport(0,0,x,y);
            _this->ResizeCallback();
        }

    public:
        Window(GLFWwindow *window, CallbackList &callback);

        ~Window();

        uint GetWidth() const { return size_x; }
        uint GetHeigth() const { return size_y; }

        operator GLFWwindow *() const { return window; }
    };
}