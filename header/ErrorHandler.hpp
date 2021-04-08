/**
 * @file ErrorHandler.hpp
 * @brief contains the callbacks for GL and GLFW errors.
 */

#pragma once
#include <glad/glad.h>

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
                              const void *userParam);

/**
 * @brief Error callback for GLFW
 * 
 * @param error the id of the error
 * @param description Why it occured, how to resolve it, ...
 */
void glfw_error_callback(int error, const char* description);