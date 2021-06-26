#include <Scene.hpp>
#include <ModelLoader.hpp>
#include <Data.hpp>
#include <Logger.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Camera/Flycam.hpp>

using namespace GL;

const uint NSTARS = 20;
const float spawnradius = 15;
const float velocity = 2;

struct Star
{
    glm::vec3 position;
    float radius;
    glm::vec3 velocity;

    Star(glm::vec3 p, float r, glm::vec3 v) : position(p), radius(r), velocity(v) {}
};

class Stars : public Scene
{
    Shader shader;
    Model model;
    uint instance_info;
    Logger log;

    glm::mat4 proj = glm::perspective(glm::radians(65.0f), (float)loader->GetWindow().GetWidth() / (float)loader->GetWindow().GetHeigth(), 0.1f, 100.0f);
    Camera3D cam;
    Flycam fc;

    std::vector<Star> stars;

    void Render()
    {

        log.print();
        shader.Bind();
        fc.Update(loader->GetTimeInfo().RenderDeltaTime());
        shader.SetUniformMat4f("u_MVP", proj * cam.ComputeMatrix());

        glBindBuffer(GL_ARRAY_BUFFER, instance_info);
        glBufferSubData(GL_ARRAY_BUFFER, 0, stars.size() * sizeof(Star), &stars[0]);
        glm::vec3 tmp;
        glGetBufferSubData(GL_ARRAY_BUFFER, 0, 3 * sizeof(float), &tmp);
        log<<tmp;
        model.Draw(shader, stars.size());

        shader.UnBind();
    }

public:
    Stars(SceneLoader *loaderr) : Scene(loaderr), shader(ROOT_Directory + "/shader/Stars.vs", ROOT_Directory + "/shader/Star.fs"),
                                  model(ROOT_Directory + "/res/Models/star.obj"), fc(&cam, loader->GetWindow())
    {
        RegisterFunc(CallbackType::Render, &Stars::Render, this);

        glGenBuffers(1, &instance_info);
        glBindBuffer(GL_ARRAY_BUFFER, instance_info);
        stars.reserve(NSTARS);
        glBufferData(GL_ARRAY_BUFFER, NSTARS * sizeof(Star), nullptr, GL_DYNAMIC_DRAW);

        InstanceBufferLayout layout;
        layout.stride = 7 * sizeof(float);
        layout.attributes.push_back({GL_FLOAT, 3, 0});
        layout.attributes.push_back({GL_FLOAT, 1, (void*)sizeof(glm::vec3)});
        model.AddInstanceBuffer(layout, instance_info);

        SetFlag("hide_menu", true);

        stars.emplace_back(glm::vec3{0, 0, -10}, 1.0f, glm::vec3{0, 0, 0});
    }

    ~Stars()
    {
        RemoveFunctions();
        glDeleteBuffers(1, &instance_info);
    }
};

SCENE_LOAD_FUNC(Stars);