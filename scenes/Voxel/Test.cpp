#include <Scene.hpp>
#include <glm/glm.hpp>
#include <Data.hpp>
#include <Voxel/Block.hpp>
#include <Shader.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Camera/FpCam.hpp>

class Voxel_t final : public GL::Scene
{
    uint va, vb, ib;
    GL::Shader shader;
    glm::mat4 proj = glm::perspective(glm::radians(60.0f), (float)loader->GetWindow().GetWidth() / loader->GetWindow().GetHeigth(), 0.05f, 500.0f);

    GL::Camera3D camera;
    GL::FpCam controller;

    void Render()
    {
        shader.Bind();
        controller.Update(loader->GetTimeInfo().RenderDeltaTime());
        shader.SetUniformMat4f("u_MVP", proj * camera.ComputeMatrix());
        glBindVertexArray(va);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);

        glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0, 1);
        // glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        shader.UnBind();
        glBindVertexArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

public:
    Voxel_t(GL::SceneLoader *_loader) : Scene(_loader), shader(ROOT_Directory + "/shader/Default.vs", ROOT_Directory + "/shader/Default.fs"),
                                        camera({0, 0, 2}), controller(&camera, loader->GetWindow())
    {
        RegisterFunc(std::bind(&Voxel_t::Render, this), GL::CallbackType::Render);
        glGenVertexArrays(1, &va);
        glGenBuffers(2, &vb);
        glBindVertexArray(va);
        glBindBuffer(GL_ARRAY_BUFFER, vb);

        glBufferData(GL_ARRAY_BUFFER, 8 * 5 * sizeof(float), &GL::Voxel::bvertices[0], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, false, 5 * sizeof(float), 0);
        glVertexAttribPointer(1, 2, GL_FLOAT, false, 5 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(uint), &GL::Voxel::bindices[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        shader.Bind();
        float color[]{0, 1, 0, 1};
        shader.SetUniform4f("u_Color", color);
        shader.UnBind();
    }
    ~Voxel_t()
    {
        glDeleteVertexArrays(1, &va);
        glDeleteBuffers(2, &vb);
        RemoveFunctions();
    }
};

SCENE_LOAD_FUNC(Voxel_t)