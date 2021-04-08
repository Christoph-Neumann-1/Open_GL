///@file
#include <fstream>
#include <glad/glad.h>

#include <Log.hpp>
#include <Shader.hpp>

static Logger logger;

Shader::Shader(const std::string &filename)
    : filepath(filename)
{
    id = CreateShader(ParseShader());
}

Shader::~Shader()
{
    glDeleteProgram(id);
}

void Shader::Init(const std::string &filename)
{
    filepath = filename;
    id=CreateShader(ParseShader());
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
        logger << "Uniform not found name: " << name << " shader: " << filepath;
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

void Shader::SetUniformMat4f(const std::string &name, const glm::mat4 &matrix)
{
    glUniformMatrix4fv(GetUniformLocation(name), 1, false, &matrix[0][0]);
}

unsigned int Shader::CreateShader(ShaderSource source)
{
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(source.VertexSource, GL_VERTEX_SHADER);
    unsigned int fs = CompileShader(source.FragmentSource, GL_FRAGMENT_SHADER);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

Shader::ShaderSource Shader::ParseShader()
{
    enum class ShaderType
    {
        NONE = -1,
        VERTEX = 0,
        FRAGMENT = 1
    };

    ShaderType type = ShaderType::NONE;
    std::ifstream file(filepath);
    std::string line;
    std::stringstream ss[2];
    while (getline(file, line))
    {
        if (line.find("#shader") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos)
                type = ShaderType::VERTEX;

            else if (line.find("fragment") != std::string::npos)
                type = ShaderType::FRAGMENT;
        }
        else
        {
            ss[(int)type] << line << "\n";
        }
    }
    return {ss[0].str(), ss[1].str()};
}

unsigned int Shader::CompileShader(const std::string &source, unsigned int type)
{
    unsigned int id = glCreateShader(type);
    const char *src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char message[length];
        glGetShaderInfoLog(id, length, &length, message);

        logger << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader\n"
               << message << '\n';
        logger.print();

        glDeleteShader(id);
        return 0;
    }

    return id;
}
