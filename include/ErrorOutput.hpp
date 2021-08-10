/**
 * @file ErrorOutput.hpp
 * @brief Contains the error handling functions for opengl and glfw.
 * @author Christoph Neumann
 * @copyright Copyright © 2021 Christoph Neumann - MIT License
 */
#pragma once

#include <Logger.hpp>
#include <glad/glad.h>

static GL::Logger ErrorLogger;
/**
 * @brief Error callback for GLEW and Opengl
 * 
 * @note max length of errors is 512 char.
 * 
 * @param source Unused
 * @param type What kind of error.
 * @param id Unused
 * @param severity How important the error is.
 * @param length Unused
 * @param message Usefull Info about the callback
 * @param userParam Unused
 */
void GLAPIENTRY ErrorCallback(GLenum source,
                              GLenum type,
                              GLuint id,
                              GLenum severity,
                              GLsizei length,
                              const GLchar *message,
                              const void *userParam)
{
    if (type == 0x8251)
        return;
    char messag[512];
    // std::abort(); //To generate core dump
    snprintf(messag, sizeof(char) * 511, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s",
             (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
             type, severity, message);
    ErrorLogger << messag;
    ErrorLogger.print();
}
/**
 * @brief Error callback for GLFW
 * 
 * @param error the id of the error
 * @param description Why it occured, how to resolve it, ...
 */
void glfw_error_callback(int error, const char *description)
{
    ErrorLogger << "Glfw Error:\t" << error << "\t" << description;
    ErrorLogger.print();
}