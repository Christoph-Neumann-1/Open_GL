//Please use fast math and o3

#include <Scene.hpp>
#include <ModelLoader.hpp>
#include <Data.hpp>
#include <Logger.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Camera/Flycam.hpp>
#include <random>
#include <stdio.h>

using namespace GL;

const double G = 6.67e-11;
const float FOV = 65;
const float clipping_distance = 10000;
const double simulations_per_second = 1000;
const double speed = 100000;
const double scale_factor = 1.0 / 5200.0;

struct SpaceObject
{
    glm::dvec3 position;
    glm::vec3 color;
    float radius;
    glm::dvec3 velocity;
    double mass;

    SpaceObject(glm::dvec3 p, float r, glm::dvec3 v, float m, glm::vec3 c) : position(p), color(c), radius(r), velocity(v), mass(m) {}
};

class SolarSim : public Scene
{
    Shader shader;
    Logger log;

    glm::mat4 proj = glm::perspective(glm::radians(FOV), (float)loader->GetWindow().GetWidth() / (float)loader->GetWindow().GetHeigth(), 0.1f, clipping_distance);
    Camera3D cam;
    Flycam fc;
    uint instance_info;
    Model model;

    std::array<SpaceObject, 2> planets{
        SpaceObject(glm::dvec3(0, 510000000, 0), 696340.0, glm::dvec3(0, 0, 0), 1.989e30, glm::vec3(1, 0, 0)),
        SpaceObject(glm::dvec3(0, 0, 0), 6371.0, glm::dvec3(0, 0, 0), 5.97e24, glm::vec3(0, 1, 0)),
    };

    void ComputePositions()
    {
        double dt = loader->GetTimeInfo().UpdateInterval() * speed;

        for (int i = planets.size() - 1; i >= 0; i--)
        {
            for (int j = i - 1; j >= 0; j--)
            {
                glm::dvec3 ij = planets[j].position - planets[i].position;
                auto ij_normalized = glm::normalize(ij);
                auto force_ij = planets[i].mass * planets[j].mass / glm::length2(ij) * G * ij_normalized * dt;
                planets[i].velocity += force_ij / planets[i].mass;
                planets[j].velocity -= force_ij / planets[j].mass;
            }
            planets[i].position += planets[i].velocity * dt;
        }
    }

    void UpdateBuffer()
    {
        float tmpbuffer[7 * sizeof(float) * planets.size()];
        for (int i = 0; i < planets.size(); i++)
        {
            auto &planet = planets[i];
            *((glm::vec3 *)&tmpbuffer[i * 7]) = planet.position * scale_factor;
            *((glm::vec3 *)&tmpbuffer[i * 7 + 3]) = planet.color;
            *((float *)&tmpbuffer[i * 7 + 6]) = planet.radius * scale_factor;
            log(planet.radius / scale_factor);
        }
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * 7 * planets.size(), tmpbuffer);
    }

    void Render()
    {
        shader.Bind();
        fc.Update(loader->GetTimeInfo().RenderDeltaTime());
        shader.SetUniformMat4f("u_MVP", proj * cam.ComputeMatrix());

        glBindBuffer(GL_ARRAY_BUFFER, instance_info);
        UpdateBuffer();

        model.Draw(shader, planets.size());

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        shader.UnBind();
    }

    void Setup()
    {
    }

public:
    SolarSim(SceneLoader *loaderr) : Scene(loaderr), shader(ROOT_Directory + "/shader/Solar.vs", ROOT_Directory + "/shader/Batch.fs"),
                                     cam({0, 0, 10}), fc(&cam, loader->GetWindow(), 100), model(ROOT_Directory + "/res/Models/sphere.obj")
    {
        RegisterFunc(CallbackType::Render, &SolarSim::Render, this);
        RegisterFunc(CallbackType::Update, &SolarSim::ComputePositions, this);

        SetFlag("hide_menu", true);

        loader->GetTimeInfo().SetUpdateInterval(1 / simulations_per_second);

        glGenBuffers(1, &instance_info);
        glBindBuffer(GL_ARRAY_BUFFER, instance_info);

        glBufferData(GL_ARRAY_BUFFER, 7 * sizeof(float) * planets.size(), nullptr, GL_DYNAMIC_DRAW);

        InstanceBufferLayout layout;
        layout.stride = 7 * sizeof(float);
        layout.attributes.push_back({GL_FLOAT, 3, 0});
        layout.attributes.push_back({GL_FLOAT, 3, (void *)sizeof(glm::vec3)});
        layout.attributes.push_back({GL_FLOAT, 1, (void *)(2 * sizeof(glm::vec3))});
        model.AddInstanceBuffer(layout, instance_info);

        // cam.UnlockMouse(loader->GetWindow());
    }

    ~SolarSim()
    {
        RemoveFunctions();
        glDeleteBuffers(1, &instance_info);
        loader->GetTimeInfo().SetUpdateInterval();
    }
};

SCENE_LOAD_FUNC(SolarSim);