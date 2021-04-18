#include <SceneLoader.hpp>
#include <Scene.hpp>
#include <Renderer.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Callback.hpp>
#include <Shader.hpp>
#include <Data.hpp>
#include <Flycam.hpp>
#include <Model_Instanced.hpp>
#include <vector>
#include <algorithm>
#include <glm/glm.hpp>
#include <random>

struct Star
{
    glm::vec3 pos;
    glm::vec3 velocity;
    float scale;
};

const float spawnrange = 25;
const float spawnv = 3.5;
const float startscale = 1;
const u_int nStars = 25;
const float G = 16;

class StarSim final : public Scene
{
    SceneLoader *loader;
    Model star;
    unsigned int Transforms;
    std::vector<Star> stars;
    Shader shader;

    Camera3D camera;
    Flycam camcontrol;
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)loader->window.x / (float)loader->window.y, 0.1f, 1500.0f);
    std::vector<std::pair<int, int>> collisions;
    std::vector<int> deleted_stars;
    void OnUpdate()
    {

        float dt = loader->callbackhandler->deltatime_update;
        camcontrol.Update(loader->callbackhandler->deltatime_update);
        for (u_int i = 0; i < stars.size(); i++)
        {
            float volume1 = 4 / 3 * M_PI * powf(stars[i].scale, 3);
            for (u_int j = i + 1; j < stars.size(); j++)
            {
                float volume2 = 4 / 3 * M_PI * powf(stars[j].scale, 3);
                glm::vec3 vec = stars[j].pos - stars[i].pos;
                float dist = glm::length(vec);
                float force = volume1 * volume2 / powf(dist, 2) * G * dt;
                stars[i].velocity += force / volume1 * glm::normalize(vec);
                stars[j].velocity += force / volume2 * glm::normalize(-vec);
                if (dist <= stars[i].scale + stars[j].scale)
                {
                    collisions.emplace_back(i, j);
                }
            }
            stars[i].pos += stars[i].velocity * dt;
        }
        for (auto collision : collisions)
        {
            float v1 = (4.0 / 3.0) * M_PI * powf(stars[collision.first].scale, 3);
            float v2 = (4.0 / 3.0) * M_PI * powf(stars[collision.second].scale, 3);
            float combined_scale = powf(3 * (v1 + v2) / (4 * M_PI), 1.0 / 3.0);
            glm::vec3 centerofmass = (stars[collision.first].pos * v1 + stars[collision.second].pos * v2) / (v1 + v2);
            glm::vec3 average_velocity = (stars[collision.first].velocity * v1 + stars[collision.second].velocity * v2) / (v1 + v2);

            stars[collision.first] = {centerofmass, average_velocity, combined_scale};
            deleted_stars.emplace_back(collision.second);
        }
        std::sort(deleted_stars.begin(), deleted_stars.end(),std::greater<int>());
        for (int i : deleted_stars)
        {
            stars.erase(stars.begin() + i);
        }
        glBindBuffer(GL_ARRAY_BUFFER, Transforms);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Star) * stars.size(), &stars[0]);
        deleted_stars.clear();
        collisions.clear();
    }

    void OnRender()
    {
        shader.SetUniformMat4f("u_MVP", proj * camera.ComputeMatrix());
        star.Draw(shader, stars.size());
    }

public:
    StarSim(SceneLoader *loader);

    ~StarSim()
    {
        loader->callbackhandler->RemoveAll(this);
    }
};

StarSim::StarSim(SceneLoader *loader)
    : loader(loader), star(ROOT_Directory + "/res/Models/star.obj"), shader(ROOT_Directory + "/res/Shaders/Star.glsl"), camera({0, 0, 3}),
      camcontrol(&camera, loader->window, 70, 120, 0.11)
{
    glGenBuffers(1, &Transforms);
    InstanceBufferLayout layout;
    layout.stride = sizeof(Star);
    layout.attributes.emplace_back(GL_FLOAT, 3, (void *)0);
    layout.attributes.emplace_back(GL_FLOAT, 3, (void *)offsetof(Star, velocity));
    layout.attributes.emplace_back(GL_FLOAT, 1, (void *)offsetof(Star, scale));
    star.AddInstanceBuffer(layout, Transforms);
    loader->callbackhandler->Register(CallbackHandler::CallbackType::Render, this, [](void *x, void *) { reinterpret_cast<StarSim *>(x)->OnRender(); });
    loader->callbackhandler->Register(CallbackHandler::CallbackType::Update, this, [](void *x, void *) { reinterpret_cast<StarSim *>(x)->OnUpdate(); });
    glBindBuffer(GL_ARRAY_BUFFER, Transforms);

    stars.reserve(nStars);
    srand(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count());
    for (u_int i = 0; i < nStars; i++)
    {
        glm::vec3 pos = {((float)rand() / (RAND_MAX / 2) - 1) * spawnrange, ((float)rand() / (RAND_MAX / 2) - 1) * spawnrange, ((float)rand() / (RAND_MAX / 2) - 1) * spawnrange};
        glm::vec3 vel = {((float)rand() / (RAND_MAX / 2) - 1) * spawnv, ((float)rand() / (RAND_MAX / 2) - 1) * spawnv, ((float)rand() / (RAND_MAX / 2) - 1) * spawnv};
        stars.emplace_back(pos, vel, ((float)rand()/RAND_MAX+0.5)*startscale);
    }
    glBufferData(GL_ARRAY_BUFFER, sizeof(Star) * nStars, &stars[0], GL_DYNAMIC_DRAW);

    flags["hide_menu"] = true;
}

extern "C"
{

    Scene *_INIT_(SceneLoader *loader)
    {
        return new StarSim(loader);
    }

    void _EXIT_(Scene *scene)
    {
        delete scene;
    }
}