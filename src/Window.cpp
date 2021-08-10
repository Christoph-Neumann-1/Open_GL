/**
 * @file Window.cpp
 * @author Christoph Neumann
 * @copyright Copyright © 2021 Christoph Neumann - MIT License
 */
#include <Window.hpp>

GL::Window::Window(GLFWwindow *_window, CallbackList &callback) : window(_window), ResizeCallback(callback)
{
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, Resize);
    glfwGetFramebufferSize(window, &size_x, &size_y);
}

GL::Window::~Window()
{
    glfwSetWindowUserPointer(window, NULL);
    glfwSetFramebufferSizeCallback(window, NULL);
}