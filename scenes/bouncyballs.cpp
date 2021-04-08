#include <Scene.hpp>
#include <imgui/imgui.h>
#include <SceneLoader.hpp>
#include <Data.hpp>
#include <Circle.hpp>
#include <Renderer.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <random>
#include <vector>

const float radius = 10;
const float G = 60;
const float color[4] = {0, 1, 0.8, 1};
const u_int segcount = 16;
const u_int n_balls = 250;
const float startv = 60;

const float force = 400;
const float friction = 3.5;

const float mouseforce = 150;

using cbt = CallbackHandler::CallbackType;

class Ball;

class BallHandle : public Scene
{
    static void OnResize(void *_this_, void *)
    {
        auto _this = reinterpret_cast<BallHandle *>(_this_);
        _this->renderer.proj = _this->loader->proj;
    }

    SceneLoader *loader;
    std::vector<Ball> balls;
    Renderer renderer;

    static void Render(void *, void *);
    static void Update(void *, void *);

public:
    BallHandle(SceneLoader *loader)
        : loader(loader)
    {
        renderer.proj = loader->proj;
        renderer.view = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));

        loader->callbackhandler->Register(CallbackHandler::CallbackType::Render, this, Render);
        loader->callbackhandler->Register(CallbackHandler::CallbackType::Render, this, Render);
        loader->callbackhandler->Register(CallbackHandler::CallbackType::Update, this, Update);
        loader->callbackhandler->Register(CallbackHandler::CallbackType::OnWindowResize2, this, OnResize);

        balls.reserve(n_balls);
        for (u_int i = 0; i < n_balls; i++)
        {
            glm::vec2 position = {rand() % loader->window.x, rand() % loader->window.y};
            glm::vec2 velocity = {(float)rand() / RAND_MAX * 2 * startv - startv, (float)rand() / RAND_MAX * 2 * startv - startv};
            balls.emplace_back(position, velocity, this);
        }
    }
    ~BallHandle()
    {
        loader->callbackhandler->RemoveAll(this);
    }
};

struct Ball
{
    BallHandle *handle;
    Circle circle;
    glm::mat4 model = glm::translate(glm::mat4(1.0f), {400, 400, 0});
    glm::vec2 velocity;

    Ball(glm::vec2 position, glm::vec2 velocity, BallHandle *_handle)
        : handle(_handle), circle(radius, segcount), model(glm::translate(glm::mat4(1.0f), glm::vec3(position, 0))), velocity(velocity)
    {
        circle.SetColor(color);
    }
};

void BallHandle::Render(void *_this_, void *)
{
    auto _this = reinterpret_cast<BallHandle *>(_this_);
    for (auto &ball : _this->balls)
    {
        _this->renderer.Draw(&ball.circle, ball.model);
    }
}

void BallHandle::Update(void *_this_, void *)
{
    auto _this = reinterpret_cast<BallHandle *>(_this_);
    float dt = _this->loader->callbackhandler->deltatime_update;
    for (auto &ball : _this->balls)
    {
        ball.velocity.y -= G * dt;
        float v = sqrtf(ball.velocity.x * ball.velocity.x + ball.velocity.y * ball.velocity.y);

        ball.velocity *= std::clamp((v - friction * dt), 0.0f, MAXFLOAT) / v;

        ball.model[3][0] += ball.velocity.x * dt;
        ball.model[3][1] += ball.velocity.y * dt;
        if (ball.model[3][1] < radius)
        {
            ball.model[3][1] = radius;
            ball.velocity.y *= -1;
        }
        else if (ball.model[3][1] > _this->loader->window.y - radius)
        {
            ball.model[3][1] = _this->loader->window.y - radius;
            ball.velocity.y *= -1;
        }
        if (ball.model[3][0] < radius)
        {
            ball.model[3][0] = radius;
            ball.velocity.x *= -1;
        }
        else if (ball.model[3][0] > _this->loader->window.x - radius)
        {
            ball.model[3][0] = _this->loader->window.x - radius;
            ball.velocity.x *= -1;
        }
        for (auto &ball2 : _this->balls)
        {
            if (&ball == &ball2)
                continue;

            glm::vec2 vec = {ball2.model[3][0] - ball.model[3][0], ball2.model[3][1] - ball.model[3][1]};
            float length = sqrtf(vec.x * vec.x + vec.y * vec.y);
            auto f = (length < 2.5 * radius) * force * dt * vec / length;
            ball.velocity -= f;
        }
    }

    if (glfwGetMouseButton(_this->loader->window, GLFW_MOUSE_BUTTON_LEFT))
    {
        double x;
        double y;
        float f_dt = mouseforce * dt;
        glfwGetCursorPos(_this->loader->window, &x, &y);
        y = _this->loader->window.y - y;
        for (auto &ball : _this->balls)
        {
            glm::vec2 vec = {x - ball.model[3][0], y - ball.model[3][1]};
            float length = sqrtf(vec.x * vec.x + vec.y * vec.y);
            auto force = f_dt / length * length * vec / length;
            ball.velocity += force;
        }
    }
    else if (glfwGetMouseButton(_this->loader->window, GLFW_MOUSE_BUTTON_RIGHT))
    {
        double x;
        double y;
        float f_dt = mouseforce * dt;
        glfwGetCursorPos(_this->loader->window, &x, &y);
        y = _this->loader->window.y - y;
        for (auto &ball : _this->balls)
        {
            glm::vec2 vec = {x - ball.model[3][0], y - ball.model[3][1]};
            float length = sqrtf(vec.x * vec.x + vec.y * vec.y);
            auto force = f_dt / length * length * vec / length;
            ball.velocity -= force;
        }
    }
}

extern "C" Scene *_INIT_(SceneLoader *loader)
{
    return new BallHandle(loader);
}

extern "C" void _EXIT_(Scene *scene)
{
    delete scene;
}
