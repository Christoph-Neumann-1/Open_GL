#include <Scene.hpp>
#include <ModelLoader.hpp>
#include <Shader.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Data.hpp>
#include <Camera/Camera3D.hpp>
#include <Camera/Fplocked.hpp>
#include <random>
#include <Input.hpp>
#include <imgui/imgui.h>
#include <Buffer.hpp>
#include <VertexArray.hpp>

using namespace GL;

const uint natoms = 180;
const float initial_velocity = 0.4f;
const float dt_factor = 0.8f;
const float physics_rate = 1000;

const float attraction = 150;
const float equal_dist = 0.075f;
const float G = 0.2f;

class AtomsSim : public Scene
{
    Model atom_model{ROOT_Directory + "/res/Models/sphere.obj"};
    Shader shader{ROOT_Directory + "/shader/Atoms.vs", ROOT_Directory + "/shader/Atoms.fs"};
    Shader wall_shader{ROOT_Directory + "/shader/Default.vs", ROOT_Directory + "/shader/Default.fs"};

    bool mouse_captured{true};

    void MouseCapture(int)
    {
        mouse_captured = !mouse_captured;
        if (mouse_captured)
            camera.LockMouse(loader->GetWindow());
        else
            camera.UnlockMouse(loader->GetWindow());
    }

    InputHandler::KeyCallback mouse_capture_m{*loader->GetWindow().inputptr, glfwGetKeyScancode(GLFW_KEY_M), InputHandler::Action::Press, &AtomsSim::MouseCapture, this};

    std::atomic_bool should_wait = 0;
    std::atomic_bool is_waiting = 0;

    Buffer vb;
    VertexArray va;

    float KinE;
    float PotE;

    glm::vec3 bar[6]{
        glm::vec3(4.0f, 0.0f, 0.0f),
        glm::vec3(4.0f, 1.0f, 0.0f),
        glm::vec3(2.0f, 1.0f, 0.0f),
        glm::vec3(2.0f, 1.0f, 0.0f),
        glm::vec3(2.0f, 0.0f, 0.0f),
        glm::vec3(4.0f, 0.0f, 0.0f)};

    glm::mat4 ortho_ui = glm::ortho(0.0f, 100.0f, 0.0f, 10.0f * natoms, -1.0f, 1.0f);

    Buffer offsets;

    float scale = 2;

    uint collsions;
    float collision_time_span = 3;
    float collision_time_span_counter = 0;

    float pressure = 0;

    struct Atom
    {
        glm::vec3 pos;
        glm::vec3 vel;

        Atom(glm::vec3 p, glm::vec3 v) : pos(p), vel(v) {}
    };

    std::vector<Atom> atoms;

    Camera3D camera{{0, 0, 1}};
    Fplocked fplocked{camera, loader->GetWindow(), 1};

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
    const float bradius = 0.04f;

    void RenderBox()
    {
        wall_shader.SetUniformMat4f("u_MVP", proj * camera.ComputeMatrix());

        for (int i = 0; i < 6; i++)
        {
            wall_shader.SetUniform4f("u_Color", side_colors[i / 2]);
            glDrawArrays(GL_TRIANGLES, 6 * i, 6);
        }
    }

    void FillBuffer()
    {
        glm::vec3 buffer[natoms];
        for (uint i = 0; i < natoms; i++)
        {
            buffer[i] = atoms[i].pos / scale;
        }
        offsets.Bind(GL_ARRAY_BUFFER);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * natoms, buffer);
        Buffer::Unbind(GL_ARRAY_BUFFER);
    }

    void RenderBar()
    {

        wall_shader.SetUniform4f("u_Color", {1, 0, 0, 1});
        wall_shader.SetUniformMat4f("u_MVP", ortho_ui * glm::scale(glm::mat4(1), glm::vec3(1, KinE, 1)));
        glDrawArrays(GL_TRIANGLES, 36, 6);
        wall_shader.SetUniform4f("u_Color", {0, 1, 0, 1});
        wall_shader.SetUniformMat4f("u_MVP", ortho_ui * glm::translate(glm::scale(glm::mat4(1), glm::vec3(1, PotE, 1)), glm::vec3(3, 0, 0)));
        glDrawArrays(GL_TRIANGLES, 36, 6);
        wall_shader.SetUniform4f("u_Color", {0, 0, 1, 1});
        wall_shader.SetUniformMat4f("u_MVP", ortho_ui * glm::translate(glm::scale(glm::mat4(1), glm::vec3(1, PotE + KinE, 1)), glm::vec3(6, 0, 0)));
        glDrawArrays(GL_TRIANGLES, 36, 6);
    }

    void Render()
    {
        if (mouse_captured)
            fplocked.Update(loader->GetTimeInfo().RenderDeltaTime());

        wall_shader.Bind();
        va.Bind();
        RenderBox();
        RenderBar();
        VertexArray::Unbind();
        shader.Bind();

        shader.SetUniformMat4f("u_V", camera.ComputeMatrix());
        shader.SetUniformMat4f("u_M", glm::scale(glm::mat4(1), glm::vec3(bradius / scale)));

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
                if (atoms[i].pos[j] + bradius > 1 * scale || atoms[i].pos[j] - bradius < -1 * scale)
                {
                    atoms[i].vel[j] = -atoms[i].vel[j];
                    atoms[i].pos[j] = std::clamp(atoms[i].pos[j], -1 * scale + bradius, 1 * scale - bradius);
                    collsions++;
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
            atom.vel.y -= G * dt;
        }
    }

    void UpdatePositions(float dt)
    {
        for (auto &atom : atoms)
        {
            atom.pos += atom.vel * dt;
        }
    }

    void UpdateEnergie()
    {
        KinE = 0;
        PotE = 0;
        for (auto &atom : atoms)
        {
            KinE += 0.5f * glm::length2(atom.vel);
            PotE += (atom.pos.y + 1 * scale) * G;
        }
    }

    void Update()
    {
        if (should_wait)
        {
            is_waiting = true;
            //TODO use condition variable
            while (should_wait)
            {
            }
        }
        static float dt = loader->GetTimeInfo().UpdateInterval() * dt_factor;
        CollideWithWalls();
        ComputeForces(dt);
        ComputeGravity(dt);
        UpdatePositions(dt);
        UpdateEnergie();
        collision_time_span_counter += dt;
        if (collision_time_span_counter > collision_time_span)
        {
            collision_time_span_counter = 0;
            pressure = collsions / collision_time_span;
            collsions = 0;
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
                glm::vec3{(float)rand() / (float)RAND_MAX * 1.8 * scale - 0.9f * scale, (float)rand() / (float)RAND_MAX * 1.8 * scale - 0.9f * scale, (float)rand() / (float)RAND_MAX * 1.8 * scale - 0.9f * scale},
                glm::vec3{(float)rand() / (float)RAND_MAX * initial_velocity - initial_velocity / 2, (float)rand() / (float)RAND_MAX * initial_velocity - initial_velocity / 2, (float)rand() / (float)RAND_MAX * initial_velocity - initial_velocity / 2});
        }
    }

    InputHandler::KeyCallback pluskey{*loader->GetWindow().inputptr, glfwGetKeyScancode(GLFW_KEY_KP_ADD), InputHandler::Action::Press, &AtomsSim::ModifyVelocity, this, 1.1f};
    InputHandler::KeyCallback minuskey{*loader->GetWindow().inputptr, glfwGetKeyScancode(GLFW_KEY_KP_SUBTRACT), InputHandler::Action::Press, &AtomsSim::ModifyVelocity, this, 0.9f};

    void ModifyVelocity(float change_percent, int)
    {
        should_wait = true;
        while (!is_waiting)
            ;

        for (auto &atom : atoms)
        {
            atom.vel *= change_percent;
        }
        is_waiting = false;
        should_wait = false;
    }

    void UpdateUI()
    {
        ImGui::Begin("Info");
        ImGui::Text("Kinetic energy: %f", KinE);
        ImGui::Text("Potential energy: %f", PotE);
        ImGui::Text("Total energy: %f", PotE + KinE);
        ImGui::Text("Pressure: %f", pressure);
        ImGui::End();
    }

public:
    AtomsSim(SceneLoader *_loader) : Scene(_loader)
    {
#pragma region Buffers
        va.Bind();
        vb.Bind(GL_ARRAY_BUFFER);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glBufferData(GL_ARRAY_BUFFER, sizeof(sides) + sizeof(bar), nullptr, GL_DYNAMIC_DRAW);

        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(sides), &sides);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(sides), sizeof(bar), &bar);

        offsets.Bind(GL_ARRAY_BUFFER);

        glBufferData(GL_ARRAY_BUFFER, natoms * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);
        VertexBufferLayout layout;
        layout.stride = sizeof(glm::vec3);
        layout.attributes.push_back({GL_FLOAT, 3, 0});
        atom_model.AddInstanceBuffer(layout, offsets);
        Buffer::Unbind(GL_ARRAY_BUFFER);

#pragma endregion

        atoms.reserve(natoms);
        Setup();

        RegisterFunc(CallbackType::Render, &AtomsSim::Render, this);
        RegisterFunc(CallbackType::Update, &AtomsSim::Update, this);
        RegisterFunc(CallbackType::ImGuiRender, &AtomsSim::UpdateUI, this);

        GetFlag("hide_menu") = true;

        loader->GetTimeInfo().SetUpdateInterval(1 / physics_rate);

#pragma region Uniforms
        shader.Bind();
        shader.SetUniform3f("u_Color", b_color);
        shader.SetUniformMat4f("u_P", proj);

        shader.SetUniform3f("u_LightDirection", glm::normalize(glm::vec3(0, -1, -0.3)));
        shader.SetUniform3f("u_LightColor", glm::vec3(0.32));
        shader.SetUniform1f("u_Ambient", 0.65);
        shader.SetUniform1f("u_Specular", 0.8f);
        shader.SetUniform1f("u_Shininess", 15.0f);

        shader.UnBind();

        camera.UnlockMouse(loader->GetWindow());

#pragma endregion
    }
    ~AtomsSim()
    {
        RemoveFunctions();
        loader->GetTimeInfo().SetUpdateInterval();
    }
};

SCENE_LOAD_FUNC(AtomsSim)