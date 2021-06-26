#include <Scene.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Shader.hpp>
#include <Data.hpp>
#include <Camera/FpCam.hpp>
#include <glm/gtx/quaternion.hpp>
#include <Logger.hpp>

using namespace GL;

class S3D final : public Scene
{

    glm::mat4 proj = glm::perspective(glm::radians(60.0f), 800.0f / 600.0f, 0.1f, 100.0f);
    Camera3D camera;
    FpCam fpcam;

    uint VBO;
    uint VAO;
    Shader shader;

    void Render()
    {
        glBindVertexArray(VAO);
        shader.Bind();
        fpcam.Update(loader->GetTimeInfo().RenderDeltaTime());

        shader.SetUniformMat4f("u_MVP", proj * camera.ComputeMatrix());

        glDrawArrays(GL_TRIANGLES, 0, 3);
        shader.UnBind();
        if (glfwGetKey(loader->GetWindow(), GLFW_KEY_L))
        {
            loader->Load(ROOT_Directory + "/scenes/bin/3d.scene");
        }
        if (glfwGetKey(loader->GetWindow(), GLFW_KEY_R))
        {
            loader->UnLoad();
        }
    }

public:
    S3D(SceneLoader *_loader) : Scene(_loader), fpcam(&camera, loader->GetWindow(), 1.0f, 6.0f),
                                shader(ROOT_Directory + "/shader/Default.vs", ROOT_Directory + "/shader/Default.fs")
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        float vertices[]{0, 0, -1, 1, 0, -1, 1, 1, -1};
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);

        float color[]{1, 1, 0, 1};
        shader.Bind();
        shader.SetUniform4f("u_Color", color);
        RegisterFunc(CallbackType::Render,&S3D::Render, this);
    }
    ~S3D()
    {
        loader->GetCallback().RemoveAll(callback_id);
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
    }
};

SCENE_LOAD_FUNC(S3D)