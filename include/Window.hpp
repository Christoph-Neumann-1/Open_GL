#pragma once
#include <atomic>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <Callback.hpp>

namespace GL
{
    class InputHandler;

    ///@brief Simple wrapper. Manages window size.
    ///
    ///@attention Not thread safe. Should only be used in render thread anyways.
    ///@attention Only create one instance
    class Window
    {
        int size_x;
        int size_y;
        GLFWwindow *window;
        CallbackList &ResizeCallback;
        int refreshrate;

        static void Resize(GLFWwindow *window, int x, int y)
        {
            auto _this = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
            _this->size_x = x;
            _this->size_y = y;
            glViewport(0, 0, x, y);
            _this->ResizeCallback();
        }

    public:
        InputHandler *inputptr;
        Window(GLFWwindow *window, CallbackList &callback, int refresh = 0);

        ~Window();

        uint GetWidth() const { return size_x; }
        uint GetHeigth() const { return size_y; }
        int GetRefreshRate() const { return refreshrate; }

        operator GLFWwindow *() const { return window; }
    };
}