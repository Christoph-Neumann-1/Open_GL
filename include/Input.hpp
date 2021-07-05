#pragma once

#include <GLFW/glfw3.h>
#include <Window.hpp>
#include <unordered_map>
#include <Window.hpp>

namespace GL
{
    //TODO: allow for text input
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
        struct MouseCallbacks
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
                    if (
                        !(action == GLFW_RELEASE && (f.type == Action::ReleasePress || f.type == Action::Release)) &&
                        !(action == GLFW_PRESS && (f.type == Action::ReleasePress || f.type == Action::Press)))
                    {
                        continue;
                    }
                    f.func(action);
                }
            }
        };

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
        std::unordered_map<int, MouseCallbacks> mousecallbacks;
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

        void MouseCallbackFunc(int button, int action)
        {
            //TODO  allow removing while called
            std::scoped_lock lk(mutex);
            if (mousecallbacks.contains(button))
            {
                mousecallbacks[button].Call(action);
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

        class MouseCallback
        {
            uint id = 0;
            InputHandler &handle;
            int button;

        public:
            template <typename F, typename... Args>
            void Bind(int buttonn, Action type, F &&func, Args... args)
            {
                UnBind();
                button = buttonn;
                id = handle.AddMouseCallback(button, type, std::move(func), args...);
            }

            template <typename F, typename... Args>
            void Bind(int buttonn, Action type, const F &func, Args... args)
            {
                UnBind();
                button = buttonn;
                id = handle.AddMouseCallback(button, type, func, args...);
            }

            void UnBind()
            {
                if (id == 0)
                    return;
                handle.RemoveMouseCallback(button, id);
            }
            MouseCallback(InputHandler &handler) : handle(handler) {}

            template <typename F, typename... Args>
            MouseCallback(InputHandler &handler, int buttonn, Action type, F &&func, Args... args) : handle(handler)
            {
                Bind(buttonn, type, std::move(func), args...);
            }

            template <typename F, typename... Args>
            MouseCallback(InputHandler &handler, int buttonn, Action type, const F &func, Args... args) : handle(handler)
            {
                Bind(buttonn, type, func, args...);
            }
            ~MouseCallback()
            {
                UnBind();
            }
        };

        template <typename F, typename... Args>
        uint AddKeyCallback(int code, Action type, const F &func, Args... args)
        {
            std::scoped_lock mmutex(mutex);
            callbacks[code].funcs.emplace_back(std::bind(func, args..., std::placeholders::_1), current_id, type);
            return current_id++;
        }

        template <typename F, typename... Args>
        uint AddKeyCallback(int code, Action type, F &&func, Args... args)
        {
            std::scoped_lock mmutex(mutex);
            callbacks[code].funcs.emplace_back(std::bind(std::move(func), args..., std::placeholders::_1), current_id, type);
            return current_id++;
        }

        template <typename F, typename... Args>
        uint AddMouseCallback(int button, Action type, const F &func, Args... args)
        {
            std::scoped_lock mmutex(mutex);
            mousecallbacks[button].funcs.emplace_back(std::bind(func, args..., std::placeholders::_1), current_id, type);
            return current_id++;
        }

        template <typename F, typename... Args>
        uint AddMouseCallback(int button, Action type, F &&func, Args... args)
        {
            std::scoped_lock mmutex(mutex);
            mousecallbacks[button].funcs.emplace_back(std::bind(std::move(func), args..., std::placeholders::_1), current_id, type);
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

        void RemoveMouseCallback(int button, uint id)
        {
            if (id == 0)
                return;
            std::scoped_lock mmutex(mutex);
            auto &cbl = mousecallbacks[button];
            cbl.funcs.erase(std::find(cbl.funcs.begin(), cbl.funcs.end(), id));
        }

        InputHandler(Window &w) : window(w)
        {

            glfwSetKeyCallback(w, [](GLFWwindow *ww, int, int code, int action, int)
                               { ((Window *)glfwGetWindowUserPointer(ww))
                                     ->inputptr->KeyCallbackFunc(code, action); });
            glfwSetMouseButtonCallback(w,[](GLFWwindow *ww, int button, int action, int)
                               { ((Window *)glfwGetWindowUserPointer(ww))
                                     ->inputptr->MouseCallbackFunc(button, action); });
            //TODO add mouse position listener
            window.inputptr = this;
        }
        ~InputHandler()
        {
            glfwSetKeyCallback(window, nullptr);
        }
    };
}