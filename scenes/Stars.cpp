//Please use fast math and o3

#include <Scene.hpp>
#include <ModelLoader.hpp>
#include <Data.hpp>
#include <Logger.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Camera/Flycam.hpp>
#include <random>
#include <Buffer.hpp>

using namespace GL;

const uint NSTARS = 800;
const float spawnradius = 315;
const float minsize = 0.5, maxsize = 2.3f;
const float velocity = 11;
const float G = 4.95f;
const float FOV = 65;
const float clipping_distance = 2000;
const float dt_factor = 2;

const glm::vec3 offset(318, 0, 0);
const glm::vec3 speeddiff(0, 6, 0);

struct Star
{
    glm::vec3 position;
    float radius;
    glm::vec3 velocity;

    Star(glm::vec3 p, float d, glm::vec3 v) : position(p), radius(d), velocity(v) {}
};

class Stars : public Scene
{
    Shader shader{ROOT_Directory + "/shader/Stars.vs", ROOT_Directory + "/shader/Star.fs"};
    Model model{ROOT_Directory + "/res/Models/star.obj"};
    Buffer instance_info;
    Logger log;

    glm::mat4 proj = glm::perspective(glm::radians(FOV), (float)loader->GetWindow().GetWidth() / (float)loader->GetWindow().GetHeigth(), 0.1f, clipping_distance);
    Camera3D cam;
    Flycam fc{cam, loader->GetWindow(), 100};

    std::vector<Star> stars;

    void ComputePositions()
    {
        float dt = loader->GetTimeInfo().UpdateInterval() * dt_factor;
        int nstars = stars.size();

        for (int i = nstars - 1; i >= 0; i--)
        {
            for (int j = i - 1; j >= 0; j--)
            {
                glm::vec3 ij = stars[j].position - stars[i].position;
                auto ij_normalized = glm::normalize(ij);
                float massi = 4.0f / 3.0f * glm::pi<float>() * powf(stars[i].radius, 3.0f);
                float massj = 4.0f / 3.0f * glm::pi<float>() * powf(stars[j].radius, 3.0f);
                auto force_ij = massi * massj / glm::length2(ij) * G * ij_normalized * dt;
                stars[i].velocity += force_ij / massi;
                stars[j].velocity -= force_ij / massj;
                if (glm::length(ij) < stars[i].radius + stars[j].radius)
                {
                    stars[j].position = (stars[i].position * massi + stars[j].position * massj) / (massi + massj);
                    stars[j].velocity = (stars[i].velocity * massi + stars[j].velocity * massj) / (massi + massj);
                    stars[j].radius = powf(3 * (massi + massj) / (4.0f * glm::pi<float>()), 1.0f / 3.0f);
                    stars.erase(stars.begin() + i);
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

        instance_info.Bind(GL_ARRAY_BUFFER);
        glBufferSubData(GL_ARRAY_BUFFER, 0, stars.size() * sizeof(Star), &stars[0]);
        model.Draw(shader, stars.size());
        Buffer::Unbind(GL_ARRAY_BUFFER);

        shader.UnBind();
    }

    void SetupStars()
    {
        uint seed = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        srand(seed);

        for (uint i = 0; i < NSTARS; i++)
        {
            stars.emplace_back(
                glm::vec3{(float)rand() / (float)RAND_MAX * spawnradius - spawnradius / 2, (float)rand() / (float)RAND_MAX * spawnradius - spawnradius / 2, (float)rand() / (float)RAND_MAX * spawnradius - spawnradius / 2},
                (float)rand() / (float)RAND_MAX * (maxsize - minsize) + minsize,
                glm::vec3{(float)rand() / (float)RAND_MAX * velocity - velocity / 2, (float)rand() / (float)RAND_MAX * velocity - velocity / 2, (float)rand() / (float)RAND_MAX * velocity - velocity / 2} - speeddiff);
        }

        for (uint i = 0; i < NSTARS; i++)
        {
            stars.emplace_back(
                glm::vec3{(float)rand() / (float)RAND_MAX * spawnradius - spawnradius / 2, (float)rand() / (float)RAND_MAX * spawnradius - spawnradius / 2, (float)rand() / (float)RAND_MAX * spawnradius - spawnradius / 2} + offset,
                (float)rand() / (float)RAND_MAX * (maxsize - minsize) + minsize,
                glm::vec3{(float)rand() / (float)RAND_MAX * velocity - velocity / 2, (float)rand() / (float)RAND_MAX * velocity - velocity / 2, (float)rand() / (float)RAND_MAX * velocity - velocity / 2} + speeddiff);
        }
    }

public:
    Stars(SceneLoader *loaderr) : Scene(loaderr)
    {
        RegisterFunc(CallbackType::Render, &Stars::Render, this);
        RegisterFunc(CallbackType::Update, &Stars::ComputePositions, this);

        instance_info.Bind(GL_ARRAY_BUFFER);
        stars.reserve(2 * NSTARS);
        glBufferData(GL_ARRAY_BUFFER, 2 * NSTARS * sizeof(Star), nullptr, GL_DYNAMIC_DRAW);

        VertexBufferLayout layout;
        layout.stride = 7 * sizeof(float);
        layout.attributes.push_back({GL_FLOAT, 3, 0});
        layout.attributes.push_back({GL_FLOAT, 1, (void *)sizeof(glm::vec3)});
        model.AddInstanceBuffer(layout, instance_info);

        Buffer::Unbind(GL_ARRAY_BUFFER);

        GetFlag("hide_menu") = true;

        SetupStars();

        loader->GetTimeInfo().SetUpdateInterval(1 / 250.0f);

        loader->GetWindow().bgcolor=glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    }

    ~Stars()
    {
        RemoveFunctions();
        loader->GetTimeInfo().SetUpdateInterval();
        loader->GetWindow().bgcolor=Window::defaultbg;
    }
};

SCENE_LOAD_FUNC(Stars);