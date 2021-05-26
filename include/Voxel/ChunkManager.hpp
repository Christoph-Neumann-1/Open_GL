#pragma once
#include <Voxel/Chunk.hpp>
#include <vector>
#include <glm/glm.hpp>
#include <Threadpool.hpp>
#include <filesystem>
#include <Data.hpp>

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

        int renderdist = 11;
        int preload = 1;

        std::atomic_uint count = 0;

        Chunk *GetFree()
        {
            Chunk *ptr;
            if (free.size() == 0)
            {
                ptr = new Chunk(config, cbh.GetList(CallbackType::PreRender), c_cbid);
                chunks.push_back(ptr);
            }
            else
            {
                ptr = free.back();
                free.pop_back();
            }
            return ptr;
        }

        void SetSeed()
        {
            auto file = fopen((ROOT_Directory + "/res/world/SEED").c_str(), "r");
            if (!file)
            {
                Chunk::NewSeed();
                StoreSeed();
            }
            else
            {
                fread(&Chunk::Seed, sizeof(int), 1, file);
                fclose(file);
            }
        }

        void StoreSeed()
        {

            auto file = fopen((ROOT_Directory + "/res/world/SEED").c_str(), "w");
            fwrite(&Chunk::Seed, sizeof(int), 1, file);
            fclose(file);
        }

    public:
        bool IsRendered(glm::ivec2 chunk, glm::ivec2 pos)
        {
            return chunk.x >= pos.x - renderdist && chunk.x <= pos.x + renderdist && chunk.y >= pos.y - renderdist && chunk.y <= pos.y + renderdist;
        }

        bool IsLoaded(glm::ivec2 chunk, glm::ivec2 pos)
        {
            return chunk.x >= pos.x - renderdist - preload && chunk.x <= pos.x + renderdist + preload &&
                   chunk.y >= pos.y - renderdist - preload && chunk.y <= pos.y + renderdist + preload;
        }

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
                    if (std::find_if(loaded.begin(), loaded.end(), [&](Chunk *chunk) { return chunk->GetPos() == glm::ivec2{x, z}; }) == loaded.end())
                    {
                        auto ptr = GetFree();
                        loaded.push_back(ptr);
                        ptr->PreLoad({x, z});
                        if (IsRendered({x, z}, pos))
                        {
                            rendered.push_back(ptr);
                            ptr->Load();
                        }
                    }
                    else if (IsRendered({x, z}, pos) && std::find_if(rendered.begin(), rendered.end(), [&](Chunk *chunk) { return chunk->GetPos() == glm::ivec2{x, z}; }) == rendered.end())
                    {
                        auto chunk = std::find_if(loaded.begin(), loaded.end(), [&](Chunk *chunk) { return chunk->GetPos() == glm::ivec2{x, z}; });
                        rendered.push_back(*chunk);
                        (*chunk)->Load();
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

            SetSeed();

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

        glm::ivec2 GetChunkPos(glm::ivec2 pos)
        {
            return {ceil((pos.x + 1) / 16.0f) - 1, ceil((pos.y + 1) / 16.0f) - 1};
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

            for (auto &file : std::filesystem::directory_iterator(ROOT_Directory + "/res/world"))
                std::filesystem::remove(file);

            if (count > 0)
                return;
            Chunk::NewSeed();
            StoreSeed();
            count = loaded.size();
            for (auto chunk : loaded)
            {
                pool.Add([&](Chunk *mchunk) {mchunk->Generate();mchunk->GenFaces(); count--; }, chunk);
            }
        }

        bool HasCrossedChunk(glm::ivec2 last, glm::ivec2 now)
        {
            return GetChunkPos(last) != GetChunkPos(now);
        }

        void UnLoadChunks(glm::ivec2 pos)
        {
            for (auto chunk = loaded.begin(); chunk != loaded.end();)
            {
                if (!IsLoaded((*chunk)->GetPos(), pos))
                {

                    (*chunk)->UnLoad();
                    if (auto chunk2 = std::find(rendered.begin(), rendered.end(), *chunk); !IsRendered((*chunk)->GetPos(), pos) && chunk2 != rendered.end())
                    {
                        rendered.erase(chunk2);
                    }
                    free.push_back(*chunk);
                    chunk = loaded.erase(chunk);
                }
                else
                    chunk++;
            }
        }

        void MoveChunk(glm::ivec2 position)
        {
            UnLoadChunks(position);
            LoadChunks(position);
        }
    };
}
