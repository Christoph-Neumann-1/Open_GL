#include <Window.hpp>

GL::Window::Window(GLFWwindow *_window, CallbackList &callback) : size_x(0), size_y(0), window(_window), ResizeCallback(callback)
{
}

GL::Window::~Window()
{
}