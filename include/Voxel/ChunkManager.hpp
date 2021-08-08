#pragma once
#include <Voxel/Chunk.hpp>
#include <vector>
#include <glm/glm.hpp>
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
        std::vector<Chunk *> chunks;        //< A list of all chunks.
        std::vector<Chunk *> free;          //< A list of unused chunks.
        std::vector<Chunk *> loaded;        //< Chunks where the data is loaded. Necessary for mesh building.
        std::vector<Chunk *> meshed_chunks; //< Chunks with meshes(renderdist+1).
        std::vector<Chunk *> rendered;      //< Chunks that are currently rendered.

        TexConfig &config;
        CallbackHandler &cbh;
        CallbackId callbackId;

        glm::ivec2 LastPlayerChunk{0,0};

        std::vector<Chunk::Face> faces;
        std::vector<Chunk::Face> faces_transparent;

    public:
        static const int renderdist = 11; //How far the player can see
        static const int preMeshed = 1;   //How many chunk beyond mesh generation data is loaded.
        static const int preLoaded = 1;

    private:
        FileLayout file; //Stores the seed

        /**
         * @brief Get a free chunk. If no chunks are free, make a new one.
         */
        Chunk *GetFree()
        {
            Chunk *ptr;
            if (free.size() == 0)
            {
                Logger()("Ran out of free chunks, allocating more memory");
                ptr = new Chunk(config, std::bind(&ChunkManager::GetBlockAt, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
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
        ///@brief Checks if the chunk should be drawn.
        bool IsRendered(glm::ivec2 chunk, glm::ivec2 pos)
        {
            return chunk.x >= pos.x - renderdist && chunk.x <= pos.x + renderdist && chunk.y >= pos.y - renderdist && chunk.y <= pos.y + renderdist;
        }
        ///@brief Checks if the Chunk should have a mesh.
        bool IsMeshed(glm::ivec2 chunk, glm::ivec2 pos)
        {
            return chunk.x >= pos.x - renderdist - preMeshed && chunk.x <= pos.x + renderdist + preMeshed &&
                   chunk.y >= pos.y - renderdist - preMeshed && chunk.y <= pos.y + renderdist + preMeshed;
        }
        ///@brief Checks if the chunk should be in memory
        bool IsLoaded(glm::ivec2 chunk, glm::ivec2 pos)
        {
            return chunk.x >= pos.x - renderdist - preMeshed - preLoaded && chunk.x <= pos.x + renderdist + preMeshed + preLoaded &&
                   chunk.y >= pos.y - renderdist - preMeshed - preLoaded && chunk.y <= pos.y + renderdist + preMeshed + preLoaded;
        }

        /**
         * @brief Check which chunks need to be loaded, have meshes build and be rendered.
         */
        void LoadChunks(glm::ivec2 pos)
        {
            int xbegin = pos.x - (renderdist + preMeshed + preLoaded);
            int xend = pos.x + (renderdist + preMeshed + preLoaded);
            for (int x = xbegin; x <= xend; x++)
            {
                int zbegin = pos.y - (renderdist + preMeshed + preLoaded);
                int zend = pos.y + (renderdist + preMeshed + preLoaded);
                for (int z = zbegin; z <= zend; z++)
                {
                    if (std::find_if(loaded.begin(), loaded.end(), [&](Chunk *chunk)
                                     { return chunk->GetPos() == glm::ivec2{x, z}; }) == loaded.end())
                    {
                        auto ptr = GetFree();
                        loaded.push_back(ptr);
                        ptr->PreLoad({x, z});
                    }
                    if (IsMeshed({x, z}, pos) && std::find_if(meshed_chunks.begin(), meshed_chunks.end(), [&](Chunk *chunk)
                                                              { return chunk->GetPos() == glm::ivec2{x, z}; }) == meshed_chunks.end())
                    {
                        auto chunk = std::find_if(loaded.begin(), loaded.end(), [&](Chunk *_chunk)
                                                  { return _chunk->GetPos() == glm::ivec2{x, z}; });
                        meshed_chunks.push_back(*chunk);
                        (*chunk)->Load();
                    }
                    if (IsRendered({x, z}, pos) && std::find_if(rendered.begin(), rendered.end(), [&](Chunk *chunk)
                                                                { return chunk->GetPos() == glm::ivec2{x, z}; }) == rendered.end())
                    {
                        auto chunk = std::find_if(meshed_chunks.begin(), meshed_chunks.end(), [&](Chunk *_chunk)
                                                  { return _chunk->GetPos() == glm::ivec2{x, z}; });
                        rendered.push_back(*chunk);
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
            return res == loaded.end() ? nullptr : *res;
        }

        ChunkManager(TexConfig &cfg, CallbackHandler &cb) : config(cfg), cbh(cb),
                                                            file(ROOT_Directory + "/res/world/SEED")
        {
            chunks.reserve(2 * (renderdist + preMeshed + 1) * 2 * (renderdist + preMeshed + 1)); //No more chunks should be needed.
            //Plus 1 because the chunk the player is standing in counts as well
            for (int i = 0; i < (2 * (renderdist + preMeshed + preLoaded) + 1) * (2 * (renderdist + preMeshed + preLoaded) + 1); i++)
            {
                auto ptr = new Chunk(cfg, std::bind(&ChunkManager::GetBlockAt, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
                chunks.push_back(ptr);
                free.push_back(ptr);
            }

            file.AddElement<int>(&Chunk::Seed);

            SetSeed();
            faces.reserve(1024);
            faces_transparent.reserve(512);
            //After rendering check if any chunks were modifiesd, if so, rebuild them.
            callbackId = cbh.GetList(CallbackType::PostRender).Add([&]()
                                                                   {
                                                                       for (auto chunk : meshed_chunks)
                                                                       {
                                                                           if (chunk->regen_mesh)
                                                                           {
                                                                               //I only use one vector for all chunks to reduce memory usage.
                                                                               faces.clear();
                                                                               faces_transparent.clear();
                                                                               //TODO: Limit the number of meshes to be generated, if there are performace issues.
                                                                               //TODO: Use worker threads if it takes too long,
                                                                               //but wait for the worker threads to finish before continuing.
                                                                               chunk->regen_mesh = false;
                                                                               chunk->GenFaces(faces,faces_transparent);
                                                                           }
                                                                       }
                                                                   });
        };

        ~ChunkManager()
        {
            for (auto chunk : chunks)
                delete chunk;
            cbh.GetList(CallbackType::PostRender).Remove(callbackId);
        }

        ChunkManager(const ChunkManager &) = delete;
        ChunkManager &operator=(const ChunkManager &) = delete;

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
            return {pos.x > 0 ? (pos.x / 16) : pos.x / 16 - 1, pos.y > 0 ? (pos.y / 16) : pos.y / 16 - 1};
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

            Chunk::NewSeed();
            file.Store();
            free = chunks;
            loaded.clear();
            meshed_chunks.clear();
            rendered.clear();
            LoadChunks({0, 0});
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
                if (!IsMeshed((*chunk)->GetPos(), pos))
                {
                    if (auto chunk2 = std::find(meshed_chunks.begin(), meshed_chunks.end(), *chunk); !IsRendered((*chunk)->GetPos(), pos) && chunk2 != meshed_chunks.end())
                    {
                        meshed_chunks.erase(chunk2);
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
            LastPlayerChunk=position;
            UnLoadChunks(position);
            LoadChunks(position);
        }

        /**
         * @brief This function decides which meshes need to be rebuilt after a block change.
         * 
         * @param changed_block The modified block.
         */
        void RegenerateMeshes(glm::ivec3 changed_block)
        {
            auto chunk_pos = GetChunkPos(changed_block.x, changed_block.z);
            auto chunk = GetChunk(chunk_pos);
            chunk->regen_mesh = true;
            auto local_pos = chunk->ToLocalCoords(changed_block);
            if (local_pos.x == 0)
            {
                GetChunk(chunk_pos - glm::ivec2(1, 0))->regen_mesh = true;
            }
            else if (local_pos.x == 15)
            {
                GetChunk(chunk_pos + glm::ivec2(1, 0))->regen_mesh = true;
            }
            if (local_pos.z == 0)
            {
                GetChunk(chunk_pos - glm::ivec2(0, 1))->regen_mesh = true;
            }
            else if (local_pos.z == 15)
            {
                GetChunk(chunk_pos + glm::ivec2(0, 1))->regen_mesh = true;
            }
        }
    };
}
