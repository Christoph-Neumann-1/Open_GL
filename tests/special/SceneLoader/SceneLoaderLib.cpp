#include <Window.hpp>

GL::Window::Window(GLFWwindow *_window, CallbackList &callback,int refresh) : size_x(0), size_y(0), window(_window), ResizeCallback(callback), refreshrate(refresh)
{
}

GL::Window::~Window()
{
}