#pragma once
#include <Voxel/Block.hpp>

#include <Noise/FastNoiseLite.h>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <array>
#include <string.h>
#include <Voxel/ConfigReader.hpp>
#include <Callback.hpp>
#include <Buffer.hpp>
#include <VertexArray.hpp>

//TODO: Move ops to render thread. Limit chunk updates and check if the chunk is still needed
namespace GL::Voxel
{

    class ChunkManager;
    /**
     * @brief This class stores the 16*64*16 blocks.
     * 
     * Chunks should always be heap allocated.
     */
    class Chunk
    {
        const static int sealevel = 8;

        /**
         * @brief Because the blocks are stored in an one dimensional array, I need an easy way to acess them.
         */
        uint &At(glm::ivec3 pos)
        {
            return blocks[pos.x][pos.y][pos.z];
        }

    public:
        bool isactive = false; //Prevents unneccesary mesh building.
        static int Seed;
        bool regen_mesh = false; //The mesh is rebuilt after rendering.
        static void NewSeed()
        {
            int nseed = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
            srand(nseed);
            Seed = rand();
        }

        struct Face
        {
            struct Vertex
            {
                glm::vec3 pos;
                glm::vec3 tex;
            };
            std::array<Vertex, 6> vertices;
        };

    private:
        std::array<uint, NBLOCKS> lookup_cache;

        /**
         * @brief Used to find the correct vertices in the array in the block file.
         * 
         */
        enum FaceIndices
        {
            Front = 0,
            Back = 6,
            Bottom = 12,
            Top = 18,
            Right = 24,
            Left = 30
        };

        glm::ivec2 chunk_offset; //Grid position of the chunk.
        std::array<std::array<std::array<uint, 16>, 64>, 16> blocks;
        Buffer buffer, buffer_transparent;
        VertexArray va, va_transparent;
        const TexConfig &config;
        uint nFaces, nFacesTp;

        const std::function<uint *(int, int, int)> GetBlockOtherChunk;

        Face GenFace(glm::ivec3 pos, FaceIndices type);

    public:
        glm::ivec3 ToWorldCoords(int x, int y, int z) const
        {
            return {chunk_offset.x * 16 + x, y, chunk_offset.y * 16 + z};
        }

        glm::ivec3 ToLocalCoords(glm::ivec3 pos) const
        {
            return {pos.x - chunk_offset.x * 16, pos.y, pos.z - chunk_offset.y * 16};
        }

        /**
         * @brief Generate new blocks using the seed.
         * 
         */
        void Generate();

        /**
         * @brief Figures out which faces are visible and generates them.
         */
        void GenFaces(std::vector<Face> &faces, std::vector<Face> &faces_transparent);

        /**
         * @brief For now trees are only a chunk of wood.
         */
        void GenTree(int x, int y, int z)
        {
            int heigth = y + 4 + rand() % 4;
            for (int i = y; i <= heigth; i++)
            {
                blocks[x][i][z] = BWood;
            }
        }

        Chunk(const TexConfig &cfg, std::function<uint *(int, int, int)>);

        Chunk(const Chunk &) = delete;
        Chunk &operator=(const Chunk &) = delete;

        /**
         * @brief Generates the mesh
         * 
         */
        void Load();

        /**
         * @brief Load data from the disk or generate new data.
         * 
         * @param position Grid position of the chunk.
         */
        void PreLoad(glm::ivec2 position);

        /**
         * @brief Saves the chunk to disk.
         */
        void UnLoad();

        ~Chunk()
        {
            if (isactive)
                UnLoad();
        }

        void DrawOpaque()
        {
            va.Bind();
            glDrawArrays(GL_TRIANGLES, 0, 6 * nFaces);
            VertexArray::Unbind();
        }

        void DrawTransparent()
        {
            va_transparent.Bind();
            glDrawArrays(GL_TRIANGLES, 0, 6 * nFacesTp);
            VertexArray::Unbind();
        }

        /// @brief Access block at a position.
        uint &operator()(int x, int y, int z)
        {
            return At({x, y, z});
        }

        // @brief Returns the grid position of the chunk.
        glm::ivec2 GetPos()
        {
            return chunk_offset;
        }
    };

}