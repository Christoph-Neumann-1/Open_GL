#include <Scene.hpp>
#include <ModelLoader.hpp>
#include <Data.hpp>
#include <Logger.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Camera/Flycam.hpp>
#include <random>

using namespace GL;

const uint NSTARS = 3;
const float spawnradius = 10;
const float velocity = 1;
const float G = 5;

struct Star
{
    glm::vec3 position;
    float radius;
    glm::vec3 velocity;

    Star(glm::vec3 p, float d, glm::vec3 v) : position(p), radius(d), velocity(v) {}
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

    void ComputePositions()
    {
        float dt = loader->GetTimeInfo().UpdateInterval();
        int nstars = stars.size();

        for (int i = nstars-1; i >=0; i--)
        {
            for (int j = i-1; j >=0; j--)
            {
                glm::vec3 ij = stars[j].position - stars[i].position;
                auto ij_normalized = glm::normalize(ij);
                float massi = 4.0f / 3.0f * glm::pi<float>() * powf(stars[i].radius, 3.0f);
                float massj = 4.0f / 3.0f * glm::pi<float>() * powf(stars[j].radius, 3.0f);
                auto force_ij = massi * massj / glm::length2(ij) * G * ij_normalized * dt;
                stars[i].velocity += force_ij / massi;
                stars[j].velocity -= force_ij / massj;
                if (glm::length2(ij) < stars[i].radius + stars[j].radius)
                {
                    stars[j].position=(stars[i].position*massi+stars[j].position*massj)/(massi+massj);
                    stars[j].velocity=(stars[i].velocity*massi+stars[j].velocity*massj)/(massi+massj);
                    stars[j].radius=powf(3*(massi+massj)/(4.0f*glm::pi<float>()),1.0f/3.0f);
                    stars.erase(stars.begin()+i);
                    break;
                }
            }
            stars[i].position += stars[i].velocity * dt;
        }
    }

    void Render()
    {
        shader.Bind();
        fc.Update(loader->GetTimeInfo().RenderDeltaTime());
        shader.SetUniformMat4f("u_MVP", proj * cam.ComputeMatrix());

        glBindBuffer(GL_ARRAY_BUFFER, instance_info);
        glBufferSubData(GL_ARRAY_BUFFER, 0, stars.size() * sizeof(Star), &stars[0]);
        model.Draw(shader, stars.size());

        shader.UnBind();
    }

    void SetupStars()
    {
        uint seed = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        srand(seed);

        for (int i = 0; i < NSTARS; i++)
        {
            stars.emplace_back(
                glm::vec3{(float)rand() / (float)RAND_MAX * spawnradius - spawnradius / 2, (float)rand() / (float)RAND_MAX * spawnradius - spawnradius / 2, (float)rand() / (float)RAND_MAX * spawnradius - spawnradius / 2},
                (float)rand() / (float)RAND_MAX + 0.5f,
                glm::vec3{(float)rand() / (float)RAND_MAX * velocity - velocity / 2, (float)rand() / (float)RAND_MAX * velocity - velocity / 2, (float)rand() / (float)RAND_MAX * velocity - velocity / 2});
        }
    }

public:
    Stars(SceneLoader *loaderr) : Scene(loaderr), shader(ROOT_Directory + "/shader/Stars.vs", ROOT_Directory + "/shader/Star.fs"),
                                  model(ROOT_Directory + "/res/Models/star.obj"), fc(&cam, loader->GetWindow())
    {
        RegisterFunc(CallbackType::Render, &Stars::Render, this);
        RegisterFunc(CallbackType::Update, &Stars::ComputePositions, this);

        glGenBuffers(1, &instance_info);
        glBindBuffer(GL_ARRAY_BUFFER, instance_info);
        stars.reserve(NSTARS);
        glBufferData(GL_ARRAY_BUFFER, NSTARS * sizeof(Star), nullptr, GL_DYNAMIC_DRAW);

        InstanceBufferLayout layout;
        layout.stride = 7 * sizeof(float);
        layout.attributes.push_back({GL_FLOAT, 3, 0});
        layout.attributes.push_back({GL_FLOAT, 1, (void *)sizeof(glm::vec3)});
        model.AddInstanceBuffer(layout, instance_info);

        SetFlag("hide_menu", true);

        SetupStars();
    }

    ~Stars()
    {
        RemoveFunctions();
        glDeleteBuffers(1, &instance_info);
    }
};

SCENE_LOAD_FUNC(Stars);