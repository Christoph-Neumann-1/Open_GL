#pragma once

#include <GLFW/glfw3.h>
#include <Window.hpp>
#include <unordered_map>
#include <Callback.hpp>
#include <Window.hpp>

namespace GL
{
    ///@attention Only create one instance
    class InputHandler
    {
        GL::Window &window;
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

            glfwSetKeyCallback(w, [](GLFWwindow * ww, int, int code, int action, int)
                               { ((Window*)glfwGetWindowUserPointer(ww))->inputptr->CallbackFunc(code, action); });
                               window.inputptr=this;
        }
        ~InputHandler()
        {
            glfwSetKeyCallback(window, nullptr);
        }
    };
}