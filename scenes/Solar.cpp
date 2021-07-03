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

const double G = 5;
const float FOV = 65;
const float clipping_distance = 2000;
const double simulations_per_second = 1000;
const double speed = 1000;
const double scale_factor = 1.0 / 52000.0;

uint nplanets;

struct SpaceObject
{
    glm::dvec3 position;
    double radius;
    glm::dvec3 velocity;
    double mass;
    Model model;

    SpaceObject(glm::dvec3 p, float d, glm::dvec3 v, float m, const std::string &mpath) : position(p), radius(d), velocity(v), mass(m), model(mpath) {}
};

class SolarSim : public Scene
{
    Shader shader;
    Logger log;

    glm::mat4 proj = glm::perspective(glm::radians(FOV), (float)loader->GetWindow().GetWidth() / (float)loader->GetWindow().GetHeigth(), 0.1f, clipping_distance);
    Camera3D cam;
    Flycam fc;

    std::vector<SpaceObject> planets;

    void ComputePositions()
    {
        double dt = loader->GetTimeInfo().UpdateInterval() * speed;

        for (int i = nplanets - 1; i >= 0; i--)
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

    void Render()
    {
        shader.Bind();
        fc.Update(loader->GetTimeInfo().RenderDeltaTime());

        for (auto &planet : planets)
        {
            shader.SetUniformMat4f("u_MVP", proj * cam.ComputeMatrix() *
                                                glm::scale(glm::translate(glm::mat4(), (glm::vec3)(planet.position * scale_factor)), (float)(scale_factor * planet.radius) * glm::vec3(1.0f)));
            planet.model.Draw(shader);
        }

        shader.UnBind();
    }

    void Setup()
    {
    }

public:
    SolarSim(SceneLoader *loaderr) : Scene(loaderr), shader(ROOT_Directory + "/shader/Stars.vs", ROOT_Directory + "/shader/Star.fs"),
                                     fc(&cam, loader->GetWindow(), 100)
    {
        RegisterFunc(CallbackType::Render, &SolarSim::Render, this);
        RegisterFunc(CallbackType::Update, &SolarSim::ComputePositions, this);

        planets.reserve(nplanets);

        SetFlag("hide_menu", true);

        loader->GetTimeInfo().SetUpdateInterval(1 / simulations_per_second);
    }

    ~SolarSim()
    {
        RemoveFunctions();
        loader->GetTimeInfo().SetUpdateInterval();
    }
};

SCENE_LOAD_FUNC(SolarSim);