///@file
#include <ErrorHandler.hpp>
#include <Log.hpp>
static Logger l;

void GLAPIENTRY ErrorCallback(GLenum source,
                              GLenum type,
                              GLuint id,
                              GLenum severity,
                              GLsizei length,
                              const GLchar *message,
                              const void *userParam)
{
    if(type==0x8251) return;
    char messag[512];
    // std::abort(); //To generate core dump
    snprintf(messag, sizeof(char) * 511, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s",
             (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
             type, severity, message);
    l << messag;
    l.print();
}


void glfw_error_callback(int error, const char* description)
{
    l<<"Glfw Error:\t"<<error<<"\t"<<description;
    l.print();
}

