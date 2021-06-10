#pragma once

#include <GLFW/glfw3.h>
#include <Window.hpp>
#include <unordered_map>
#include <Callback.hpp>

namespace GL
{
    class InputHandler
    {
        Window &window;
        std::unordered_map<int, CallbackList *> callbacks;
        std::function<void(GLFWwindow *, int, int code, int action, int)> cbf;

        void CallbackFunc(int code, int action)
        {
            if (callbacks.contains(code))
            {
            }
        }

    public:
        InputHandler(Window &w) : window(w)
        {

            glfwSetKeyCallback(w, [](GLFWwindow * window, int, int code, int action, int)
                               { ((InputHandler*)glfwGetWindowUserPointer(window))->CallbackFunc(code, action); });
        }
        ~InputHandler()
        {
            glfwSetKeyCallback(window, nullptr);
        }
    };
}