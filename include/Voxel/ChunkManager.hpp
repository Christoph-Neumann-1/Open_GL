#pragma once
#include <Voxel/Chunk.hpp>
#include <vector>
#include <glm/glm.hpp>
#include <Threadpool.hpp>

namespace GL::Voxel
{

    class ChunkManager
    {
        std::vector<Chunk *> chunks;
        std::vector<Chunk *> loaded;
        std::vector<Chunk *> free;
        std::vector<Chunk *> rendered;

        TexConfig &config;
        ThreadPool pool;
        CallbackHandler &cbh;
        uint cbid;
        uint c_cbid = cbh.GenId();

        int renderdist = 4;
        int preload = 1;

        std::atomic_uint count = 0;

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

        ChunkManager(glm::ivec2 starting_pos, TexConfig &cfg, CallbackHandler &cb) : config(cfg), pool(2), cbh(cb)
        {
            chunks.reserve(2 * (renderdist + preload + 1) * 2 * (renderdist + preload + 1));
            auto &pre_render = cbh.GetList(CallbackType::PreRender);
            for (int i = 0; i < 2 * (renderdist + preload + 1) * 2 * (renderdist + preload + 1); i++)
            {
                auto ptr = new Chunk(cfg, pre_render, c_cbid);
                chunks.push_back(ptr);
                free.push_back(ptr);
            }
            LoadChunks(starting_pos);

            cbid = cbh.GetList(CallbackType::PostRender).Add([&]() {
                for (auto chunk : rendered)
                {
                    if (chunk->regen_mesh)
                    {
                        chunk->regen_mesh = false;
                        pool.Add(&Chunk::GenFaces, chunk);
                    }
                }
            });
        };

        ~ChunkManager()
        {
            pool.Terminate();
            for (auto chunk : chunks)
                delete chunk;
            cbh.GetList(CallbackType::PostRender).Remove(cbid);
            cbh.RemoveAll(c_cbid);
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
            if (count > 0)
                return;
            Chunk::NewSeed();
            count = rendered.size();
            for (auto chunk : rendered)
            {
                pool.Add([&](Chunk *mchunk) {mchunk->Generate();mchunk->GenFaces(); count--;}, chunk);
            }
        }
    };
}
