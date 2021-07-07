//This is an attempt to make a solar system simulation.
// The main problem right now is that the distances between the planets and the sun makes the planets to small to see.
// I do not have a good solution for this yet.

#include <Scene.hpp>
#include <ModelLoader.hpp>
#include <Data.hpp>
#include <Logger.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Camera/Flycam.hpp>

using namespace GL;

const double G = 6.67e-11;
const float FOV = 65;
const float clipping_distance = 10000;// How far you can see in after scaling down.
const double simulations_per_second = 1000;//The resulution of the simulation. The higher the number the more accurate the simulation will be. 
const double speed = 100000;//How fast the simulation runs.
const double scale_factor = 1.0 / 5200.0;//How much smaller to draw everything.

//The information relevant for renderering is position, radius and color.
//The rest is to simulate gravity.
//The doubles are used to ensure accuracy for the outer planets, as well as allow the camera to get closer once I implement more interesting planets.
struct SpaceObject
{
    glm::dvec3 position;//km
    glm::vec3 color;//RGB
    float radius;//km
    glm::dvec3 velocity;//kilometes per second
    double mass;//kg

    SpaceObject(glm::dvec3 p, float r, glm::dvec3 v, float m, glm::vec3 c) : position(p), color(c), radius(r), velocity(v), mass(m) {}
};

class SolarSim : public Scene
{
    Shader shader{ROOT_Directory + "/shader/Solar.vs", ROOT_Directory + "/shader/Batch.fs"};
    Logger log;

    glm::mat4 proj = glm::perspective(glm::radians(FOV), (float)loader->GetWindow().GetWidth() / (float)loader->GetWindow().GetHeigth(), 0.1f, clipping_distance);
    Camera3D cam{{0, 0, 10}};
    Flycam fc{&cam, loader->GetWindow(), 100};//This type of camera allows for you to move and rotate freely, ideal for a space simulation.
    uint instance_info;//The buffer storing the information about every object. Position, color and radius are stored in this buffer.
    Model model{ROOT_Directory + "/res/Models/sphere.obj"};//A model of a sphere with no textures.

    //All the planets and the sun are defined here. There is no need for this container to be a vector, as I do not intend to add or remove objects at runtime.
    std::array<SpaceObject, 2> planets{
        SpaceObject(glm::dvec3(0, 0, 0), 696340.0, glm::dvec3(0, 0, 0), 1.989e30, glm::vec3(1, 0, 0)),//Sun
        SpaceObject(glm::dvec3(0, 510000000, 0), 6371.0, glm::dvec3(0, 0, 0), 5.97e24, glm::vec3(0, 1, 0)),//Earth
    };

    //Compute forces between objects and update positions accoringly.
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

    //Fill the buffer with data from the array of planets.
    //The postion and radius are scaled down acoording to the scale_factor.
    //The position is also cast to a vector of floats afterwards.
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

    //This function is called every frame and draws the planets. All Opengl operations are done here.
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

public:
    SolarSim(SceneLoader *loaderr) : Scene(loaderr)
    {
        RegisterFunc(CallbackType::Render, &SolarSim::Render, this);
        RegisterFunc(CallbackType::Update, &SolarSim::ComputePositions, this);//This run on a sepearate thread. 

        GetFlag("hide_menu")= true;

        loader->GetTimeInfo().SetUpdateInterval(1 / simulations_per_second);//Tells the Update thread how often to run.

        glGenBuffers(1, &instance_info);
        glBindBuffer(GL_ARRAY_BUFFER, instance_info);

        glBufferData(GL_ARRAY_BUFFER, 7 * sizeof(float) * planets.size(), nullptr, GL_DYNAMIC_DRAW);

        InstanceBufferLayout layout;
        layout.stride = 7 * sizeof(float);
        layout.attributes.push_back({GL_FLOAT, 3, 0});//position
        layout.attributes.push_back({GL_FLOAT, 3, (void *)sizeof(glm::vec3)});//color
        layout.attributes.push_back({GL_FLOAT, 1, (void *)(2 * sizeof(glm::vec3))});//radius
        model.AddInstanceBuffer(layout, instance_info);

        loader->GetWindow().bgcolor=glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        // cam.UnlockMouse(loader->GetWindow()); //In case I want to debug the program, I need to be able to move the mouse.
    }

    ~SolarSim()
    {
        RemoveFunctions();
        glDeleteBuffers(1, &instance_info);
        loader->GetTimeInfo().SetUpdateInterval();
        loader->GetWindow().bgcolor=Window::defaultbg;
    }
};

//The load function simply creates a new instance of the scene and returns a pointer to it.
//The scene will be deleted by the loader.
//This function can be created manually, but it looks the same for almost all scenes, so I created a macro for it.
SCENE_LOAD_FUNC(SolarSim);