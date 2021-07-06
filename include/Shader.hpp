#pragma once
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>
namespace GL
{

    /**
     * @brief This class stores a single shader program and allows you to modify uniforms .
     */
    class Shader
    {
        uint id;
        std::unordered_map<std::string, int> cache;
        std::string paths;

        int GetUniformLocation(const std::string &name);

        unsigned int CreateShader(const std::string &vsrc, const std::string &fsrc);
        std::string ParseShader(const std::string &src);
        uint CompileShader(const std::string &src, uint type);

    public:
        /**
         * @brief Compile and link the shader. 
         * 
         * @param vsrc vertex shader
         * @param fsrc fragment shader
         */
        Shader(const std::string &vsrc, const std::string &fsrc);
        ~Shader();

        ///@brief activate the shader
        void Bind() const;
        ///@brief bind shader 0
        void UnBind() const;


        //The following functions allow for setting uniform values. There are multiple ways to set some uniforms, for example the 
        //4f uniforms can be set using individual values or an array. The array make sense if you store colors in a vec4 for example.

        void SetUniform1i(const std::string &name, int value);   ///<Set an integer uniform
        void SetUniform1f(const std::string &name, float value); ///<Set a float uniform
        void SetUniform2f(const std::string &name, float v0, float v1);
        void SetUniform2f(const std::string &name, const float values[2]);
        void SetUniform4f(const std::string &name, float v0, float v1, float v2, float v3); ///<Set a vec4 uniform of floats
        void SetUniform4f(const std::string &name, const float values[4]);
        void SetUniform3f(const std::string &name, float v0, float v1, float v2);
        void SetUniform3f(const std::string &name, const float values[3]);
        void SetUniformMat4f(const std::string &name, const glm::mat4 &matrix); ///<Set a mat4 uniform
    };
}