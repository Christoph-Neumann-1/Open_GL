#include <Voxel/Chunk.hpp>
#include <Data.hpp>

namespace GL::Voxel
{
    int Chunk::Seed;

    Chunk::Face Chunk::GenFace(glm::ivec3 pos, FaceIndices type)
    {
        Face face;
        for (int i = 0; i < 6; i++)
        {
            auto vert = bvertices[i + type];
            face.vertices[i].tex = {(vert.tex.x + 0.5) / 64.0, (vert.tex.y + 0.5) / 64.0, config[lookup_cache[(BlockTypes)At(pos)]].faces[type / 6]};
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

    Chunk::Chunk(const TexConfig &cfg, CallbackList &cb, uint cbid) : render_thread(cb), callback_id(cbid), config(cfg)
    {

        lookup_cache = MakeBlockCache(config);

        va.Bind();

        buffer.Bind(GL_ARRAY_BUFFER);
        glVertexAttribPointer(0, 3, GL_FLOAT, false, 6 * sizeof(float), 0);
        glVertexAttribPointer(1, 3, GL_FLOAT, false, 6 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        va_transparent.Bind();

        buffer_transparent.Bind(GL_ARRAY_BUFFER);
        glVertexAttribPointer(0, 3, GL_FLOAT, false, 6 * sizeof(float), 0);
        glVertexAttribPointer(1, 3, GL_FLOAT, false, 6 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        Buffer::Unbind(GL_ARRAY_BUFFER);
        VertexArray::Unbind();
    }

    void Chunk::Load()
    {
        if (isactive)
            return;
        regen_mesh = true;
        isactive = true;
    }

    void Chunk::PreLoad(glm::ivec2 position)
    {
        chunk_offset = position;

        auto file = fopen((ROOT_Directory + "/res/world/" + std::to_string(position.x) + "__" + std::to_string(position.y)).c_str(), "r");
        if (!file)
            Generate();
        else
        {
            fread(&blocks, sizeof(blocks), 1, file);
            fclose(file);
        }
    }

    void Chunk::UnLoad()
    {
        auto file = fopen((ROOT_Directory + "/res/world/" + std::to_string(chunk_offset.x) + "__" + std::to_string(chunk_offset.y)).c_str(), "w");
        if (file)
        {
            fwrite(&blocks, sizeof(blocks), 1, file);
            fclose(file);
        }
        else
            perror("Unload Chunk");
        isactive = false;
    }

    void Chunk::GenFaces()
    {
        auto is_tp=[&](int x, int y, int z)->bool{return IsTransparent((BlockTypes)blocks[x][y][z]);};

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
                    //If the block is on the edge of the chunk, it is treated differently. Right now this means the face is always drawn.
                    bool special = false;

                    if (!is_tp(x, y, z))
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
                                if (is_tp(x - 1, y, z))
                                    faces.push_back(GenFace({x, y, z}, Left));
                            }
                            if (x != 15)
                            {
                                if (is_tp(x + 1, y, z))
                                    faces.push_back(GenFace({x, y, z}, Right));
                            }
                            if (y != 0)
                            {
                                if (is_tp(x, y - 1, z))
                                    faces.push_back(GenFace({x, y, z}, Bottom));
                            }
                            if (y != 63)
                            {
                                if (is_tp(x, y + 1, z))
                                    faces.push_back(GenFace({x, y, z}, Top));
                            }
                            if (z != 0)
                            {
                                if (is_tp(x, y, z - 1))
                                    faces.push_back(GenFace({x, y, z}, Back));
                            }
                            if (z != 15)
                            {
                                if (is_tp(x, y, z + 1))
                                    faces.push_back(GenFace({x, y, z}, Front));
                            }
                        }
                        else
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

        //This function will probably be called in a seperate thread but Opengl only works in the render thread.
        renderid = render_thread.Add([&]()
                                     {
                                         buffer.Bind(GL_ARRAY_BUFFER);
                                         glBufferData(GL_ARRAY_BUFFER, faces.size() * sizeof(Face), &faces[0], GL_STATIC_DRAW);
                                         buffer_transparent.Bind(GL_ARRAY_BUFFER);
                                         glBufferData(GL_ARRAY_BUFFER, faces_transparent.size() * sizeof(Face), &faces_transparent[0], GL_STATIC_DRAW);
                                         Buffer::Unbind(GL_ARRAY_BUFFER);
                                         render_thread.Remove(renderid);
                                     },
                                     callback_id);
    }
}