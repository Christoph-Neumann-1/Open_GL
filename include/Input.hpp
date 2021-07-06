#pragma once

#include <GLFW/glfw3.h>
#include <Window.hpp>
#include <unordered_map>
#include <Window.hpp>

namespace GL
{
    //TODO: allow for text input
    ///@attention Only create one instance per window.
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
        //Contains a list of all callbacks for each mouse button.
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

        //A list of all callbacks for a key.
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

        //This function is called every time something happens with a key. It then decides if a callback should be called.
        void KeyCallbackFunc(int code, int action)
        {
            //TODO  allow removing while called
            std::scoped_lock lk(mutex);
            if (callbacks.contains(code))
            {
                callbacks[code].Call(action);
            }
        }

        //Same for mouse buttons.
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
        /**
         * @brief This class allows you to register one callback for one key. 
         * 
         * While you can use the functions to add callbacks directly, I recommend this approach as
         * it removes the risk of forgetting to remove the callback which might lead to segfaults.
         * 
         * You can still change the key later on or remove the callback early, so there is no real reason not to use this class.
         */
        class KeyCallback
        {
            uint id = 0;
            InputHandler &handle;
            int scancode;

        public:
            /**
             * @brief Bind a new function to a key.
             * 
             * If a callback was bound before it will be removed.
             * The last parameter to the callback must be an integer. The integer is from glfw and represents the type of action.
             * @tparam F any function or lambda
             * @tparam arguments that need to be bound
             * @param code key scancode
             * @param type when the callback should be triggered
             * @param func //The function that should be called
             * @param args arguments that need to be bound
             */
            template <typename F, typename... Args>
            void Bind(int code, Action type, F &&func, Args... args)
            {
                UnBind();
                scancode = code;
                id = handle.AddKeyCallback(scancode, type, std::move(func), args...);
            }

            ///@overload
            template <typename F, typename... Args>
            void Bind(int code, Action type, const F &func, Args... args)
            {
                UnBind();
                scancode = code;
                id = handle.AddKeyCallback(scancode, type, func, args...);
            }

            ///@brief Disables the callback.
            void UnBind()
            {
                if (id == 0)
                    return;
                handle.RemoveKeyCallback(scancode, id);
            }

            /**
             * @brief Construct a new Key Callback object but don't bind it to a key.
             * 
             * @param handler the input handler of the window
             */
            KeyCallback(InputHandler &handler) : handle(handler) {}

            /**
             * @brief Construct a new Key Callback object and bind a callback directly
             * 
             * For more information see Bind()
             */
            template <typename F, typename... Args>
            KeyCallback(InputHandler &handler, int code, Action type, F &&func, Args... args) : handle(handler)
            {
                Bind(code, type, std::move(func), args...);
            }

            ///@overload
            template <typename F, typename... Args>
            KeyCallback(InputHandler &handler, int code, Action type, const F &func, Args... args) : handle(handler)
            {
                Bind(code, type, func, args...);
            }

            ///Simply unbinds any bound callback.
            ~KeyCallback()
            {
                UnBind();
            }
        };

        /**
         * @brief This class records the current state of the key.
         * 
         * Its main use is if you want to access the state of a key from a different thread.
         * 
         */
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

            ///@brief Returns the state of the key.
            const int &GetValue() { return value; }
        };

        /**
         * @brief Look at KeyCallback for more information.
         * 
         * Its almost the same just replace the scancode with something like GLFW_MOUSE_BUTTON_LEFT.
         * 
         */
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

        //The following functions add or remove callbacks from the lists. They all require the key or mouse button as well as an id.
        //The id is used to identify a callback. It is represented as a uint and there is only one counter for all callbacks.
        //If you register more callbacks than an uint can hold, you are doing something wrong.
        //All callbacks need an int as the last parameter. The int comes directly from glfw and represents the press or release.

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

        /** @brief This will bind the glfw key callback functions as well as set a value in the window class. 
         * 
         * That value is neccesary, because glfw only allows setting one user pointer so some class needs to store all 
         * information needed in callback funtions. It made sense to use the window class for that as it already used this user pointer.
         **/
        InputHandler(Window &w) : window(w)
        {

            glfwSetKeyCallback(w, [](GLFWwindow *ww, int, int code, int action, int)
                               { ((Window *)glfwGetWindowUserPointer(ww))
                                     ->inputptr->KeyCallbackFunc(code, action); });
            glfwSetMouseButtonCallback(w, [](GLFWwindow *ww, int button, int action, int)
                                       { ((Window *)glfwGetWindowUserPointer(ww))
                                             ->inputptr->MouseCallbackFunc(button, action); });
            //TODO add mouse position listener
            window.inputptr = this;
        }

        ///@brief This will remove the glfw callbacks. The user pointer is also set to null.
        ~InputHandler()
        {
            glfwSetKeyCallback(window, nullptr);
            glfwSetMouseButtonCallback(window, nullptr);
            window.inputptr = nullptr;
        }
    };
}