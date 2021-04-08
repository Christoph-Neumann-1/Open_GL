///@file
#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

/**
 * @brief Provides abstraction for creating a simple window.
 * 
 * Can be used like a GLFWwindow*
 */
class Window
{
    GLFWwindow *window;

public:
    int x, y;
    bool paused = false;
    Window(int size_x, int size_y, const char *name, unsigned short Major, unsigned short Minor, bool resizable = true, bool Core = true, int swap_interval = 1);
    inline void Swap() { glfwSwapBuffers(window); }
    /**
     * @brief Checks if the window chould close
     * 
     * @return 0 if the window should not close 1 for normal closing 
     */
    inline int CloseFlag() { return glfwWindowShouldClose(window); }

    ///@brief cast to a GLFWwindow*. Makes it easier to work with.
    operator GLFWwindow *() const { return window; }
};
