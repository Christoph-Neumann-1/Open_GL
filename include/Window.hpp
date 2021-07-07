#pragma once
#include <atomic>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <Callback.hpp>
#include <glm/glm.hpp>

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

        static void Resize(GLFWwindow *window, int x, int y)
        {
            auto _this = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
            _this->size_x = x;
            _this->size_y = y;
            glViewport(0, 0, x, y);
            _this->ResizeCallback();
        }
        Window(const Window &) = delete;
        Window &operator=(const Window &) = delete;

    public:
        InputHandler *inputptr;
        constexpr static glm::vec4 defaultbg{21 / 255.0, 132 / 255.0, 201 / 255.0, 1};
        glm::vec4 bgcolor{21 / 255.0, 132 / 255.0, 201 / 255.0, 1};
        Window(GLFWwindow *window, CallbackList &callback);

        ~Window();

        uint GetWidth() const { return size_x; }
        uint GetHeigth() const { return size_y; }

        operator GLFWwindow *() const { return window; }

        /**
         * @brief Switch between fullscreen and windowed.
         * 
         * The window will have the size of the primary monitor.
         * 
         * @param fs 
         */
        void SetFullscreen(bool fs)
        {
            GLFWmonitor *monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode *mode = glfwGetVideoMode(monitor);
            if (fs)
            {
                glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
            }
            else
            {
                ///-1 Prevents weird issue where it stays full screen.
                glfwSetWindowMonitor(window, nullptr, 0, 0, mode->width, mode->height - 1, 0);
            }
        }
        ///Check if the window is in fullscreen mode.
        bool IsFullscreen()
        {
            return glfwGetWindowMonitor(window) != nullptr;
        }
    };
}