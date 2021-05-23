#pragma once
#include <Voxel/Block.hpp>

#include <Noise/FastNoiseLite.h>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <array>
#include <string.h>
#include <Voxel/ConfigReader.hpp>

using std::array;
using std::vector;

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
            array<Vertex, 6> vertices;
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
        vector<Face> faces;
        vector<Face> faces_transparent;
        array<array<array<uint, 16>, 64>, 16> blocks;
        uint buffer, buffer_transparent, va, va_transparent;
        const TexConfig &config;

        Face GenFace(glm::ivec3 pos, FaceIndices type)
        {
            Face face;
            for (int i = 0; i < 6; i++)
            {
                auto vert = bvertices[i + type];
                face.vertices[i].tex = {(vert.tex.x + 0.5) / 64.0, (vert.tex.y + 0.5) / 64.0, config[lookup((BlockTypes)At(pos))].faces[type / 6]};
                face.vertices[i].pos = {vert.pos.x + 16 * chunk_offset.x + pos.x, vert.pos.y + pos.y, vert.pos.z + 16 * chunk_offset.y + pos.z};
            };
            return face;
        }

    public:
        void UpdateCache()
        {

            lookup(BGrass) = config.FindByName("Grass");
            lookup(BDirt) = config.FindByName("Dirt");
            lookup(BStone) = config.FindByName("Stone");
            lookup(BWater) = config.FindByName("Water");
            lookup(BSand) = config.FindByName("Sand");
            lookup(BWood) = config.FindByName("Wood");
        };

        void GenFaces()
        {
            faces.clear();
            faces_transparent.clear();
            for (int x = 0; x < 16; x++)
            {
                for (int y = 0; y < 64; y++)
                {
                    for (int z = 0; z < 16; z++)
                    {
                        if (blocks[x][y][z] == BAir)
                            continue;
                        bool special = false;

                        if (!IsTransparent(x, y, z))
                        {
                            if (z == 15)
                            {
                                faces.push_back(GenFace({x, y, z}, Front));
                                special = true;
                            }
                            else if (z == 0)
                            {
                                faces.push_back(GenFace({x, y, z}, Back));
                                special = true;
                            }

                            if (x == 0)
                            {
                                faces.push_back(GenFace({x, y, z}, Left));
                                special = true;
                            }
                            else if (x == 15)
                            {
                                faces.push_back(GenFace({x, y, z}, Right));
                                special = true;
                            }

                            if (y == 0)
                            {
                                faces.push_back(GenFace({x, y, z}, Bottom));
                                special = true;
                            }
                            else if (y == 63)
                            {
                                faces.push_back(GenFace({x, y, z}, Top));
                                special = true;
                            }

                            if (special)
                            {
                                if (x != 0)
                                {
                                    if (IsTransparent(x - 1, y, z))
                                        faces.push_back(GenFace({x, y, z}, Left));
                                }
                                if (x != 15)
                                {
                                    if (IsTransparent(x + 1, y, z))
                                        faces.push_back(GenFace({x, y, z}, Right));
                                }
                                if (y != 0)
                                {
                                    if (IsTransparent(x, y - 1, z))
                                        faces.push_back(GenFace({x, y, z}, Bottom));
                                }
                                if (y != 63)
                                {
                                    if (IsTransparent(x, y + 1, z))
                                        faces.push_back(GenFace({x, y, z}, Top));
                                }
                                if (z != 0)
                                {
                                    if (IsTransparent(x, y, z - 1))
                                        faces.push_back(GenFace({x, y, z}, Back));
                                }
                                if (z != 15)
                                {
                                    if (IsTransparent(x, y, z + 1))
                                        faces.push_back(GenFace({x, y, z}, Front));
                                }
                            }
                            else
                            {
                                if (IsTransparent(x - 1, y, z))
                                    faces.push_back(GenFace({x, y, z}, Left));
                                if (IsTransparent(x + 1, y, z))
                                    faces.push_back(GenFace({x, y, z}, Right));
                                if (IsTransparent(x, y - 1, z))
                                    faces.push_back(GenFace({x, y, z}, Bottom));
                                if (IsTransparent(x, y + 1, z))
                                    faces.push_back(GenFace({x, y, z}, Top));
                                if (IsTransparent(x, y, z - 1))
                                    faces.push_back(GenFace({x, y, z}, Back));
                                if (IsTransparent(x, y, z + 1))
                                    faces.push_back(GenFace({x, y, z}, Front));
                            }
                        }
                        else
                        {
                            if (blocks[x][y][z] == BWater)
                            {
                                if (y == sealevel)
                                {
                                    faces_transparent.push_back(GenFace({x, y, z}, Top));
                                }
                                continue;
                            }
                            faces_transparent.push_back(GenFace({x, y, z}, Left));
                            faces_transparent.push_back(GenFace({x, y, z}, Right));
                            faces_transparent.push_back(GenFace({x, y, z}, Bottom));
                            faces_transparent.push_back(GenFace({x, y, z}, Top));
                            faces_transparent.push_back(GenFace({x, y, z}, Back));
                            faces_transparent.push_back(GenFace({x, y, z}, Front));

                            if (z == 15)
                            {
                                faces_transparent.push_back(GenFace({x, y, z}, Front));
                                special = true;
                            }
                            else if (z == 0)
                            {
                                faces_transparent.push_back(GenFace({x, y, z}, Back));
                                special = true;
                            }

                            if (x == 0)
                            {
                                faces_transparent.push_back(GenFace({x, y, z}, Left));
                                special = true;
                            }
                            else if (x == 15)
                            {
                                faces_transparent.push_back(GenFace({x, y, z}, Right));
                                special = true;
                            }

                            if (y == 0)
                            {
                                faces_transparent.push_back(GenFace({x, y, z}, Bottom));
                                special = true;
                            }
                            else if (y == 63)
                            {
                                faces_transparent.push_back(GenFace({x, y, z}, Top));
                                special = true;
                            }

                            if (special)
                            {
                                if (x != 0)
                                {
                                    if (blocks[x - 1][y][z] == 0)
                                        faces_transparent.push_back(GenFace({x, y, z}, Left));
                                }
                                if (x != 15)
                                {
                                    if (blocks[x + 1][y][z] == 0)
                                        faces_transparent.push_back(GenFace({x, y, z}, Right));
                                }
                                if (y != 0)
                                {
                                    if (blocks[x][y - 1][z] == 0)
                                        faces_transparent.push_back(GenFace({x, y, z}, Bottom));
                                }
                                if (y != 63)
                                {
                                    if (blocks[x][y + 1][z] == 0)
                                        faces_transparent.push_back(GenFace({x, y, z}, Top));
                                }
                                if (z != 0)
                                {
                                    if (blocks[x][y][z - 1] == 0)
                                        faces_transparent.push_back(GenFace({x, y, z}, Back));
                                }
                                if (z != 15)
                                {
                                    if (blocks[x][y][z + 1] == 0)
                                        faces_transparent.push_back(GenFace({x, y, z}, Front));
                                }
                            }
                            else
                            {
                                if (blocks[x - 1][y][z] == 0)
                                    faces_transparent.push_back(GenFace({x, y, z}, Left));
                                if (blocks[x + 1][y][z] == 0)
                                    faces_transparent.push_back(GenFace({x, y, z}, Right));
                                if (blocks[x][y - 1][z] == 0)
                                    faces_transparent.push_back(GenFace({x, y, z}, Bottom));
                                if (blocks[x][y + 1][z] == 0)
                                    faces_transparent.push_back(GenFace({x, y, z}, Top));
                                if (blocks[x][y - 1][z] == 0)
                                    faces_transparent.push_back(GenFace({x, y, z}, Back));
                                if (blocks[x][y + 1][z] == 0)
                                    faces_transparent.push_back(GenFace({x, y, z}, Front));
                            }
                        }
                    }
                }
            }
            glBindBuffer(GL_ARRAY_BUFFER, buffer);
            glBufferData(GL_ARRAY_BUFFER, faces.size() * sizeof(Face), &faces[0], GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ARRAY_BUFFER, buffer_transparent);
            glBufferData(GL_ARRAY_BUFFER, faces_transparent.size() * sizeof(Face), &faces_transparent[0], GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        void GenTree(int x, int y, int z)
        {
            int heigth = y + 4 + rand() % 4;
            for (int i = y; i <= heigth; i++)
            {
                blocks[x][i][z] = BWood;
            }
        }

        Chunk(glm::ivec2 position, const TexConfig &cfg) : chunk_offset(position), config(cfg)
        {
            GL::Logger logger;
            FastNoiseLite noise;
            noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
            noise.SetFractalType(FastNoiseLite::FractalType::FractalType_DomainWarpIndependent);
            noise.SetFractalOctaves(3);
            noise.SetFrequency(0.009);
            noise.SetSeed(Seed);

            UpdateCache();

            memset(&blocks, 0, sizeof(blocks));
            for (int x = 0; x < 16; x++)
            {
                for (int z = 0; z < 16; z++)
                {
                    double val = noise.GetNoise((float)x - 8 + 16 * position.x, (float)z - 8 + 16 * position.y);
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

            glGenVertexArrays(2, &va);
            glGenBuffers(2, &buffer);

            glBindVertexArray(va);

            glBindBuffer(GL_ARRAY_BUFFER, buffer);
            glVertexAttribPointer(0, 3, GL_FLOAT, false, 6 * sizeof(float), 0);
            glVertexAttribPointer(1, 3, GL_FLOAT, false, 6 * sizeof(float), (void *)(3 * sizeof(float)));
            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);

            glBindVertexArray(va_transparent);

            glBindBuffer(GL_ARRAY_BUFFER, buffer_transparent);
            glVertexAttribPointer(0, 3, GL_FLOAT, false, 6 * sizeof(float), 0);
            glVertexAttribPointer(1, 3, GL_FLOAT, false, 6 * sizeof(float), (void *)(3 * sizeof(float)));
            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);

            GenFaces();
        }

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