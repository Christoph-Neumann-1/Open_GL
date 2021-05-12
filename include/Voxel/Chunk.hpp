#pragma once
#include <Voxel/Block.hpp>

#include <stdio.h>
#include <string.h>
#include <glm/glm.hpp>
#include <glad/glad.h>

#define C_AT(x, y, z) x * 16 * 64 + y * 64 + z

namespace GL::Voxel
{
    class Chunk
    {
        int posx, posy;
        uint va, texbuf, posbuf;
        u_char *blocks;
        float *b_float;
        glm::vec3 *positions;

        void SetupPos()
        {
        }
        void Setup()
        {
            memset(blocks, 0, 16 * 16 * 64);
        }

    public:
        Chunk()
        {
            positions = reinterpret_cast<glm::vec3 *>(malloc(16 * 16 * 64 * sizeof(glm::vec3)));
            blocks = reinterpret_cast<u_char *>(malloc(16 * 16 * 64));
            b_float = reinterpret_cast<float *>(malloc(16 * 16 * 64 * 4));
            for (int i = 0; i < 16 * 16 * 64; i++)
                b_float[i] = blocks[i];
            glGenVertexArrays(1, &va);
            glGenBuffers(2, &texbuf);
            glBindVertexArray(va);

            glBindBuffer(GL_ARRAY_BUFFER, vb);
            glVertexAttribPointer(0, 3, GL_FLOAT, false, 5 * sizeof(float), 0);
            glVertexAttribPointer(1, 2, GL_FLOAT, false, 5 * sizeof(float), (void *)(3 * sizeof(float)));
            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);

            glBindBuffer(GL_ARRAY_BUFFER, posbuf);
            glBufferData(GL_ARRAY_BUFFER, 16 * 16 * 64 * 3 * sizeof(float), b_float, GL_STATIC_DRAW);
            glVertexAttribPointer(2, 3, GL_FLOAT, false, 3 * sizeof(float), 0);
            glEnableVertexAttribArray(2);
            glVertexAttribDivisor(2, 1);

            glBindBuffer(GL_ARRAY_BUFFER, texbuf);
            glBufferData(GL_ARRAY_BUFFER, 16 * 16 * 64 * sizeof(float), b_float, GL_STATIC_DRAW);
            glVertexAttribPointer(3, 1, GL_FLOAT, false, sizeof(float), 0);
            glEnableVertexAttribArray(3);
            glVertexAttribDivisor(3, 1);

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
        }

        void Load(int x, int y)
        {
            posx = x;
            posy = y;
            auto f = fopen((ROOT_Directory + "/res/world/" + std::to_string(posx) + "|" + std::to_string(posy)).c_str(), "a+b");
            rewind(f);
            if (getc(f) == EOF)
            {
                ungetc(EOF, f);
                Setup();
                printf("Not found\n");
            }
            else
            {
                fread(blocks, 1, 16 * 16 * 64, f);
            }
            SetupPos();
            fclose(f);
        }

        void Store()
        {
            auto f = fopen((ROOT_Directory + "/res/world/" + std::to_string(posx) + "|" + std::to_string(posy)).c_str(), "a+b");
            rewind(f);
            fwrite(blocks, 1, 16 * 16 * 64, f);
            fclose(f);
        }

        ~Chunk()
        {
            free(blocks);
            free(positions);
        }

        u_char GetTypeAtIndex(int x, int y, int z)
        {
            return blocks[C_AT(x, y, z)];
        }
    };
}