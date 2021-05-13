#include <Scene.hpp>
#include <glm/glm.hpp>
#include <Data.hpp>
#include <Voxel/Block.hpp>
#include <Shader.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Camera/Fplocked.hpp>
#include <Image/stb_image.h>
#include <Voxel/Chunk.hpp>

const float offsets[4 * 2]{
    -1, 0, 0, 0,
    1, 0.5, 0.9, 1};

class Voxel_t final : public GL::Scene
{
    uint va, vb, instbuff;
    uint texid;
    GL::Shader shader;
    GL::Shader cshader;
    glm::mat4 proj = glm::perspective(glm::radians(60.0f), (float)loader->GetWindow().GetWidth() / loader->GetWindow().GetHeigth(), 0.05f, 500.0f);

    GL::Camera3D camera;
    GL::Fplocked controller;
    GL::Voxel::Chunk *c;

    void Render()
    {
        glDisable(GL_MULTISAMPLE);
        shader.Bind();
        controller.Update(loader->GetTimeInfo().RenderDeltaTime());
        shader.SetUniformMat4f("u_MVP", proj * camera.ComputeMatrix());
        glBindVertexArray(va);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texid);

        glDrawArraysInstanced(GL_TRIANGLES, 0, 36, 2);

        glBindVertexArray(0);

        cshader.Bind();
        cshader.SetUniformMat4f("u_MVP", proj * camera.ComputeMatrix());
        c->Draw();

        cshader.UnBind();
        glBindTexture(GL_TEXTURE_2D, 0);
        glEnable(GL_MULTISAMPLE);
    }

    void TexSetup();

public:
    Voxel_t(GL::SceneLoader *_loader) : Scene(_loader), shader(ROOT_Directory + "/shader/Voxel/Block.vs", ROOT_Directory + "/shader/Voxel/Block.fs"),
                                        cshader(ROOT_Directory + "/shader/Voxel/Chunk.vs", ROOT_Directory + "/shader/Voxel/Block.fs"),
                                        camera({0, 0, 2}), controller(&camera, loader->GetWindow(),8)
    {
        RegisterFunc(std::bind(&Voxel_t::Render, this), GL::CallbackType::Render);
        glGenVertexArrays(1, &va);
        glGenBuffers(2, &vb);
        glBindVertexArray(va);
        glBindBuffer(GL_ARRAY_BUFFER, vb);

        glBufferData(GL_ARRAY_BUFFER, 36 * 5 * sizeof(float), &GL::Voxel::bvertices[0], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, false, 5 * sizeof(float), 0);
        glVertexAttribPointer(1, 2, GL_FLOAT, false, 5 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, instbuff);
        glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(float), offsets, GL_STATIC_DRAW);
        glVertexAttribPointer(2, 3, GL_FLOAT, false, 4 * sizeof(float), 0);
        glVertexAttribPointer(3, 1, GL_FLOAT, false, 4 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glEnableVertexAttribArray(3);
        glVertexAttribDivisor(2, 1);
        glVertexAttribDivisor(3, 1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        shader.Bind();
        shader.SetUniform1i("u_Texture", 0);
        shader.SetUniform1f("tex_size", 2 * 192.0f);
        shader.UnBind();

        cshader.Bind();
        cshader.SetUniform1i("u_Texture", 0);
        cshader.SetUniform1f("tex_size", 2 * 192.0f);
        cshader.UnBind();

        TexSetup();

        c = new GL::Voxel::Chunk({0, -1});
    }
    ~Voxel_t()
    {
        glDeleteVertexArrays(1, &va);
        glDeleteBuffers(2, &vb);
        RemoveFunctions();
        delete c;
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
}

SCENE_LOAD_FUNC(Voxel_t)