#include <Scene.hpp>
#include <glm/glm.hpp>
#include <Data.hpp>
#include <Voxel/Block.hpp>
#include <Shader.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Camera/Fplocked.hpp>
#include <Image/stb_image.h>
#include <Voxel/Chunk.hpp>
#include <Voxel/ConfigReader.hpp>

const double raydist = 8;

class Voxel_t final : public GL::Scene
{
    uint texid;
    GL::Shader cshader;
    glm::mat4 proj = glm::perspective(glm::radians(65.0f), (float)loader->GetWindow().GetWidth() / loader->GetWindow().GetHeigth(), 0.05f, 500.0f);

    GL::Camera3D camera;
    GL::Fplocked controller;
    GL::Voxel::Chunk *chunks;
    GL::Voxel::TexConfig blocks;

    glm::ivec2 GetChunk(int x, int z)
    {
        return {ceil((x + 1) / 16.0f) - 1, ceil((z + 1) / 16.0f) - 1};
    }

    uint &GetBlockAt(int x, int y, int z)
    {
        auto chunk = GetChunk(x, z);
        return chunks[(chunk.x + 8) * 16 + chunk.y + 8](x - 16 * chunk.x, y, z - 16 * chunk.y);
    }

    void RayCast()
    {
        auto ray_pos = camera.position;
        auto stepvec = camera.Forward() / 16.0 * raydist;
        for (int i = 0; i < 16; i++)
        {
            ray_pos += stepvec;
            int x = round(ray_pos.x);
            int y = round(ray_pos.y);
            int z = round(ray_pos.z);

            uint &block = GetBlockAt(x, y, z);
            if (block != GL::Voxel::Chunk::BAir && block != GL::Voxel::Chunk::BWater)
            {
                block = 0;
                auto chunk = GetChunk(x, z);
                chunks[(chunk.x + 8) * 16 + chunk.y + 8].GenFaces();
                return;
            }
        }
    }


    void Render()
    {
        controller.Update(loader->GetTimeInfo().RenderDeltaTime());

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, texid);

        cshader.Bind();
        cshader.SetUniformMat4f("u_MVP", proj * camera.ComputeMatrix());

        for (int i = 0; i < 16 * 16; i++)
        {
            chunks[i].DrawOpaque();
        }
        for (int i = 0; i < 16 * 16; i++)
        {
            chunks[i].DrawTransparent();
        }
        cshader.UnBind();
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
        if (glfwGetMouseButton(loader->GetWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
            RayCast();
    }

    void TexSetup();

public:
    Voxel_t(GL::SceneLoader *_loader) : Scene(_loader), cshader(ROOT_Directory + "/shader/Voxel/Chunk.vs", ROOT_Directory + "/shader/Voxel/Block.fs"),
                                        camera({0, 30, 0}), controller(&camera, loader->GetWindow(), 16), blocks(ROOT_Directory + "/res/Textures/Newblock.cfg")
    {
        RegisterFunc(std::bind(&Voxel_t::Render, this), GL::CallbackType::Render);

        cshader.Bind();
        cshader.SetUniform1i("u_Texture", 0);

        TexSetup();
        cshader.UnBind();

        GL::Voxel::Chunk::NewSeed();

        chunks = (GL::Voxel::Chunk *)malloc(16 * 16 * sizeof(GL::Voxel::Chunk));
        for (int x = 0; x < 16; x++)
        {
            for (int z = 0; z < 16; z++)
            {
                new (&chunks[x * 16 + z]) GL::Voxel::Chunk({x - 8, z - 8},blocks);
            }
        }
        loader->GetFlag("hide_menu") = 1;
    }
    ~Voxel_t()
    {
        RemoveFunctions();
        for (int i = 0; i < 16 * 16; i++)
            chunks[i].~Chunk();
        free(chunks);
    }
};

void Voxel_t::TexSetup()
{
    stbi_set_flip_vertically_on_load(1);
    int w, h, bpp;
    auto local_buffer = stbi_load((ROOT_Directory + "/res/Textures/Newblock.png").c_str(), &w, &h, &bpp, 4);

    glGenTextures(1, &texid);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texid);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, 64, 64, h / 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, local_buffer);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    if (local_buffer)
        stbi_image_free(local_buffer);

}

SCENE_LOAD_FUNC(Voxel_t)