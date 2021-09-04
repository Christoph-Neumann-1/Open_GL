/**
 * @file Chunk.cpp
 * @author Christoph Neumann
 * @copyright Copyright © 2021 Christoph Neumann - MIT License
 */
#include <Voxel/Chunk.hpp>
#include <Data.hpp>
#include <Time.hpp>

namespace GL::Voxel
{
    int Chunk::Seed;

    Chunk::Face Chunk::GenFace(glm::ivec3 pos, FaceIndices type)
    {
        Face face;
        for (int i = 0; i < 6; i++)
        {
            auto vert = bvertices[i + type];
            face.vertices[i].tex = {(vert.tex.x + 0.5) / 64.0, (vert.tex.y + 0.5) / 64.0, config[lookup_cache[At(pos)]].faces[type / 6]};
            face.vertices[i].pos = {vert.pos.x + 16 * chunk_offset.x + pos.x, vert.pos.y + pos.y, vert.pos.z + 16 * chunk_offset.y + pos.z};
        };
        return face;
    }

    void Chunk::Generate()
    {
        FastNoiseLite noise;
        noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
        noise.SetFractalType(FastNoiseLite::FractalType::FractalType_DomainWarpIndependent);
        noise.SetFractalOctaves(3);
        noise.SetFrequency(0.009);
        noise.SetSeed(Seed);

        memset(&blocks, 0, sizeof(blocks));

        for (int x = 0; x < 16; x++)
        {
            for (int z = 0; z < 16; z++)
            {
                double val = noise.GetNoise((float)x - 8 + 16 * chunk_offset.x, (float)z - 8 + 16 * chunk_offset.y);
                int heigth = std::clamp((int)((val + 1) * 18), 1, 63);
                blocks[x][heigth][z] = heigth >= sealevel ? BGrass : BSand;
                for (int y = 0; y < heigth - 3; y++)
                {
                    blocks[x][y][z] = BStone;
                }
                for (int y = heigth - 3 > 0 ? heigth - 3 : 0; y < heigth; y++)
                {
                    blocks[x][y][z] = BDirt;
                }

                if (heigth >= sealevel && rand() % 200 == 1 && heigth < 48)
                {
                    GenTree(x, heigth + 1, z);
                }
                if (heigth < sealevel)
                {
                    for (int i = heigth + 1; i <= sealevel; i++)
                        blocks[x][i][z] = BWater;
                }
            }
        }
    }

    Chunk::Chunk(const TexConfig &cfg, const std::function<Chunk *(int, int)> GetOtherChunk_)
        : config(cfg), GetOtherChunk(GetOtherChunk_)
    {

        lookup_cache = MakeBlockCache(config);

        va.Bind();

        buffer.Bind(GL_ARRAY_BUFFER);
        VertexBufferLayout layout;
        layout.stride = 6 * sizeof(float);
        layout.Push(GL_FLOAT, 3, 0);
        layout.Push(GL_FLOAT, 3, sizeof(glm::vec3));
        layout.AddToVertexArray(va);

        layout.BindAndAddToVertexArray(va_transparent, buffer_transparent);
    }

    void Chunk::Load()
    {
        if (isactive)
            return;
        //Prevents drawing in the first frame after loading
        nFaces = 0;
        nFacesTp = 0;
        regen_mesh = true;
        isactive = true;
    }

    void Chunk::PreLoad(glm::ivec2 position)
    {
        chunk_offset = position;

        auto file = fopen(("res/world/" + std::to_string(position.x) + "__" + std::to_string(position.y)).c_str(), "r");
        if (!file)
            Generate();
        else
        {
            fread(&blocks, sizeof(blocks), 1, file);
            fclose(file);
        }
        isactive = false;
    }

    void Chunk::UnLoad()
    {
        //TODO: compress data
        auto file = fopen(("res/world/" + std::to_string(chunk_offset.x) + "__" + std::to_string(chunk_offset.y)).c_str(), "w");
        if (file)
        {
            fwrite(&blocks, sizeof(blocks), 1, file);
            fclose(file);
        }
        else
            perror("Unload Chunk");
    }

    //TODO: find a way to optimize this function even in debug mode
    //TODO: optimize the whole function
    //TODO: greedy meshing
    //TODO: fast and slow meshing
    void Chunk::GenFaces(std::vector<Face> &faces, std::vector<Face> &faces_transparent)
    {
        Chunk *left_chunk, *right_chunk, *front_chunk, *back_chunk;
        left_chunk = GetOtherChunk(chunk_offset.x - 1, chunk_offset.y);
        right_chunk = GetOtherChunk(chunk_offset.x + 1, chunk_offset.y);
        front_chunk = GetOtherChunk(chunk_offset.x, chunk_offset.y - 1);
        back_chunk = GetOtherChunk(chunk_offset.x, chunk_offset.y + 1);

        auto is_tp = [&](int x, int y, int z) -> bool
        { return IsTransparent(blocks[x][y][z]); };

        for (int x = 0; x < 16; x++)
        {
            for (int y = 0; y < 64; y++)
            {
                for (int z = 0; z < 16; z++)
                {
                    if (blocks[x][y][z] == BAir)
                        continue;
                    if (!is_tp(x, y, z)) [[likely]]
                    {
                        //If the block is on the edge of the chunk, it is treated differently. Right now this means the face is always drawn.

                        if (z == 0 || z == 15 || y == 0 || y == 63 || x == 0 || x == 15) [[unlikely]]
                        {
                            if (x != 0)
                            {
                                if (is_tp(x - 1, y, z))
                                    faces.push_back(GenFace({x, y, z}, Left));
                            }
                            else
                            {
                                if (IsTransparent(left_chunk->At(15, y, z)))
                                    faces.push_back(GenFace({x, y, z}, Left));
                            }
                            if (x != 15)
                            {
                                if (is_tp(x + 1, y, z))
                                    faces.push_back(GenFace({x, y, z}, Right));
                            }
                            else
                            {
                                if (IsTransparent(right_chunk->At(0, y, z)))
                                    faces.push_back(GenFace({x, y, z}, Right));
                            }
                            if (y != 0)
                            {
                                if (is_tp(x, y - 1, z))
                                    faces.push_back(GenFace({x, y, z}, Bottom));
                            }
                            else
                            {
                                faces.push_back(GenFace({x, y, z}, Bottom));
                            }
                            if (y != 63)
                            {
                                if (is_tp(x, y + 1, z))
                                    faces.push_back(GenFace({x, y, z}, Top));
                            }
                            else
                            {
                                faces.push_back(GenFace({x, y, z}, Top));
                            }
                            if (z != 0)
                            {
                                if (is_tp(x, y, z - 1))
                                    faces.push_back(GenFace({x, y, z}, Back));
                            }
                            else
                            {
                                if (IsTransparent(front_chunk->At(x, y, 15)))
                                    faces.push_back(GenFace({x, y, z}, Back));
                            }
                            if (z != 15)
                            {
                                if (is_tp(x, y, z + 1))
                                    faces.push_back(GenFace({x, y, z}, Front));
                            }
                            else
                            {
                                if (IsTransparent(back_chunk->At(x, y, 0)))
                                    faces.push_back(GenFace({x, y, z}, Front));
                            }
                        }
                        else [[likely]]
                        {
                            if (is_tp(x - 1, y, z))
                                faces.push_back(GenFace({x, y, z}, Left));
                            if (is_tp(x + 1, y, z))
                                faces.push_back(GenFace({x, y, z}, Right));
                            if (is_tp(x, y - 1, z))
                                faces.push_back(GenFace({x, y, z}, Bottom));
                            if (is_tp(x, y + 1, z))
                                faces.push_back(GenFace({x, y, z}, Top));
                            if (is_tp(x, y, z - 1))
                                faces.push_back(GenFace({x, y, z}, Back));
                            if (is_tp(x, y, z + 1))
                                faces.push_back(GenFace({x, y, z}, Front));
                        }
                    }
                    else
                    {
                        if (blocks[x][y][z] == BWater)
                        {
                            if (blocks[x][std::clamp(y + 1, 0, 63)][z] != BWater)
                                faces_transparent.push_back(GenFace({x, y, z}, Top));
                            continue;
                        }
                        faces_transparent.push_back(GenFace({x, y, z}, Left));
                        faces_transparent.push_back(GenFace({x, y, z}, Right));
                        faces_transparent.push_back(GenFace({x, y, z}, Bottom));
                        faces_transparent.push_back(GenFace({x, y, z}, Top));
                        faces_transparent.push_back(GenFace({x, y, z}, Back));
                        faces_transparent.push_back(GenFace({x, y, z}, Front));
                    }
                }
            }
        }
        nFacesTp = faces_transparent.size();
        nFaces = faces.size();
        buffer.Bind(GL_ARRAY_BUFFER);
        glBufferData(GL_ARRAY_BUFFER, faces.size() * sizeof(Face), &faces[0], GL_STATIC_DRAW);
        buffer_transparent.Bind(GL_ARRAY_BUFFER);
        glBufferData(GL_ARRAY_BUFFER, faces_transparent.size() * sizeof(Face), &faces_transparent[0], GL_STATIC_DRAW);
        Buffer::Unbind(GL_ARRAY_BUFFER);
    }
}