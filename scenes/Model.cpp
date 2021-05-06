#include <Scene.hpp>
#include <Data.hpp>
#include <Logger.hpp>
#include <ModelLoader.hpp>
#include <Flycam.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace GL;

class Star final : public Scene
{

    Shader shader;
    Model model;

    glm::mat4 proj = glm::perspective(glm::radians(65.0f), (float)loader->GetWindow().GetWidth() / (float)loader->GetWindow().GetHeigth(), 0.1f, 100.0f);
    Camera3D cam;
    Flycam fc;

    void Render()
    {
        shader.Bind();
        fc.Update(loader->GetTimeInfo().RenderDeltaTime());
        shader.SetUniformMat4f("u_MVP", proj *cam.ComputeMatrix());

        model.Draw(shader);

        shader.UnBind();

        if (glfwGetKey(loader->GetWindow(), GLFW_KEY_R))
        {
            loader->UnLoad();
        }
    }

public:
    Star(SceneLoader *_loader) : Scene(_loader),
                                 shader(ROOT_Directory + "/shader/Star.vs", ROOT_Directory + "/shader/Star.fs"),
                                 model(ROOT_Directory + "/res/Models/star.obj"), fc(&cam, loader->GetWindow(), 1,120,0.11)
    {
        RegisterFunc(std::bind(&Star::Render, this), CallbackType::Render);
    }

    ~Star()
    {
        loader->GetCallback().RemoveAll(callback_id);
    }
};

// Calls the constructor of Star
SCENE_LOAD_FUNC(Star)