#include <Shader.hpp>
#include <Data.hpp>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <cmath>

namespace GL
{
    struct Circle
    {
        uint VBO, VAO;
        Shader shader;
        uint vcount = 0;

        //TODO: use external shader and add support for instancing
        Circle(std::string vshader, std::string fshader)
            : shader(vshader, fshader)
        {
            glGenBuffers(1, &VBO);
            glGenVertexArrays(1, &VAO);
            glBindVertexArray(VAO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, false, 2 * sizeof(float), 0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
        }
        ~Circle()
        {
            glDeleteBuffers(1, &VBO);
            glDeleteVertexArrays(1, &VAO);
        }

        void ComputeVertices(float radius, uint segcount)
        {
            vcount = segcount + 2;
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
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
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        //TODO: add a draw function
    };
}