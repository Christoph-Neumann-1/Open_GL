#include <Scene.hpp>
#include <glm/glm.hpp>
#include <Data.hpp>
#include <Voxel/Block.hpp>
#include <Shader.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Camera/Fplocked.hpp>
#include <Image/stb_image.h>
#include <Voxel/Chunk.hpp>

class Voxel_t final : public GL::Scene
{
    uint texid;
    GL::Shader cshader;
    glm::mat4 proj = glm::perspective(glm::radians(60.0f), (float)loader->GetWindow().GetWidth() / loader->GetWindow().GetHeigth(), 0.05f, 500.0f);

    GL::Camera3D camera;
    GL::Fplocked controller;
    GL::Voxel::Chunk *chunks;

    void Render()
    {
        glDisable(GL_MULTISAMPLE);
        controller.Update(loader->GetTimeInfo().RenderDeltaTime());

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texid);

        cshader.Bind();
        cshader.SetUniformMat4f("u_MVP", proj * camera.ComputeMatrix());

        for (int i = 0; i < 16 * 16; i++)
        {
            chunks[i].Draw();
        }

        cshader.UnBind();
        glBindTexture(GL_TEXTURE_2D, 0);
        glEnable(GL_MULTISAMPLE);
    }

    void TexSetup();

public:
    Voxel_t(GL::SceneLoader *_loader) : Scene(_loader), cshader(ROOT_Directory + "/shader/Voxel/Chunk.vs", ROOT_Directory + "/shader/Voxel/Block.fs"),
                                        camera({0, 30, 0}), controller(&camera, loader->GetWindow(), 16)
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
                new (&chunks[x * 16 + z]) GL::Voxel::Chunk({x - 8, z - 8});
            }
        }
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
    auto local_buffer = stbi_load((ROOT_Directory + "/res/Textures/Block.png").c_str(), &w, &h, &bpp, 3);

    glGenTextures(1, &texid);
    glBindTexture(GL_TEXTURE_2D, texid);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, local_buffer);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);

    if (local_buffer)
        stbi_image_free(local_buffer);

    cshader.SetUniform1f("tex_size", w);
}

SCENE_LOAD_FUNC(Voxel_t)