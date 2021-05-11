#pragma once
#include <Voxel/Block.hpp>

#include <stdio.h>
#include <string.h>
#include <glm/glm.hpp>

#define C_AT(x, y, z) x * 16 * 64 + y * 64 + z

namespace GL::Voxel
{
    class Chunk
    {
        int posx, posy;
        u_char *blocks;
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
        }

        void Load(int x, int y)
        {
            posx = x;
            posy = y;
            auto f = fopen((ROOT_Directory + "/res/world/" + std::to_string(posx) + "|" + std::to_string(posy)).c_str(), "r");
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
            auto f = fopen((ROOT_Directory + "/res/world/" + std::to_string(posx) + "|" + std::to_string(posy)).c_str(), "w+");
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