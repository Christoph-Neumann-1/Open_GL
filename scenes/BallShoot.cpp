#include <Scene.hpp>
#include <Shader.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Data.hpp>
#include <Input.hpp>
#include <Logger.hpp>

using namespace GL;

class BallShoot : public Scene
{
    Shader shader{ROOT_Directory + "/shader/Default.vs", ROOT_Directory + "/shader/Default.fs"};

    uint vb, va;

    glm::mat4 ortho;

    glm::vec2 b_pos{0.0f, 0.0f};
    glm::vec2 b_vel;
    const glm::vec4 b_color{1, 0, 0, 1};

    glm::vec2 vertices[32 + 2];

    float xdiameter, ydiameter;

    uint movement;
    std::atomic_bool is_moving{false};

    InputHandler::MouseCallback mouse_callback{*loader->GetWindow().inputptr, GLFW_MOUSE_BUTTON_LEFT, InputHandler::Action::ReleasePress, &BallShoot::DragMouse, this};

    void ComputeVertices()
    {
        vertices[0] = {0, 0};
        for (uint i = 1; i <= 32; i++)
        {
            vertices[i] = {0.08f * cos(i * 2 * M_PI / 32),
                           0.08f * sin(i * 2 * M_PI / 32)};
        }
        vertices[32 + 1] = {0.08f * cos(2 * M_PI / 32),
                            0.08f * sin(2 * M_PI / 32)};
    }

    glm::dvec2 mouse_start, mouse_end;

    void DragMouse(int action)
    {
        if (is_moving)
            return;

        if (action == GLFW_PRESS)
        {
            glfwGetCursorPos(loader->GetWindow(), &mouse_start.x, &mouse_start.y);
            return;
        }

        glfwGetCursorPos(loader->GetWindow(), &mouse_end.x, &mouse_end.y);

        glm::vec2 movevec = (mouse_end - mouse_start);
        movevec.x =movevec.x/ loader->GetWindow().GetWidth() * xdiameter;
        movevec.y = movevec.y/loader->GetWindow().GetHeigth() * ydiameter;
        movevec.y *= -1;

        movement=RegisterFunc(CallbackType::Update,&BallShoot::Fly,this);
        b_vel = movevec*0.5f;
    }

    void Render()
    {
        shader.Bind();

        shader.SetUniform4f("u_Color", b_color);
        shader.SetUniformMat4f("u_MVP", ortho * glm::translate(glm::mat4(1), glm::vec3(b_pos, 0)));

        glBindVertexArray(va);

        glDrawArrays(GL_TRIANGLE_FAN, 0, 32 + 2);

        shader.UnBind();
    }

    void Fly()
    {
        auto dt = loader->GetTimeInfo().UpdateInterval();
        b_pos+=b_vel*dt;
    }

public:
    BallShoot(SceneLoader *_loader) : Scene(_loader)
    {
        glGenVertexArrays(1, &va);
        glBindVertexArray(va);
        glGenBuffers(1, &vb);
        glBindBuffer(GL_ARRAY_BUFFER, vb);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

        ComputeVertices();

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        RegisterFunc(CallbackType::Render, &BallShoot::Render, this);

        auto &window = loader->GetWindow();

        if (window.GetWidth() > window.GetHeigth())
        {
            ortho = glm::ortho(-(float)window.GetWidth() / window.GetHeigth(), (float)window.GetWidth() / window.GetHeigth(), -1.0f, 1.0f);
            xdiameter = (float)window.GetWidth() / window.GetHeigth();
            ydiameter = 1.0f;
        }
        else
        {
            ortho = glm::ortho(-1.0f, 1.0f, -(float)window.GetHeigth() / window.GetWidth(), (float)window.GetHeigth() / window.GetWidth());
            xdiameter = 1.0f;
            ydiameter = (float)window.GetHeigth() / window.GetWidth();
        }
    }
    ~BallShoot()
    {
        glDeleteBuffers(1, &vb);
        glDeleteVertexArrays(1, &va);
        RemoveFunctions();
    }
};

SCENE_LOAD_FUNC(BallShoot)