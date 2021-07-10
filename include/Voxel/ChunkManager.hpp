#pragma once
#include <Voxel/Chunk.hpp>
#include <vector>
#include <glm/glm.hpp>
#include <Threadpool.hpp>
#include <filesystem>
#include <Data.hpp>
#include <Voxel/Commonfs.hpp>

namespace GL::Voxel
{

    /**
     * @brief This class stores chunks and decides which chunks are visible and which need to be loaded/unloaded.
     */
    class ChunkManager
    {
        std::vector<Chunk *> chunks;     //< A list of all chunks.
        std::vector<Chunk *> free;       //< A list of unused chunks.
        std::vector<Chunk *> loaded;     //< Chunks with meshes(renderdist+1).
        std::vector<Chunk *> pre_loaded; //< Chunks where the data is loaded.
        std::vector<Chunk *> rendered;   //< Chunks that are rendered.

        TexConfig &config;
        ThreadPool pool;//Used for mesh building.
        CallbackHandler &cbh;
        uint cbid;
        uint c_cbid = cbh.GenId();

        int renderdist = 11;//How far the player can see
        int preload = 1;//How many chunk beyond mesh generation data is loades.

        std::atomic_uint count = 0;//Number of chunks which meshes are being rebuilt.

        FileLayout file;

        /**
         * @brief Get a free chunk. If no chunks are free, make a new one.
         */
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

        /**
         * @brief Try loading the seed from the file, if the file does not exist create a new seed.
         * 
         */
        void SetSeed()
        {

            if (!file.Load())
            {
                Chunk::NewSeed();
                file.Store();
            }
        }

    public:
        bool IsRendered(glm::ivec2 chunk, glm::ivec2 pos)
        {
            return chunk.x >= pos.x - renderdist && chunk.x <= pos.x + renderdist && chunk.y >= pos.y - renderdist && chunk.y <= pos.y + renderdist;
        }
        bool IsPreLoaded(glm::ivec2 chunk, glm::ivec2 pos)
        {
            return chunk.x >= pos.x - renderdist - preload && chunk.x <= pos.x + renderdist + preload &&
                   chunk.y >= pos.y - renderdist - preload && chunk.y <= pos.y + renderdist + preload;
        }
        bool IsLoaded(glm::ivec2 chunk, glm::ivec2 pos)
        {
            return chunk.x >= pos.x - renderdist - preload - 1 && chunk.x <= pos.x + renderdist + preload + 1 &&
                   chunk.y >= pos.y - renderdist - preload - 1 && chunk.y <= pos.y + renderdist + preload + 1;
        }

        /**
         * @brief Check which chunks need to be loaded, have meshes build and be rendered.
         */
        void LoadChunks(glm::ivec2 pos)
        {
            int xbegin = pos.x - (renderdist + preload + 1);
            int xend = pos.x + (renderdist + preload + 1);
            for (int x = xbegin; x <= xend; x++)
            {
                int zbegin = pos.y - (renderdist + preload + 1);
                int zend = pos.y + (renderdist + preload + 1);
                for (int z = zbegin; z <= zend; z++)
                {
                    if (std::find_if(loaded.begin(), loaded.end(), [&](Chunk *chunk)
                                     { return chunk->GetPos() == glm::ivec2{x, z}; }) == loaded.end())
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
                    else if (IsRendered({x, z}, pos) && std::find_if(rendered.begin(), rendered.end(), [&](Chunk *chunk)
                                                                     { return chunk->GetPos() == glm::ivec2{x, z}; }) == rendered.end())
                    {
                        auto chunk = std::find_if(loaded.begin(), loaded.end(), [&](Chunk *chunk)
                                                  { return chunk->GetPos() == glm::ivec2{x, z}; });
                        rendered.push_back(*chunk);
                        (*chunk)->Load();
                    }
                    else if (IsPreLoaded({x, z}, pos) && std::find_if(pre_loaded.begin(), pre_loaded.end(), [&](Chunk *chunk)
                                                                      { return chunk->GetPos() == glm::ivec2{x, z}; }) == pre_loaded.end())
                    {
                        auto chunk = std::find_if(loaded.begin(), loaded.end(), [&](Chunk *chunk)
                                                  { return chunk->GetPos() == glm::ivec2{x, z}; });
                        pre_loaded.push_back(*chunk);
                        (*chunk)->Load();
                    }
                }
            }
        }

        /**
         * @brief Get the chunk at the grid position.
         * 
         * @param pos from GetChunkPos
         */
        Chunk *GetChunk(glm::ivec2 pos)
        {
            auto res = std::find_if(loaded.begin(), loaded.end(), [&](Chunk *ptr)
                                    { return ptr->GetPos() == pos; });
            return res == rendered.end() ? nullptr : *res;
        }

        ChunkManager(TexConfig &cfg, CallbackHandler &cb) : config(cfg), pool(2), cbh(cb),
                                                            file(ROOT_Directory + "/res/world/SEED")
        {
            chunks.reserve(2 * (renderdist + preload + 1) * 2 * (renderdist + preload + 1));//No more chunks should be needed.
            auto &pre_render = cbh.GetList(CallbackType::PreRender);
            for (int i = 0; i < 2 * (renderdist + preload + 1) * 2 * (renderdist + preload + 1); i++)
            {
                auto ptr = new Chunk(cfg, pre_render, c_cbid);
                chunks.push_back(ptr);
                free.push_back(ptr);
            }

            file.AddElement<int>(&Chunk::Seed);

            SetSeed();

            //After rendering check if any chunks were modifiesd, if so, rebuild them.
            cbid = cbh.GetList(CallbackType::PostRender).Add([&]()
                                                             {
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

        ChunkManager(const ChunkManager &)=delete;
        ChunkManager& operator=(const ChunkManager &)=delete;

        /**
         * @brief Get the grid coordinates of a chunk using world coordinates.
         * 
         */
        glm::ivec2 GetChunkPos(int x, int z)
        {
            return {ceil((x + 1) / 16.0f) - 1, ceil((z + 1) / 16.0f) - 1};
        }

        ///@overload
        glm::ivec2 GetChunkPos(glm::ivec2 pos)
        {
            return {ceil((pos.x + 1) / 16.0f) - 1, ceil((pos.y + 1) / 16.0f) - 1};
        }

        /**
         * @brief First finds the coresponding chunk, then gets the block at that position.
         */
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

        /**
         * @brief Generate a new seed, delete world data and regenerate everything. If called again while the world is being rebuilt, the function will do nothing.
         * 
         */
        void Regenerate()
        {

            for (auto &file_ : std::filesystem::directory_iterator(ROOT_Directory + "/res/world"))
                std::filesystem::remove(file_);

            //Check if all chunks have finished rebuilding.
            if (count > 0)
                return;
            Chunk::NewSeed();
            file.Store();
            count = loaded.size();
            for (auto chunk : loaded)
            {
                pool.Add([&](Chunk *mchunk)
                         {
                             mchunk->Generate();
                             mchunk->GenFaces();
                             count--;
                         },
                         chunk);
            }
        }

        /**
         * @brief Used to check if the player has crossed a chunk boundary. If so new chunks must be loaded.
         */
        bool HasCrossedChunk(glm::ivec2 last, glm::ivec2 now)
        {
            return GetChunkPos(last) != GetChunkPos(now);
        }

        /**
         * @brief Check which chunks are no longer needed and remove them.
         * 
         * If the only stop being visible but are still supposed to be loaded,  they will still be in the corresponding lists.
         */
        void UnLoadChunks(glm::ivec2 pos)
        {
            for (auto chunk = loaded.begin(); chunk != loaded.end();)
            {
                if (!IsRendered((*chunk)->GetPos(), pos))
                {
                    if (auto chunk2 = std::find(rendered.begin(), rendered.end(), *chunk); !IsRendered((*chunk)->GetPos(), pos) && chunk2 != rendered.end())
                    {
                        rendered.erase(chunk2);
                    }
                }
                if (!IsPreLoaded((*chunk)->GetPos(), pos))
                {
                    if (auto chunk2 = std::find(rendered.begin(), rendered.end(), *chunk); !IsRendered((*chunk)->GetPos(), pos) && chunk2 != rendered.end())
                    {
                        pre_loaded.erase(chunk2);
                    }
                }
                if (!IsLoaded((*chunk)->GetPos(), pos))
                {
                    (*chunk)->UnLoad();
                    free.push_back(*chunk);
                    chunk = loaded.erase(chunk);
                }
                else
                    chunk++;
            }
        }

        /**
         * @brief This loads all the chunks around the player.
         * 
         * @param position In grid coords.
         */
        void MoveChunk(glm::ivec2 position)
        {
            UnLoadChunks(position);
            LoadChunks(position);
        }
    };
}
