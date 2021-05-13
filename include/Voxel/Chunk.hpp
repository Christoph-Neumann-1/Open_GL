#pragma once
#include <Voxel/Block.hpp>

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <array>

using std::array;
using std::vector;
namespace GL::Voxel
{
    class Chunk
    {
        struct Face
        {
            struct Vertex
            {
                glm::vec3 pos;
                glm::vec2 tex;
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

        glm::ivec2 chunk_offset;
        vector<Face> faces;
        array<array<array<float, 16>, 64>, 16> blocks;
        uint buffer, va;

        Face GenFace(glm::ivec3 pos, FaceIndices type)
        {
            Face face;
            for (int i = 0; i < 6; i++)
            {
                auto vert = bvertices[i + type];
                face.vertices[i].tex = {vert.tex.x + (blocks[pos.x][pos.y][pos.z] - 1) * 192, vert.tex.y};
                face.vertices[i].pos = {vert.pos.x - 8 + 16 * chunk_offset.x + pos.x, vert.pos.y + pos.y, vert.pos.z - 8 + 16 * chunk_offset.y + pos.z};
            };
            return face;
        }

        void GenFaces()
        {
            for (int x = 0; x < 16; x++)
            {
                for (int y = 0; y < 64; y++)
                {
                    for (int z = 0; z < 16; z++)
                    {
                        if (blocks[x][y][z] == 0)
                            continue;
                        bool special = false;
#pragma region CheckEdge
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
                        else if (y == 15)
                        {
                            faces.push_back(GenFace({x, y, z}, Top));
                            special = true;
                        }
#pragma endregion

                        if (special)
                        {
                            if (x != 0)
                            {
                                if (blocks[x - 1][y][z] == 0)
                                    faces.push_back(GenFace({x, y, z}, Left));
                            }
                            if (x != 15)
                            {
                                if (blocks[x + 1][y][z] == 0)
                                    faces.push_back(GenFace({x, y, z}, Right));
                            }
                            if (y != 0)
                            {
                                if (blocks[x][y - 1][z] == 0)
                                    faces.push_back(GenFace({x, y, z}, Bottom));
                            }
                            if (y != 63)
                            {
                                if (blocks[x][y + 1][z] == 0)
                                    faces.push_back(GenFace({x, y, z}, Top));
                            }
                            if (z != 0)
                            {
                                if (blocks[x][y][z - 1] == 0)
                                    faces.push_back(GenFace({x, y, z}, Back));
                            }
                            if (z != 15)
                            {
                                if (blocks[x][y][z + 1] == 0)
                                    faces.push_back(GenFace({x, y, z}, Front));
                            }
                        }
                        else
                        {
                            if (blocks[x - 1][y][z] == 0)
                                faces.push_back(GenFace({x, y, z}, Left));
                            if (blocks[x + 1][y][z] == 0)
                                faces.push_back(GenFace({x, y, z}, Right));
                            if (blocks[x][y - 1][z] == 0)
                                faces.push_back(GenFace({x, y, z}, Bottom));
                            if (blocks[x][y + 1][z] == 0)
                                faces.push_back(GenFace({x, y, z}, Top));
                            if (blocks[x][y][z - 1] == 0)
                                faces.push_back(GenFace({x, y, z}, Back));
                            if (blocks[x][y][z + 1] == 0)
                                faces.push_back(GenFace({x, y, z}, Front));
                        }
                    }
                }
            }
            glBindBuffer(GL_ARRAY_BUFFER, buffer);
            glBufferData(GL_ARRAY_BUFFER, faces.size() * sizeof(Face), &faces[0], GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

    public:
        Chunk(glm::ivec2 position) : chunk_offset(position)
        {
            for (int x = 0; x < 16; x++)
            {
                for (int y = 0; y < 16; y++)
                {
                    for (int z = 0; z < 16; z++)
                    {
                        blocks[x][y][z] = 1;
                    }
                }
            }
            for (int x = 0; x < 16; x++)
            {
                for (int y = 16; y < 64; y++)
                {
                    for (int z = 0; z < 16; z++)
                    {
                        blocks[x][y][z] = 0;
                    }
                }
            }

            glGenVertexArrays(1, &va);
            glGenBuffers(1, &buffer);

            glBindVertexArray(va);

            glBindBuffer(GL_ARRAY_BUFFER, buffer);
            glVertexAttribPointer(0, 3, GL_FLOAT, false, 5 * sizeof(float), 0);
            glVertexAttribPointer(1, 2, GL_FLOAT, false, 5 * sizeof(float), (void *)(3 * sizeof(float)));
            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);

            GenFaces();
        }

        ~Chunk()
        {
            glDeleteBuffers(1, &buffer);
            glDeleteVertexArrays(1, &va);
        }

        void Draw()
        {
            glBindVertexArray(va);
            glDrawArrays(GL_TRIANGLES, 0, 6 * faces.size());
            glBindVertexArray(0);
        }
    };
}