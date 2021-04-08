///@file
#include <Window.hpp>
#include <Log.hpp>
#include <stdexcept>

Window::Window(int size_x, int size_y, const char *name, unsigned short Major, unsigned short Minor, bool resizable, bool Core, int swap_interval)
    : x(size_x), y(size_y)
{

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, Major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, Minor);
    glfwWindowHint(GLFW_RESIZABLE, resizable);

    glfwWindowHint(GLFW_OPENGL_PROFILE, Core ? GLFW_OPENGL_CORE_PROFILE : GLFW_OPENGL_COMPAT_PROFILE);
    window = glfwCreateWindow(x, y, name, NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        Logger l;
        l << "ERROR: Could not create window";
        l.print();
        throw std::runtime_error("GLFW INIT FAILED");
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(swap_interval);
}