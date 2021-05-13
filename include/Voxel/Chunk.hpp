#pragma once
#include <Voxel/Block.hpp>

#include <glm/glm.hpp>
#include <glad/glad.h>

namespace GL::Voxel
{
    class Chunk
    {
        glm::ivec2 chunk_offset;
        std::vector<glm::vec4> blocks;
        uint buffer, va;

    public:
        Chunk(uint vb, glm::ivec2 position) : chunk_offset(position)
        {
            for (int i = 0; i < 16; i++)
            {
                for (int j = 0; j < 16; j++)
                {
                    for (int k = 0; k < 16; k++)
                    {
                        blocks.emplace_back(i + chunk_offset.x * 16, j, k + chunk_offset.y * 16, 0);
                    }
                }
            }

            glGenVertexArrays(1, &va);
            glGenBuffers(1, &buffer);

            glBindVertexArray(va);

            glBindBuffer(GL_ARRAY_BUFFER, vb);
            glVertexAttribPointer(0, 3, GL_FLOAT, false, 5 * sizeof(float), 0);
            glVertexAttribPointer(1, 2, GL_FLOAT, false, 5 * sizeof(float), (void *)(3 * sizeof(float)));
            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);

            glBindBuffer(GL_ARRAY_BUFFER, buffer);
            glBufferData(GL_ARRAY_BUFFER, 4 * blocks.size() * sizeof(float), &blocks[0], GL_STATIC_DRAW);
            glVertexAttribPointer(2, 3, GL_FLOAT, false, 4 * sizeof(float), 0);
            glVertexAttribPointer(3, 1, GL_FLOAT, false, 4 * sizeof(float), (void *)(3 * sizeof(float)));
            glEnableVertexAttribArray(2);
            glEnableVertexAttribArray(3);
            glVertexAttribDivisor(2, 1);
            glVertexAttribDivisor(3, 1);

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
        }

        ~Chunk()
        {
            glDeleteBuffers(1, &buffer);
            glDeleteVertexArrays(1, &va);
        }

        void Draw()
        {
            glBindVertexArray(va);
            glDrawArraysInstanced(GL_TRIANGLES, 0, 36, blocks.size());
            glBindVertexArray(0);
        }
    };
}