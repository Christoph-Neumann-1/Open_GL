#pragma once

#include <GLFW/glfw3.h>
#include <Window.hpp>
#include <unordered_map>
#include <Window.hpp>

namespace GL
{
    ///@attention Only create one instance
    class InputHandler
    {
        struct KeyCallbacks
        {
            struct Element
            {
                std::function<void()> func;
                uint id;
                bool operator==(uint idd)
                {   
                    return idd==id;
                }

                Element(const std::function<void()> &f, uint idd) : func(f), id(idd) {}
                Element(std::function<void()> &&f, uint idd) : func(std::move(f)), id(idd) {}
            };
            std::vector<Element> funcs;
            void Call()
            {
                for (auto &f : funcs)
                    f.func();
            }
        };

        GL::Window &window;
        std::unordered_map<int, KeyCallbacks *> callbacks;
        std::mutex mutex;
        uint current_id = 1;

        void CallbackFunc(int code, int action)
        {
            if (callbacks.contains(code))
            {
            }
        }

    public:
        template <typename F, typename... Args>
        class KeyCallback
        {
            uint id;
            InputHandler &handle;
            int scancode;

            KeyCallback(InputHandler &handler, int code, F &&func, Args... args) : handle(handler), scancode(code)
            {
                id = handle.AddCallback(scancode, std::move(func), args...);
            }
            KeyCallback(InputHandler &handler, int code, const F &func, Args... args) : handle(handler), scancode(code)
            {
                id = handle.AddCallback(scancode, func, args...);
            }
            ~KeyCallback()
            {
                handle.RemoveCallback(scancode, id);
            }
        };

        template <typename F, typename... Args>
        uint AddCallback(int code, const F &func, Args... args)
        {
            std::scoped_lock mmutex(mutex);
            callbacks[code]->funcs.emplace_back(std::bind(func, args...), current_id);
            return current_id++;
        }

        template <typename F, typename... Args>
        uint AddCallback(int code, F &&func, Args... args)
        {
            std::scoped_lock mmutex(mutex);
            callbacks[code]->funcs.emplace_back(std::bind(std::move(func), args...), current_id);
            return current_id++;
        }

        void RemoveCallback(int code, uint id)
        {
            std::scoped_lock mmutex(mutex);
            auto &cbl = callbacks[code];
            cbl->funcs.erase(std::find(cbl->funcs.begin(), cbl->funcs.end(), id));
        }

        InputHandler(Window &w) : window(w)
        {

            glfwSetKeyCallback(w, [](GLFWwindow *ww, int, int code, int action, int)
                               { ((Window *)glfwGetWindowUserPointer(ww))->inputptr->CallbackFunc(code, action); });
            window.inputptr = this;
        }
        ~InputHandler()
        {
            glfwSetKeyCallback(window, nullptr);
        }
    };
}