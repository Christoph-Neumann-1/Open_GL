#include <Shader.hpp>
#include <Data.hpp>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <cmath>
#include <Buffer.hpp>

namespace GL
{
    struct Circle
    {
        Buffer VBO;
        uint VAO;
        Shader shader;
        uint vcount = 0;

        //TODO: use external shader and add support for instancing
        Circle(std::string vshader, std::string fshader)
            : shader(vshader, fshader)
        {
            glGenVertexArrays(1, &VAO);
            glBindVertexArray(VAO);
            VBO.Bind(GL_ARRAY_BUFFER);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, false, 2 * sizeof(float), 0);
            Buffer::Unbind(GL_ARRAY_BUFFER);
            glBindVertexArray(0);
        }
        ~Circle()
        {
            glDeleteVertexArrays(1, &VAO);
        }

        void ComputeVertices(float radius, uint segcount)
        {
            vcount = segcount + 2;
            VBO.Bind(GL_ARRAY_BUFFER);
            glm::vec2 vertices[vcount];
            vertices[0] = {0, 0};
            for (uint i = 1; i <= segcount; i++)
            {
                vertices[i] = {radius * cos(i * 2 * M_PI / segcount),
                               radius * sin(i * 2 * M_PI / segcount)};
            }
            vertices[segcount + 1] = {radius * cos(2 * M_PI / segcount),
                                      radius * sin(2 * M_PI / segcount)};
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
            Buffer::Unbind(GL_ARRAY_BUFFER);
        }

        //TODO: add a draw function
    };
}