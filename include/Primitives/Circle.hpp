#include <Shader.hpp>
#include <Data.hpp>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <cmath>
#include <Buffer.hpp>
#include <VertexArray.hpp>

namespace GL
{
    struct Circle
    {
        Buffer VBO;
        VertexArray VAO;
        Shader shader;
        uint vcount = 0;

        //TODO: use external shader and add support for instancing
        //TODO: allow for the vertices to be generated without creating buffers and such
        Circle(std::string vshader, std::string fshader)
            : shader(vshader, fshader)
        {
            VertexBufferLayout layout;
            layout.stride=sizeof(glm::vec2);
            layout.Push({GL_FLOAT,2,0});
            layout.BindAndAddToVertexArray(VAO,VBO);
        }

        void ComputeVertices(float radius, uint segcount)
        {
            vcount = segcount + 2;
            VBO.Bind(GL_ARRAY_BUFFER);
            glm::vec2 *vertices =new glm::vec2[vcount];
            vertices[0] = {0, 0};
            for (uint i = 1; i <= segcount; i++)
            {
                vertices[i] = {radius * cos(i * 2 * M_PI / segcount),
                               radius * sin(i * 2 * M_PI / segcount)};
            }
            vertices[segcount + 1] = {radius * cos(2 * M_PI / segcount),
                                      radius * sin(2 * M_PI / segcount)};
            glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2)*vcount, vertices, GL_STATIC_DRAW);
            Buffer::Unbind(GL_ARRAY_BUFFER);
            delete[] vertices;
        }

        //TODO: add a draw function
    };
}