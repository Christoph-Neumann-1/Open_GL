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
    public:
        enum class Action
        {
            Release,
            Press,
            Repeat,
            ReleasePress,
            PressRepeat,
            All
        };

    private:
        struct KeyCallbacks
        {
            struct Element
            {
                std::function<void(int)> func;
                uint id;
                Action type;
                bool operator==(uint idd)
                {
                    return idd == id;
                }

                Element(const std::function<void(int)> &f, uint idd, Action type_ = Action::Press) : func(f), id(idd), type(type_) {}
                Element(std::function<void(int)> &&f, uint idd, Action type_ = Action::Press) : func(std::move(f)), id(idd), type(type_) {}
            };
            std::vector<Element> funcs;
            void Call(int action)
            {
                for (auto &f : funcs)
                {
                    if (f.type != Action::All &&
                        !(action == GLFW_RELEASE && (f.type == Action::ReleasePress || f.type == Action::Release)) &&
                        !(action == GLFW_PRESS && (f.type == Action::ReleasePress || f.type == Action::Press || f.type == Action::PressRepeat)) &&
                        !(action == GLFW_REPEAT && (f.type == Action::PressRepeat || f.type == Action::Repeat)))
                    {
                        continue;
                    }
                    f.func(action);
                }
            }
        };

        GL::Window &window;
        std::unordered_map<int, KeyCallbacks> callbacks;
        std::mutex mutex;
        uint current_id = 1;

        void KeyCallbackFunc(int code, int action)
        {
            //TODO  allow removing while called
            std::scoped_lock lk(mutex);
            if (callbacks.contains(code))
            {
                callbacks[code].Call(action);
            }
        }

    public:
        class KeyCallback
        {
            uint id = 0;
            InputHandler &handle;
            int scancode;

        public:
            template <typename F, typename... Args>
            void Bind(int code, Action type, F &&func, Args... args)
            {
                UnBind();
                scancode = code;
                id = handle.AddKeyCallback(scancode, type, std::move(func), args...);
            }

            template <typename F, typename... Args>
            void Bind(int code, Action type, const F &func, Args... args)
            {
                UnBind();
                scancode = code;
                id = handle.AddKeyCallback(scancode, type, func, args...);
            }

            void UnBind()
            {
                if (id == 0)
                    return;
                handle.RemoveKeyCallback(scancode, id);
            }
            KeyCallback(InputHandler &handler) : handle(handler) {}

            template <typename F, typename... Args>
            KeyCallback(InputHandler &handler, int code, Action type, F &&func, Args... args) : handle(handler)
            {
                Bind(code, type, std::move(func), args...);
            }

            template <typename F, typename... Args>
            KeyCallback(InputHandler &handler, int code, Action type, const F &func, Args... args) : handle(handler)
            {
                Bind(code, type, func, args...);
            }
            ~KeyCallback()
            {
                UnBind();
            }
        };

        class KeyState
        {
            uint id;
            InputHandler &handle;
            int scancode;

            int value = 0;

        public:
            KeyState(InputHandler &handler, int code) : handle(handler), scancode(code)
            {
                id = handle.AddKeyCallback(scancode, Action::ReleasePress, [&](int action)
                                           { value = action; });
            }
            ~KeyState()
            {
                handle.RemoveKeyCallback(scancode, id);
            }
            const int &GetValue() { return value; }
        };

        template <typename F, typename... Args>
        uint AddKeyCallback(int code, Action type, const F &func, Args... args)
        {
            std::scoped_lock mmutex(mutex);
            callbacks[code].funcs.emplace_back(std::bind(func, std::placeholders::_1, args...), current_id, type);
            return current_id++;
        }

        template <typename F, typename... Args>
        uint AddKeyCallback(int code, Action type, F &&func, Args... args)
        {
            std::scoped_lock mmutex(mutex);
            callbacks[code].funcs.emplace_back(std::bind(std::move(func), std::placeholders::_1, args...), current_id, type);
            return current_id++;
        }

        void RemoveKeyCallback(int code, uint id)
        {
            if (id == 0)
                return;
            std::scoped_lock mmutex(mutex);
            auto &cbl = callbacks[code];
            cbl.funcs.erase(std::find(cbl.funcs.begin(), cbl.funcs.end(), id));
        }

        InputHandler(Window &w) : window(w)
        {

            glfwSetKeyCallback(w, [](GLFWwindow *ww, int, int code, int action, int)
                               { ((Window *)glfwGetWindowUserPointer(ww))
                                     ->inputptr->KeyCallbackFunc(code, action); });
            // TODO add mouse buttons
            window.inputptr = this;
        }
        ~InputHandler()
        {
            glfwSetKeyCallback(window, nullptr);
        }
    };
}