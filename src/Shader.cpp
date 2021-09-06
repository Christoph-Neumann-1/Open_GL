/**
 * @file Shader.cpp
 * @author Christoph Neumann
 * @copyright Copyright © 2021 Christoph Neumann - MIT License
 */
#include <fstream>
#include <glad/glad.h>

#include <Logger.hpp>
#include <Shader.hpp>

using namespace GL;

static Logger logger;

Shader::Shader(const std::string &vsrc, const std::string &fsrc)
    : id(CreateShader(vsrc, fsrc)), paths(vsrc + "   " + fsrc)
{
}

Shader::~Shader()
{
    glDeleteProgram(id);
}

void Shader::Bind() const
{
    glUseProgram(id);
}

void Shader::UnBind() const
{
    glUseProgram(0);
}

int Shader::GetUniformLocation(const std::string &name)
{
    if (cache.find(name) != cache.end())
        return cache[name];

    auto uid = glGetUniformLocation(id, name.c_str());

    if (uid == -1)
    {
        logger << "Uniform not found name: " << name << " shader: " << paths;
        logger.print();
    }
    cache[name] = uid;
    return uid;
}

void Shader::SetUniform1i(const std::string &name, int value)
{
    glUniform1i(GetUniformLocation(name), value);
}

void Shader::SetUniform1f(const std::string &name, float value)
{
    glUniform1f(GetUniformLocation(name), value);
}

void Shader::SetUniform4f(const std::string &name, float v0, float v1, float v2, float v3)
{
    glUniform4f(GetUniformLocation(name), v0, v1, v2, v3);
}

void Shader::SetUniform4f(const std::string &name, const float values[4])
{
    glUniform4f(GetUniformLocation(name), values[0], values[1], values[2], values[3]);
}

void Shader::SetUniform4f(const std::string &name, const glm::vec4 values)
{
    glUniform4f(GetUniformLocation(name), values[0], values[1], values[2], values[3]);
}

void Shader::SetUniform3f(const std::string &name, float v0, float v1, float v2)
{
    glUniform3f(GetUniformLocation(name), v0, v1, v2);
}

void Shader::SetUniform3f(const std::string &name, const float values[3])
{
    glUniform3f(GetUniformLocation(name), values[0], values[1], values[2]);
}

void Shader::SetUniform3f(const std::string &name, const glm::vec3 values)
{
    glUniform3f(GetUniformLocation(name), values[0], values[1], values[2]);
}

void Shader::SetUniform2f(const std::string &name, float v0, float v1)
{
    glUniform2f(GetUniformLocation(name), v0, v1);
}

void Shader::SetUniform2f(const std::string &name, const float values[2])
{
    glUniform2f(GetUniformLocation(name), values[0], values[1]);
}

void Shader::SetUniform2f(const std::string &name, const glm::vec2 values)
{
    glUniform2f(GetUniformLocation(name), values[0], values[1]);
}

void Shader::SetUniformMat4f(const std::string &name, const glm::mat4 &matrix)
{
    glUniformMatrix4fv(GetUniformLocation(name), 1, false, &matrix[0][0]);
}

unsigned int Shader::CreateShader(const std::string &vsrc, const std::string &fsrc)
{
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(ParseShader(vsrc), GL_VERTEX_SHADER);
    unsigned int fs = CompileShader(ParseShader(fsrc), GL_FRAGMENT_SHADER);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

std::string Shader::ParseShader(const std::string &src)
{
    std::ifstream file(src);
    if(!file.is_open())
    {
        logger << "Shader file not found: " << src;
        logger.print();
        return "";
    }
    std::string line;
    std::stringstream ss;
    while (getline(file, line))
    {
        ss << line << "\n";
    }
    return ss.str();
}

unsigned int Shader::CompileShader(const std::string &src, unsigned int type)
{
    unsigned int s_id = glCreateShader(type);
    const char *src_cs = src.c_str();
    glShaderSource(s_id, 1, &src_cs, nullptr);
    glCompileShader(s_id);

    int result;
    glGetShaderiv(s_id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        int length;
        glGetShaderiv(s_id, GL_INFO_LOG_LENGTH, &length);
        char message[length];
        glGetShaderInfoLog(s_id, length, &length, message);

        logger << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader Paths: " << paths << '\n'
               << message << '\n';
        logger.print();

        glDeleteShader(s_id);
        return 0;
    }

    return s_id;
}