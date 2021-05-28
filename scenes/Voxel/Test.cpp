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
#include <Voxel/ChunkManager.hpp>
const double raydist = 8;

class Voxel_t final : public GL::Scene
{
    uint texid;
    GL::Shader cshader;
    glm::mat4 proj = glm::perspective(glm::radians(65.0f), (float)loader->GetWindow().GetWidth() / loader->GetWindow().GetHeigth(), 0.05f, 400.0f);

    GL::Camera3D camera;
    GL::Fplocked controller;
    GL::Voxel::TexConfig blocks;
    GL::Voxel::ChunkManager chunks;
    bool r_pressed = false;

    void StorePlayerData()
    {
        auto file = fopen((ROOT_Directory + "/res/world/PLAYER").c_str(), "w");
        if (!file)
        {
            perror("Store Player Data");
            return;
        }
        fwrite(&camera.position, sizeof(camera.position), 1, file);
        fwrite(&controller.pitch, 2 * sizeof(double), 1, file);
        fclose(file);
    }

    void LoadPlayerData()
    {
        auto file = fopen((ROOT_Directory + "/res/world/PLAYER").c_str(), "r");
        if (file)
        {
            fread(&camera.position, sizeof(camera.position), 1, file);
            fread(&controller.pitch, 2 * sizeof(double), 1, file);

            fclose(file);
        }
    }

    void RayCast()
    {
        auto ray_pos = camera.position;
        auto stepvec = camera.Forward() / 32.0 * raydist;
        for (int i = 0; i < 32; i++)
        {
            ray_pos += stepvec;
            int x = round(ray_pos.x);
            int y = round(ray_pos.y);
            int z = round(ray_pos.z);
            y = std::clamp(y, 0, 63);

            uint *block = chunks.GetBlockAt(x, y, z);
            if (*block != GL::Voxel::Chunk::BAir && *block != GL::Voxel::Chunk::BWater)
            {
                *block = 0;
                auto chunk = chunks.GetChunkPos(x, z);
                chunks.GetChunk(chunk)->regen_mesh = true;
                return;
            }
        }
    }

    void Render()
    {
        glm::ivec2 lastpos = {round(camera.position.x), round(camera.position.z)};
        controller.Update(loader->GetTimeInfo().RenderDeltaTime());

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, texid);

        cshader.Bind();
        cshader.SetUniformMat4f("u_MVP", proj * camera.ComputeMatrix());

        chunks.DrawChunks();
        cshader.UnBind();
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
        if (glfwGetMouseButton(loader->GetWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
            RayCast();
        if (glfwGetKey(loader->GetWindow(), GLFW_KEY_R))
        {
            if (!r_pressed)
            {
                chunks.Regenerate();
                r_pressed = true;
            }
        }
        else if (r_pressed)
        {
            r_pressed = false;
        }
        if (chunks.HasCrossedChunk(lastpos, {round(camera.position.x), round(camera.position.z)}))
            chunks.MoveChunk(chunks.GetChunkPos({round(camera.position.x), round(camera.position.z)}));
    }

    void TexSetup();

public:
    Voxel_t(GL::SceneLoader *_loader) : Scene(_loader), cshader(ROOT_Directory + "/shader/Voxel/Chunk.vs", ROOT_Directory + "/shader/Voxel/Block.fs"),
                                        camera({0, 30, 0}), controller(&camera, loader->GetWindow(), 22), blocks(ROOT_Directory + "/res/Textures/Newblock.cfg"),
                                        chunks({0, 0}, blocks, loader->GetCallback())
    {
        RegisterFunc(GL::CallbackType::Render, &Voxel_t::Render, this);

        cshader.Bind();
        cshader.SetUniform1i("u_Texture", 0);

        TexSetup();
        cshader.UnBind();

        loader->GetFlag("hide_menu") = 1;

        LoadPlayerData();
    }
    ~Voxel_t()
    {
        RemoveFunctions();

        StorePlayerData();
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