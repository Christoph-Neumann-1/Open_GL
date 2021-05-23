#pragma once
#include <Voxel/Block.hpp>

#include <Noise/FastNoiseLite.h>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <array>
#include <string.h>
#include <Voxel/ConfigReader.hpp>

namespace GL::Voxel
{
    class Chunk
    {
        const static int sealevel = 8;
        static int Seed;

        uint &At(glm::ivec3 pos)
        {
            return blocks[pos.x][pos.y][pos.z];
        }

    public:
        static void NewSeed()
        {
            int nseed = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
            seed48((u_short *)&nseed);
            Seed = rand();
        }
        enum BlockTypes
        {
            BAir,
            BGrass,
            BDirt,
            BStone,
            BWater,
            BSand,
            BWood,
        };

    private:
        std::array<uint, 7> lookup_cache;

        uint &lookup(BlockTypes index)
        {
            return lookup_cache[index - 1];
        };
        struct Face
        {
            struct Vertex
            {
                glm::vec3 pos;
                glm::vec3 tex;
            };
            std::array<Vertex, 6> vertices;
        };

        enum FaceIndices
        {
            Front = 0,
            Back = 6,
            Bottom = 12,
            Top = 18,
            Right = 24,
            Left = 30
        };

        bool IsTransparent(int x, int y, int z)
        {
            switch (blocks[x][y][z])
            {
            case BAir:
                return true;
            case BWater:
                return true;
            default:
                return false;
            }
        }

        glm::ivec2 chunk_offset;
        std::vector<Face> faces;
        std::vector<Face> faces_transparent;
        std::array<std::array<std::array<uint, 16>, 64>, 16> blocks;
        uint buffer, buffer_transparent, va, va_transparent;
        const TexConfig &config;

        Face GenFace(glm::ivec3 pos, FaceIndices type);

    public:
        void UpdateCache();

        void GenFaces();

        void GenTree(int x, int y, int z)
        {
            int heigth = y + 4 + rand() % 4;
            for (int i = y; i <= heigth; i++)
            {
                blocks[x][i][z] = BWood;
            }
        }

        Chunk(glm::ivec2 position, const TexConfig &cfg);
        

        ~Chunk()
        {
            glDeleteBuffers(2, &buffer);
            glDeleteVertexArrays(2, &va);
        }

        void DrawOpaque()
        {
            glBindVertexArray(va);
            glDrawArrays(GL_TRIANGLES, 0, 6 * faces.size());
            glBindVertexArray(0);
        }

        void DrawTransparent()
        {
            glBindVertexArray(va_transparent);
            glDrawArrays(GL_TRIANGLES, 0, 6 * faces_transparent.size());
            glBindVertexArray(0);
        }

        uint &operator()(int x, int y, int z)
        {
            return blocks[x][y][z];
        }

        glm::ivec2 GetPos()
        {
            return chunk_offset;
        }
    };
}