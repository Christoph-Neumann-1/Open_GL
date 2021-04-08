///@file
#pragma once
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

/**
 * @brief A class intended to make working with Shaders easier.
 */
class Shader
{
    struct ShaderSource
    {
        std::string VertexSource;
        std::string FragmentSource;
    };
    unsigned int id;
    std::string filepath;
    std::unordered_map<std::string, int> cache;

    int GetUniformLocation(const std::string &name);

    unsigned int CreateShader(ShaderSource source);
    ShaderSource ParseShader();
    unsigned int CompileShader(const std::string &source, unsigned int type);

public:
    /**
     * @brief Construct a new Shader and compile the program found at filename
     * 
     * @param filename Path to the shader
     */
    explicit Shader(const std::string &filename);
    Shader(){}
    ~Shader();

    void Init(const std::string &filename);

    void Bind() const;   ///<Bind the Shader
    void UnBind() const; ///<Bind Shader 0 (Unbind it)

    void SetUniform1i(const std::string &name, int value);                              ///<Set an integer uniform
    void SetUniform1f(const std::string &name, float value);                            ///<Set a float uniform
    void SetUniform4f(const std::string &name, float v0, float v1, float v2, float v3); ///<Set a vec4 uniform of floats
    void SetUniform4f(const std::string &name, const float values[4]);                  ///<Same as @ref SetUniform4f(const std::string &name, float v0, float v1, float v2, float v3) "above" but the parameter is an array
    void SetUniformMat4f(const std::string &name, const glm::mat4 &matrix);             ///<Set a mat4 uniform
};