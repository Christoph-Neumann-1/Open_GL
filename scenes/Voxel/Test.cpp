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
const float bps = 4;
const double raysteps = 32;
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
    float break_cooldown = 0;
    float place_cooldown = 0;

    float crosshair[8]{0.005, 0.005, -0.005, 0.005, -0.005, -0.005, 0.005, -0.005};
    uint vbo, ibo, vao;
    GL::Shader shader;

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

    void Mine()
    {
        if (!(break_cooldown > 0))
        {
            auto ray_pos = camera.position;
            auto stepvec = camera.Forward() / raysteps * raydist;
            for (int i = 0; i < raysteps; i++)
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
                    break_cooldown = 1 / bps;
                    return;
                }
            }
        }
    }

    void Place()
    {
        if (!(place_cooldown > 0))
        {
            auto ray_pos = camera.position;
            auto stepvec = camera.Forward() / raysteps * raydist;
            for (int i = 0; i < raysteps; i++)
            {
                ray_pos += stepvec;
                int x = round(ray_pos.x);
                int y = round(ray_pos.y);
                int z = round(ray_pos.z);
                y = std::clamp(y, 0, 63);

                uint *block = chunks.GetBlockAt(x, y, z);
                if (*block != GL::Voxel::Chunk::BAir && *block != GL::Voxel::Chunk::BWater)
                {
                    auto new_block = glm::vec3(x, y, z) - (glm::vec3)glm::normalize(stepvec);
                    int nx = roundf(new_block.x);
                    int ny = std::clamp((int)roundf(new_block.y), 0, 63);
                    int nz = roundf(new_block.z);
                    *chunks.GetBlockAt(nx, ny, nz) = GL::Voxel::Chunk::BStone;

                    auto chunk = chunks.GetChunkPos(x, z);
                    chunks.GetChunk(chunk)->regen_mesh = true;
                    place_cooldown = 1 / bps;
                    return;
                }
            }
        }
    }

    void Render()
    {
        auto dt = loader->GetTimeInfo().RenderDeltaTime();
        glm::ivec2 lastpos = {round(camera.position.x), round(camera.position.z)};
        controller.Update(dt);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, texid);

        cshader.Bind();
        cshader.SetUniformMat4f("u_MVP", proj * camera.ComputeMatrix());

        chunks.DrawChunks();

        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

        shader.Bind();
        glBindVertexArray(vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        shader.UnBind();

        if (glfwGetMouseButton(loader->GetWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
            Mine();
        else if (glfwGetMouseButton(loader->GetWindow(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
            Place();
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

        break_cooldown = break_cooldown - dt * (break_cooldown > 0);
        place_cooldown = place_cooldown - dt * (place_cooldown > 0);
    }

    void TexSetup();

public:
    Voxel_t(GL::SceneLoader *_loader) : Scene(_loader), cshader(ROOT_Directory + "/shader/Voxel/Chunk.vs", ROOT_Directory + "/shader/Voxel/Block.fs"),
                                        camera({0, 30, 0}), controller(&camera, loader->GetWindow(), 22), blocks(ROOT_Directory + "/res/Textures/Newblock.cfg"),
                                        chunks({0, 0}, blocks, loader->GetCallback()), shader(ROOT_Directory + "/shader/Default.vs", ROOT_Directory + "/shader/Default.fs")
    {
        RegisterFunc(GL::CallbackType::Render, &Voxel_t::Render, this);

        cshader.Bind();
        cshader.SetUniform1i("u_Texture", 0);

        TexSetup();
        cshader.UnBind();

        loader->GetFlag("hide_menu") = 1;

        LoadPlayerData();

        glGenBuffers(2, &vbo);
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glVertexAttribPointer(0, 2, GL_FLOAT, 0, 2 * sizeof(float), 0);
        glEnableVertexAttribArray(0);
        glBufferData(GL_ARRAY_BUFFER, sizeof(crosshair), crosshair, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        uint indices[]{
            0, 1, 2,
            2, 3, 0};
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        shader.Bind();
        glm::mat4 mat;
        auto window = loader->GetWindow();
        if (window.GetWidth() > window.GetHeigth())
        {
            mat = glm::ortho(-(float)window.GetWidth() / window.GetHeigth(), (float)window.GetWidth() / window.GetHeigth(), -1.0f, 1.0f);
        }
        else
        {
            mat = glm::ortho(-1.0f, 1.0f, -(float)window.GetHeigth() / window.GetWidth(), (float)window.GetHeigth() / window.GetWidth());
        }
        shader.SetUniformMat4f("u_MVP", mat);
        shader.SetUniform4f("u_Color", 1, 1, 1, 1);
        shader.UnBind();
    }
    ~Voxel_t()
    {
        RemoveFunctions();
        glDeleteBuffers(2, &vbo);
        glDeleteVertexArrays(1, &vao);
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