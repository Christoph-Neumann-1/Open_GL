#pragma once
#include <Voxel/Chunk.hpp>
#include <vector>
#include <glm/glm.hpp>

namespace GL::Voxel
{

    class ChunkManager
    {
        std::vector<Chunk *> chunks;
        std::vector<Chunk *> loaded;
        std::vector<Chunk *> free;
        std::vector<Chunk *> rendered;

        TexConfig &config;

        int renderdist = 4;
        int preload = 1;

    public:
        void LoadChunks(glm::ivec2 pos)
        {
            int xbegin = pos.x - (renderdist + preload);
            int xend = pos.x + (renderdist + preload);
            for (int x = xbegin; x <= xend; x++)
            {
                int zbegin = pos.y - (renderdist + preload);
                int zend = pos.y + (renderdist + preload);
                for (int z = zbegin; z <= zend; z++)
                {
                    auto ptr = free.back();
                    loaded.push_back(ptr);
                    free.pop_back();
                    ptr->PreLoad({x, z});
                    if (x >= xbegin + preload && x <= xend - preload && z >= zbegin + preload && z <= zend - preload)
                    {
                        rendered.push_back(ptr);
                        ptr->Load();
                    }
                }
            }
        }

        Chunk *GetChunk(glm::ivec2 pos)
        {
            auto res = std::find_if(loaded.begin(), loaded.end(), [&](Chunk *ptr) { return ptr->GetPos() == pos; });
            return res == rendered.end() ? nullptr : *res;
        }

        ChunkManager(glm::ivec2 starting_pos, TexConfig &cfg) : config(cfg)
        {
            chunks.reserve(2 * (renderdist + preload + 1) * 2 * (renderdist + preload + 1));
            for (int i = 0; i < 2 * (renderdist + preload + 1) * 2 * (renderdist + preload + 1); i++)
            {
                auto ptr = new Chunk(cfg);
                chunks.push_back(ptr);
                free.push_back(ptr);
            }
            LoadChunks(starting_pos);
        };

        ~ChunkManager()
        {
            for (auto chunk : chunks)
                delete chunk;
        }

        glm::ivec2 GetChunkPos(int x, int z)
        {
            return {ceil((x + 1) / 16.0f) - 1, ceil((z + 1) / 16.0f) - 1};
        }

        uint *GetBlockAt(int x, int y, int z)
        {
            auto chunk = GetChunk(GetChunkPos(x, z));
            return chunk ? &(*chunk)(x - 16 * chunk->GetPos().x, y, z - 16 * chunk->GetPos().y) : nullptr;
        }

        void DrawChunks()
        {
            for (auto chunk : rendered)
            {
                chunk->DrawOpaque();
            }
            for (auto chunk : rendered)

            {
                chunk->DrawTransparent();
            }
        };

        void Regenerate()
        {
            for (auto chunk : rendered)
            {
                chunk->Generate();
                chunk->GenFaces();
            }
        }
    };
}
