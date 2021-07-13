#include <Scene.hpp>
#include <ModelLoader.hpp>
#include <Shader.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Data.hpp>
#include <Camera/Camera3D.hpp>
#include <Camera/Fplocked.hpp>
#include <random>
#include <Input.hpp>

using namespace GL;

const uint natoms = 50;
const float initial_velocity = 0.2f;
const float dt_factor = 0.8f;
const float physics_rate = 5000;

const float attraction = 110;
const float equal_dist = 0.07f;
const float G = 0.15f;

class AtomsSim : public Scene
{
    Model atom_model{ROOT_Directory + "/res/Models/sphere.obj"};
    Shader shader{ROOT_Directory + "/shader/Atoms.vs", ROOT_Directory + "/shader/Atoms.fs"};
    Shader wall_shader{ROOT_Directory + "/shader/Default.vs", ROOT_Directory + "/shader/Default.fs"};

    std::mutex mutex;

    uint vb, va;

    float Temperature;

    glm::vec2 temp_gauge[6]{
        glm::vec2(1.0f, 0.0f),
        glm::vec2(1.0f, 1.0f),
        glm::vec2(0.0f, 1.0f),
        glm::vec2(0.0f, 1.0f),
        glm::vec2(0.0f, 0.0f),
        glm::vec2(1.0f, 0.0f)
    };
    
    glm::mat4 ortho_ui=glm::ortho(0.0f, 100.0f, 0.0f, 1000.0f, -1.0f, 1.0f);

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
        glm::vec4(0.8f, 0.0f, 0.7f, 1), //top/bottom
        glm::vec4(1.0f, 0.2f, 0.0f, 1), //left/right
    };

    const glm::vec3 b_color{1.0f, 1.0f, 0.1f};
    const float bradius = 0.055f;

    void RenderBox()
    {
        glBindVertexArray(va);

        wall_shader.Bind();

        fplocked.Update(loader->GetTimeInfo().RenderDeltaTime());

        wall_shader.SetUniformMat4f("u_MVP", proj * camera.ComputeMatrix());

        for (int i = 0; i < 6; i++)
        {
            wall_shader.SetUniform4f("u_Color", side_colors[i / 2]);
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
        RenderBox();
        shader.Bind();

        shader.SetUniformMat4f("u_V", camera.ComputeMatrix());
        shader.SetUniform3f("u_CameraPosition", camera.position);

        FillBuffer();
        atom_model.Draw(shader, natoms);
        shader.UnBind();
    }

    void CollideWithWalls()
    {
        for (uint i = 0; i < atoms.size(); i++)
            for (uint j = 0; j < 3; j++)
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
                glm::vec3 ij = atoms[j].pos - atoms[i].pos;
                glm::vec3 nij = glm::normalize(ij);
                float distij = glm::length(ij);

                glm::vec3 force = 4 * attraction * (powf((equal_dist / distij), 12) - powf((equal_dist / distij), 6)) * nij * dt;
                atoms[i].vel -= force;
                atoms[j].vel += force;
            }
        }
    }

    void ComputeGravity(float dt)
    {
        for (auto &atom : atoms)
        {
            atom.vel -= G * dt;
        }
    }

    void UpdatePositions(float dt)
    {
        for (auto &atom : atoms)
        {
            atom.pos += atom.vel * dt;
        }
    }

    void Update()
    {
        static float dt = loader->GetTimeInfo().UpdateInterval() * dt_factor;
        {
            std::scoped_lock lock(mutex);
            CollideWithWalls();
            ComputeForces(dt);
            ComputeGravity(dt);
            UpdatePositions(dt);
        }
    }

    //TODO: Avoid collisions
    void Setup()
    {
        uint seed = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        srand(seed);

        for (uint i = 0; i < natoms; i++)
        {
            atoms.emplace_back(
                glm::vec3{(float)rand() / (float)RAND_MAX * 1.8 - 0.9f, (float)rand() / (float)RAND_MAX * 1.8 - 0.9f, (float)rand() / (float)RAND_MAX * 1.8 - 0.9f},
                glm::vec3{(float)rand() / (float)RAND_MAX * initial_velocity - initial_velocity / 2, (float)rand() / (float)RAND_MAX * initial_velocity - initial_velocity / 2, (float)rand() / (float)RAND_MAX * initial_velocity - initial_velocity / 2});
        }
    }

    InputHandler::KeyCallback pluskey{*loader->GetWindow().inputptr, glfwGetKeyScancode(GLFW_KEY_KP_ADD), InputHandler::Action::Press, &AtomsSim::ModifyVelocity, this, 1.1f};
    InputHandler::KeyCallback minuskey{*loader->GetWindow().inputptr, glfwGetKeyScancode(GLFW_KEY_KP_SUBTRACT), InputHandler::Action::Press, &AtomsSim::ModifyVelocity, this, 0.9f};

    void ModifyVelocity(float change_percent, int)
    {
        std::scoped_lock lock(mutex);

        for (auto &atom : atoms)
        {
            atom.vel *= change_percent;
        }
    }

public:
    AtomsSim(SceneLoader *_loader) : Scene(_loader)
    {
#pragma region Buffers
        glGenVertexArrays(1, &va);
        glBindVertexArray(va);
        glGenBuffers(1, &vb);
        glGenBuffers(1, &offsets);
        glBindBuffer(GL_ARRAY_BUFFER, vb);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glBufferData(GL_ARRAY_BUFFER, sizeof(sides), &sides, GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, offsets);

        glBufferData(GL_ARRAY_BUFFER, natoms * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);
        InstanceBufferLayout layout;
        layout.stride = sizeof(glm::vec3);
        layout.attributes.push_back({GL_FLOAT, 3, 0});
        atom_model.AddInstanceBuffer(layout, offsets);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

#pragma endregion

        atoms.reserve(natoms);
        Setup();

        RegisterFunc(CallbackType::Render, &AtomsSim::Render, this);
        RegisterFunc(CallbackType::Update, &AtomsSim::Update, this);

        GetFlag("hide_menu") = true;

        loader->GetTimeInfo().SetUpdateInterval(1 / physics_rate);

#pragma region Uniforms
        shader.Bind();
        shader.SetUniform3f("u_Color", b_color);
        shader.SetUniformMat4f("u_P", proj);
        shader.SetUniformMat4f("u_M", glm::scale(glm::mat4(1), glm::vec3(bradius)));

        shader.SetUniform3f("u_LightDirection", glm::normalize(glm::vec3(0, -1, -0.3)));
        shader.SetUniform3f("u_LightColor", glm::vec3(0.32));
        shader.SetUniform1f("u_Ambient", 0.65);
        shader.SetUniform1f("u_Specular", 0.8f);
        shader.SetUniform1f("u_Shininess", 15.0f);

        shader.UnBind();

#pragma endregion
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