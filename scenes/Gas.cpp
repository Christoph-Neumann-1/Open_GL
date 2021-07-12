#include <Scene.hpp>
#include <ModelLoader.hpp>
#include <Shader.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Data.hpp>
#include <Camera/Camera3D.hpp>
#include <Camera/Fplocked.hpp>
#include <random>

using namespace GL;

const uint natoms = 30;
const float initial_velocity = 0.2f;
const float dt_factor = 0.8f;
const float physics_rate = 5000;

const float attraction=120;
const float equal_dist=0.07f;

class AtomsSim : public Scene
{
    Model ball{ROOT_Directory + "/res/Models/sphere.obj"};
    Shader shader{ROOT_Directory + "/shader/Atoms.vs", ROOT_Directory + "/shader/Default.fs"};

    uint vb, va;

    uint offsets;

    struct Atom
    {
        glm::vec3 pos;
        glm::vec3 vel;

        Atom(glm::vec3 p, glm::vec3 v) : pos(p), vel(v) {}
    };

    std::vector<Atom> atoms;

    Camera3D camera{{0, 0, 1}};
    Fplocked fplocked{&camera, loader->GetWindow()};

    const glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)loader->GetWindow().GetWidth() / (float)loader->GetWindow().GetHeigth(), 0.1f, 100.0f);

    const std::array<glm::vec3, 6 * 6> sides{
        glm::vec3(1, -1, -1), glm::vec3(1, 1, -1), glm::vec3(-1, 1, -1), glm::vec3(-1, 1, -1), glm::vec3(-1, -1, -1), glm::vec3(1, -1, -1), //back
        glm::vec3(-1, -1, 1), glm::vec3(-1, 1, 1), glm::vec3(1, 1, 1), glm::vec3(1, 1, 1), glm::vec3(1, -1, 1), glm::vec3(-1, -1, 1),       //front
        glm::vec3(1, 1, 1), glm::vec3(-1, 1, 1), glm::vec3(-1, 1, -1), glm::vec3(1, 1, 1), glm::vec3(-1, 1, -1), glm::vec3(1, 1, -1),       //top
        glm::vec3(-1, -1, 1), glm::vec3(1, -1, 1), glm::vec3(1, -1, -1), glm::vec3(-1, -1, 1), glm::vec3(1, -1, -1), glm::vec3(-1, -1, -1), //bottom
        glm::vec3(-1, 1, 1), glm::vec3(-1, -1, 1), glm::vec3(-1, -1, -1), glm::vec3(-1, 1, 1), glm::vec3(-1, -1, -1), glm::vec3(-1, 1, -1), //left
        glm::vec3(1, 1, 1), glm::vec3(1, 1, -1), glm::vec3(1, -1, -1), glm::vec3(1, -1, -1), glm::vec3(1, -1, 1), glm::vec3(1, 1, 1)        //right
    };

    const glm::vec4 side_colors[6]{
        glm::vec4(0.0f, 0.8f, 0.0f, 1), //back/front
        glm::vec4(0.2f, 0.0f, 1.0f, 1), //top/bottom
        glm::vec4(1.0f, 0.2f, 0.0f, 1), //left/right
    };

    const glm::vec4 b_color{1.0f, 1.0f, 0.1f, 1};
    const float bradius = 0.055f;

    void RenderBox()
    {
        glBindVertexArray(va);

        fplocked.Update(loader->GetTimeInfo().RenderDeltaTime());

        shader.SetUniformMat4f("u_MVP", proj * camera.ComputeMatrix());
        shader.SetUniform1f("u_scale", 1);

        for (int i = 0; i < 6; i++)
        {
            shader.SetUniform4f("u_Color", side_colors[i / 2]);
            glDrawArrays(GL_TRIANGLES, 6 * i, 6);
        }

        glBindVertexArray(0);
    }

    void FillBuffer()
    {
        glm::vec3 buffer[natoms];
        for (uint i = 0; i < natoms; i++)
        {
            buffer[i] = atoms[i].pos;
        }
        glBindBuffer(GL_ARRAY_BUFFER, offsets);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * natoms, buffer);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void Render()
    {
        shader.Bind();
        RenderBox();

        shader.SetUniform4f("u_Color", b_color);
        shader.SetUniformMat4f("u_MVP", proj * camera.ComputeMatrix());
        shader.SetUniform1f("u_scale", bradius);

        FillBuffer();
        ball.Draw(shader, natoms);
        shader.UnBind();
    }

    void CollideWithWalls()
    {
        for (uint i = 0; i < atoms.size(); i++)
            for (int j = 0; j < 3; j++)
            {
                if (atoms[i].pos[j] + bradius > 1 || atoms[i].pos[j] - bradius < -1)
                {
                    atoms[i].vel[j] = -atoms[i].vel[j];
                }
            }
    }

    void ComputeForces(float dt)
    {
        for (int i = natoms - 1; i >= 0; i--)
        {
            for (int j = i - 1; j >= 0; j--)
            {
                glm::vec3 ij=atoms[j].pos-atoms[i].pos;
                glm::vec3 nij=glm::normalize(ij);
                float distij=glm::length(ij);

                glm::vec3 force=4*attraction*(powf((equal_dist/distij),12)-powf((equal_dist/distij),6))*nij*dt;
                atoms[i].vel-=force;
                atoms[j].vel+=force;
            }
        }
    }

        void Update()
        {
            auto dt = loader->GetTimeInfo().UpdateInterval() * dt_factor;
            CollideWithWalls();
            ComputeForces(dt);
            for (uint i = 0; i < natoms; i++)
                atoms[i].pos += atoms[i].vel * dt;
        }

//TODO: Avoid collisions
        void Setup()
        {
            uint seed = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
            srand(seed);

            for (uint i = 0; i < natoms; i++)
            {
                atoms.emplace_back(
                    glm::vec3{(float)rand() / (float)RAND_MAX * 1.8 - 0.9f, (float)rand() / (float)RAND_MAX * 1.8 - 0.9f, (float)rand() / (float)RAND_MAX * 1.8 -0.9f },
                    glm::vec3{(float)rand() / (float)RAND_MAX * initial_velocity - initial_velocity / 2, (float)rand() / (float)RAND_MAX * initial_velocity - initial_velocity / 2, (float)rand() / (float)RAND_MAX * initial_velocity - initial_velocity / 2});
            }
        }

    public:
        AtomsSim(SceneLoader * _loader) : Scene(_loader)
        {
            glGenVertexArrays(1, &va);
            glBindVertexArray(va);
            glGenBuffers(1, &vb);
            glBindBuffer(GL_ARRAY_BUFFER, vb);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

            glBufferData(GL_ARRAY_BUFFER, sizeof(sides), &sides, GL_STATIC_DRAW);

            glBindBuffer(GL_ARRAY_BUFFER, 0);

            RegisterFunc(CallbackType::Render, &AtomsSim::Render, this);
            RegisterFunc(CallbackType::Update, &AtomsSim::Update, this);

            GetFlag("hide_menu") = true;

            glGenBuffers(1, &offsets);
            glBindBuffer(GL_ARRAY_BUFFER, offsets);

            atoms.reserve(natoms);
            Setup();
            glBufferData(GL_ARRAY_BUFFER, natoms * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);

            InstanceBufferLayout layout;
            layout.stride = sizeof(glm::vec3);
            layout.attributes.push_back({GL_FLOAT, 3, 0});
            ball.AddInstanceBuffer(layout, offsets);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            loader->GetTimeInfo().SetUpdateInterval(1 / physics_rate);
        }
        ~AtomsSim()
        {
            glDeleteBuffers(1, &vb);
            glDeleteBuffers(1, &offsets);
            glDeleteVertexArrays(1, &va);
            RemoveFunctions();
        }
    };

    SCENE_LOAD_FUNC(AtomsSim)