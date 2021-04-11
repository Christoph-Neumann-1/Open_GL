#include <Scene.hpp>
#include <imgui/imgui.h>
#include <SceneLoader.hpp>
#include <Data.hpp>
#include <Circle.hpp>
#include <Renderer.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <random>

const float radius = 10;
const float G = 60;
const float color[4] = {0, 1, 0.8, 1};
const u_int segcount = 16;
const u_int n_balls = 150;
const float startv = 60;

const float force = 450;
const float friction = 3.5;

const float mouseforce = 150;

using cbt = CallbackHandler::CallbackType;

class BallHandle : public Scene
{
    static void OnResize(void *_this_, void *)
    {
        auto _this = reinterpret_cast<BallHandle *>(_this_);
        _this->renderer.proj = _this->loader->proj;
        _this->circle.GetShader().SetUniformMat4f("u_VP", _this->renderer.view * _this->renderer.proj);
    }

    SceneLoader *loader;
    Renderer renderer;
    Circle circle;
    u_int transform_buffer;
    glm::vec2 ball_v[n_balls];

    static void Render(void *, void *);
    static void Update(void *, void *);

public:
    BallHandle(SceneLoader *loader)
        : loader(loader), circle(radius, segcount, {0, 0}, "/res/Shaders/instanced.glsl")
    {
        circle.SetColor(color);

        glGenBuffers(1, &transform_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, transform_buffer);
        glBufferData(GL_ARRAY_BUFFER, n_balls * 2 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, false, 2*sizeof(float), 0);
        glVertexAttribDivisor(1, 1);

        renderer.proj = loader->proj;
        renderer.view = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));
        circle.GetShader().SetUniformMat4f("u_VP", renderer.proj * renderer.view);

        loader->callbackhandler->Register(CallbackHandler::CallbackType::Render, this, Render);
        loader->callbackhandler->Register(CallbackHandler::CallbackType::Render, this, Render);
        loader->callbackhandler->Register(CallbackHandler::CallbackType::Update, this, Update);
        loader->callbackhandler->Register(CallbackHandler::CallbackType::OnWindowResize2, this, OnResize);

        glm::vec2 *buffer = (glm::vec2 *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        for (u_int i = 0; i < n_balls; i++)
        {
            buffer[i] = {rand() % loader->window.x, rand() % loader->window.y};
            ball_v[i]={(float)rand() / RAND_MAX * 2 * startv - startv, (float)rand() / RAND_MAX * 2 * startv - startv};
        }
        glUnmapBuffer(GL_ARRAY_BUFFER);
        circle.GetVertexArray().UnBind();
    }
    ~BallHandle()
    {
        loader->callbackhandler->RemoveAll(this);
    }
};

void BallHandle::Render(void *_this_, void *)
{
    auto _this = reinterpret_cast<BallHandle *>(_this_);
    _this->circle.GetVertexArray().Bind();
    _this->circle.GetIndexBuffer().Bind();
    _this->circle.GetShader().Bind();
    glDrawElementsInstanced(GL_TRIANGLES,segcount*3,GL_UNSIGNED_INT,nullptr,n_balls);
}

void BallHandle::Update(void *_this_, void *)
{
    auto _this = reinterpret_cast<BallHandle *>(_this_);
    float dt = _this->loader->callbackhandler->deltatime_update;
    glm::vec2 *ball_v = _this->ball_v;
    glBindBuffer(GL_ARRAY_BUFFER,_this->transform_buffer);
    glm::vec2 *ball_p = (glm::vec2 *)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
    for (u_int i = 0; i < n_balls; i++)
    {
        ball_v[i].y -= G * dt;
        float v = sqrtf(ball_v[i].x * ball_v[i].x + ball_v[i].y * ball_v[i].y);

        ball_v[i] *= std::clamp((v - friction * dt), 0.0f, 4000.0f) / v;

        ball_p[i].x += ball_v[i].x * dt;
        ball_p[i].y += ball_v[i].y * dt;
        if (ball_p[i].y < radius)
        {
            ball_p[i].y = radius;
            ball_v[i].y *= -1;
        }
        else if (ball_p[i].y > _this->loader->window.y - radius)
        {
            ball_p[i].y = _this->loader->window.y - radius;
            ball_v[i].y *= -1;
        }
        if (ball_p[i].x < radius)
        {
            ball_p[i].x = radius;
            ball_v[i].x *= -1;
        }
        else if (ball_p[i].x > _this->loader->window.x - radius)
        {
            ball_p[i].x = _this->loader->window.x - radius;
            ball_v[i].x *= -1;
        }
        for (u_int j=0;j<n_balls;j++)
        {
            if (i == j)
                continue;

            glm::vec2 vec = {ball_p[j].x - ball_p[i].x, ball_p[j].y - ball_p[i].y};
            float length = glm::length(vec);
            auto f = (length < 2.5 * radius) * force * dt * vec / length;
            ball_v[i] -= f;
        }
    }

    if (glfwGetMouseButton(_this->loader->window, GLFW_MOUSE_BUTTON_LEFT))
    {
        double x;
        double y;
        float f_dt = mouseforce * dt;
        glfwGetCursorPos(_this->loader->window, &x, &y);
        y = _this->loader->window.y - y;
        for (u_int i=0; i<n_balls;i++)
        {
            glm::vec2 vec = {x - ball_p[i].x, y - ball_p[i].y};
            float length = glm::length(vec);
            auto force = f_dt / length * length * vec / length;
            ball_v[i] += force;
        }
    }
    else if (glfwGetMouseButton(_this->loader->window, GLFW_MOUSE_BUTTON_RIGHT))
    {
        double x;
        double y;
        float f_dt = mouseforce * dt;
        glfwGetCursorPos(_this->loader->window, &x, &y);
        y = _this->loader->window.y - y;
        for (u_int i=0; i<n_balls;i++)
        {
            glm::vec2 vec = {x - ball_p[i].x, y - ball_p[i].y};
            float length = glm::length(vec);
            auto force = f_dt / length * length * vec / length;
            ball_v[i] -= force;
        }
    }
    glUnmapBuffer(GL_ARRAY_BUFFER);
}

extern "C" Scene *_INIT_(SceneLoader *loader)
{
    return new BallHandle(loader);
}

extern "C" void _EXIT_(Scene *scene)
{
    delete scene;
}
