#include <Scene.hpp>
#include <Shader.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Data.hpp>
#include <Input.hpp>
#include <Logger.hpp>
#include <random>
#include <glm/gtx/rotate_vector.hpp>

//TODO: background image
//TODO: better colors
//TODO: postprocessing

using namespace GL;

class Breakout : public Scene
{
#pragma region Variables
    Shader shader{ROOT_Directory + "/shader/Default.vs", ROOT_Directory + "/shader/Default.fs"};
    Shader bshader{ROOT_Directory + "/shader/Boxes.vs", ROOT_Directory + "/shader/Boxes.fs"};

    uint vb, va;
    uint instance_buffer;

    glm::mat4 ortho;

    glm::vec2 b_pos;
    glm::vec2 b_vel;
    const glm::vec4 b_color{rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, 1};
    const float bradius = 0.045f;

    glm::vec2 vertices[32 + 2];

    glm::vec2 bounds;

    int points = 0;

    float barx = 0.0f;

    float bary = -0.9f;

    const glm::vec2 bar_size{0.4f, 0.04f};

    const glm::vec2 square_vertices[6]{
        {-1.0f, -1.0f},
        {1.0f, -1.0f},
        {1.0f, 1.0f},
        {1.0f, 1.0f},
        {-1.0f, 1.0f},
        {-1.0f, -1.0f}};
    const float speedincrease = 0.025f;

    const float centerLineWidth = 0.005f;

    struct BufferElement
    {
        glm::vec2 pos;
        glm::vec4 color;
        BufferElement(glm::vec2 p, glm::vec4 c) : pos(p), color(c) {}
    };

    std::vector<BufferElement> boxes;

    const uint rows = 9;
    const uint cols = 10;

    const glm::vec2 box_size{2.0f / cols, 1.0f / rows};

#pragma endregion

    void ComputeVertices()
    {
        vertices[0] = {0, 0};
        for (uint i = 1; i <= 32; i++)
        {
            vertices[i] = {bradius * cos(i * 2 * M_PI / 32),
                           bradius * sin(i * 2 * M_PI / 32)};
        }
        vertices[32 + 1] = {bradius * cos(2 * M_PI / 32),
                            bradius * sin(2 * M_PI / 32)};
    }

    void DragMouse()
    {
        glm::dvec2 mpos;

        glfwGetCursorPos(loader->GetWindow(), &mpos.x, &mpos.y);

        barx = mpos.x / loader->GetWindow().GetWidth() * 2 * bounds.x - bounds.x;
    }

    void UpdateBuffer()
    {
        glBindBuffer(GL_ARRAY_BUFFER, instance_buffer);

        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(BufferElement) * boxes.size(), boxes.data());

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void Render()
    {
        DragMouse();

        UpdateBuffer();

        shader.Bind();

        shader.SetUniform4f("u_Color", b_color);
        shader.SetUniformMat4f("u_MVP", ortho * glm::translate(glm::mat4(1), glm::vec3(b_pos, 0)));

        glBindVertexArray(va);

        glDrawArrays(GL_TRIANGLE_FAN, 0, 32 + 2);

        shader.SetUniform4f("u_Color", glm::vec4(1, 1, 1, 1));
        shader.SetUniformMat4f("u_MVP", ortho * glm::scale(glm::translate(glm::mat4(1), glm::vec3(barx, bary, 0)), glm::vec3(bar_size, 0)));

        glDrawArrays(GL_TRIANGLES, 34, 6);

        shader.SetUniform4f("u_Color", {1, 0, 0, 1});
        //Needed because opengl seems to draw only once if something already is at this z value
        shader.SetUniformMat4f("u_MVP", ortho * glm::scale(glm::translate(glm::mat4(1), glm::vec3(barx, bary, 0.001)), glm::vec3(centerLineWidth,bar_size.y, 0)));
        glDrawArrays(GL_TRIANGLES, 34, 6);


        bshader.Bind();

        bshader.SetUniformMat4f("u_MVP", ortho);
        bshader.SetUniformMat4f("scale_mat", glm::scale(glm::mat4(1), glm::vec3(0.5f * box_size, 0)));

        glDrawArraysInstanced(GL_TRIANGLES, 34, 6, boxes.size());

        shader.UnBind();
    }

    void BounceWalls()
    {
        for (int i = 0; i < 2; i++)
        {
            if (b_pos[i] + bradius > bounds[i] || b_pos[i] - bradius < -bounds[i])
            {
                b_vel[i] = -b_vel[i];
                points = 0;
            }
        }
    }

    void CollideBar()
    {
        if (b_pos.y - bradius < bary + bar_size.y && b_pos.x < barx + bar_size.x && b_pos.x > barx - bar_size.x)
        {
            float offset = (b_pos.x - barx) / bar_size.x;
            b_vel = glm::rotate(glm::vec2(0, glm::length(b_vel)+ speedincrease), glm::radians(-offset * 60));
            b_pos.y = bary + bar_size.y + bradius;
        }
    }

    void UpdatePosition(float dt)
    {
        b_pos += b_vel * dt;
    }

    void CollideBottom()
    {
        if (b_pos.y - bradius < -bounds.y)
        {
            Setup();
        }
    }

    bool circleRectCollision(glm::vec2 circlepos, float radius, glm::vec2 rectpos, glm::vec2 rectsize)
    {
        float testX = circlepos.x;
        float testY = circlepos.y;

        if (circlepos.x < rectpos.x)
            testX = rectpos.x; // test left edge
        else if (circlepos.x > rectpos.x + rectsize.x)
            testX = rectpos.x + rectsize.x; // right edge
        if (circlepos.y < rectpos.y)
            testY = rectpos.y; // top edge
        else if (circlepos.y > rectpos.y + rectsize.y)
            testY = rectpos.y + rectsize.y; // bottom edge

        float distX = circlepos.x - testX;
        float distY = circlepos.y - testY;
        float distance = sqrt((distX * distX) + (distY * distY));

        if (distance <= radius)
        {
            return true;
        }
        return false;
    }

    void CollideBoxes()
    {
        for (int i = boxes.size() - 1; i >= 0; i--)
        {
            auto &box = boxes[i];
            if (circleRectCollision(b_pos, bradius, box.pos - box_size / 2.0f, box_size))
            {
                //TODO fix corners
                if (b_pos.x > box.pos.x + box_size.x / 2.0f)
                {
                    b_vel.x = -b_vel.x;
                }
                else if (b_pos.x < box.pos.x - box_size.x / 2.0f)
                {
                    b_vel.x = -b_vel.x;
                }
                if (b_pos.y > box.pos.y + box_size.y / 2.0f)
                {
                    b_vel.y = -b_vel.y;
                }
                else if (b_pos.y < box.pos.y - box_size.y / 2.0f)
                {
                    b_vel.y = -b_vel.y;
                }
                boxes.erase(boxes.begin() + i);
            }
        }
    }

    void Fly()
    {
        auto dt = loader->GetTimeInfo().UpdateInterval();

        CollideBottom();
        BounceWalls();
        CollideBar();
        CollideBoxes();

        UpdatePosition(dt);
    }

    void Setup()
    {
        boxes.clear();
        for (uint i = 0; i < rows; i++)
            for (uint j = 0; j < cols; j++)
            {
                glm::vec2 pos{-1 + j * box_size.x + box_size.x / 2.0f, i * box_size.y + box_size.y / 2.0f};
                glm::vec4 color{rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, 1};
                boxes.push_back({pos, color});
            }
        b_pos = {0, -0.3};
        b_vel = {rand() / (float)RAND_MAX * 2 - 1, rand() / (float)RAND_MAX / 4 - 1};
    }

public:
    Breakout(SceneLoader *_loader) : Scene(_loader)
    {

#pragma region Buffers
        glGenVertexArrays(1, &va);
        glBindVertexArray(va);
        glGenBuffers(1, &vb);

        glBindBuffer(GL_ARRAY_BUFFER, vb);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

        ComputeVertices();

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(square_vertices), 0, GL_STATIC_DRAW);

        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(square_vertices), square_vertices);

        glGenBuffers(1, &instance_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, instance_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(BufferElement) * rows * cols, 0, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(BufferElement), 0);
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(BufferElement), (void *)sizeof(glm::vec2));
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glVertexAttribDivisor(1, 1);
        glVertexAttribDivisor(2, 1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

#pragma endregion

        Setup();

        RegisterFunc(CallbackType::Render, &Breakout::Render, this);
        RegisterFunc(CallbackType::Update, &Breakout::Fly, this);

        auto &window = loader->GetWindow();

        if (window.GetWidth() > window.GetHeigth())
        {
            ortho = glm::ortho(-(float)window.GetWidth() / window.GetHeigth(), (float)window.GetWidth() / window.GetHeigth(), -1.0f, 1.0f);
            bounds.x = (float)window.GetWidth() / window.GetHeigth();
            bounds.y = 1.0f;
        }
        else
        {
            ortho = glm::ortho(-1.0f, 1.0f, -(float)window.GetHeigth() / window.GetWidth(), (float)window.GetHeigth() / window.GetWidth());
            bounds.x = 1.0f;
            bounds.y = (float)window.GetHeigth() / window.GetWidth();
        }

        GetFlag("hide_menu") = true;

        loader->GetWindow().bgcolor = {0.0f, 0.0f, 0.0f, 1.0f};
    }
    ~Breakout()
    {
        glDeleteBuffers(1, &vb);
        glDeleteVertexArrays(1, &va);
        RemoveFunctions();
        loader->GetWindow().bgcolor = Window::defaultbg;
    }
};

SCENE_LOAD_FUNC(Breakout)