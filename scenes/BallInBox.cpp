#include <Scene.hpp>
#include <ModelLoader.hpp>
#include <Shader.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Data.hpp>
#include <Camera/Camera3D.hpp>
#include <Camera/Fplocked.hpp>

using namespace GL;

class BallInBox : public Scene
{
    Model ball{ROOT_Directory + "/res/Models/sphere.obj"};
    Shader shader{ROOT_Directory + "/shader/Default.vs", ROOT_Directory + "/shader/Default.fs"};

    uint vb, va;

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

    glm::vec3 b_pos;
    glm::vec3 b_vel{0.5f, 0.2f, 0.3f};
    const glm::vec4 b_color{1.0f, 0.8f, 0.6f, 1};
    const float bradius = 0.07f;

    void RenderBox()
    {
        glBindVertexArray(va);

        fplocked.Update(loader->GetTimeInfo().RenderDeltaTime());

        shader.SetUniformMat4f("u_MVP", proj * camera.ComputeMatrix());

        for (int i = 0; i < 6; i++)
        {
            shader.SetUniform4f("u_Color", side_colors[i / 2]);
            glDrawArrays(GL_TRIANGLES, 6 * i, 6);
        }

        glBindVertexArray(0);
    }

    //TODO render trail
    void Render()
    {
        shader.Bind();
        RenderBox();

        shader.SetUniform4f("u_Color", b_color);
        shader.SetUniformMat4f("u_MVP", proj * camera.ComputeMatrix() * glm::scale(glm::translate(glm::mat4(1),b_pos), {bradius, bradius, bradius}));

        ball.Draw(shader);
        shader.UnBind();
    }

    void CollideWithWalls()
    {

        for (int i = 0; i < 3; i++)
        {
            if (b_pos[i] + bradius > 1 || b_pos[i] - bradius < -1)
            {
                b_vel[i] = -b_vel[i];
            }
        }
    }

    void Update()
    {
        auto dt = loader->GetTimeInfo().UpdateInterval();
        CollideWithWalls();
        b_pos += b_vel * dt;
    }

public:
    BallInBox(SceneLoader *_loader) : Scene(_loader)
    {
        glGenVertexArrays(1, &va);
        glBindVertexArray(va);
        glGenBuffers(1, &vb);
        glBindBuffer(GL_ARRAY_BUFFER, vb);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glBufferData(GL_ARRAY_BUFFER, sizeof(sides), &sides, GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        RegisterFunc(CallbackType::Render, &BallInBox::Render, this);
        RegisterFunc(CallbackType::Update, &BallInBox::Update, this);

        GetFlag("hide_menu") = true;
    }
    ~BallInBox()
    {
        glDeleteBuffers(1, &vb);
        glDeleteVertexArrays(1, &va);
        RemoveFunctions();
    }
};

SCENE_LOAD_FUNC(BallInBox)