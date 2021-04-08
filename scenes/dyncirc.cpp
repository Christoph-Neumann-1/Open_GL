#include <Scene.hpp>
#include <imgui/imgui.h>
#include <SceneLoader.hpp>
#include <Data.hpp>
#include <Circle.hpp>
#include <Renderer.hpp>
#include <glm/gtc/matrix_transform.hpp>

const float G = 100;

class Circle_Bounce final : public Scene
{
    static void OnResize(void *_this_, void *)
    {
        auto _this = reinterpret_cast<Circle_Bounce *>(_this_);
        _this->renderer.proj = _this->loader->proj;
    }
    using cbt = CallbackHandler::CallbackType;
    float radius = 50;

    SceneLoader *loader;
    Circle circle;
    Renderer renderer;
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));
    glm::mat4 model = glm::translate(glm::mat4(1.0f), {400, 400, 0});
    glm::vec2 velocity{200, -290};
    static void Render(void *, void *);
    static void Physics(void *, void *);

public:
    explicit Circle_Bounce(SceneLoader *loader);
    ~Circle_Bounce();
};

Circle_Bounce::Circle_Bounce(SceneLoader *loader) : loader(loader), circle(radius)
{
    loader->callbackhandler->Register(cbt::Render, this, Render);
    loader->callbackhandler->Register(cbt::Update, this, Physics);
    loader->callbackhandler->Register(cbt::OnWindowResize2, this, OnResize);
    float color[] = {1, 0.5, 0, 1};

    circle.SetColor(color);

    renderer.proj = loader->proj;
    renderer.view = view;
}
Circle_Bounce::~Circle_Bounce()
{
    loader->callbackhandler->RemoveAll(this);
}

void Circle_Bounce::Render(void *_this_, void *)
{
    auto _this = reinterpret_cast<Circle_Bounce *>(_this_);
    _this->renderer.Draw(&_this->circle, _this->model);
}

void Circle_Bounce::Physics(void *_this_, void *)
{
    auto _this = reinterpret_cast<Circle_Bounce *>(_this_);
    _this->velocity.y -= G * _this->loader->callbackhandler->deltatime_update;
    _this->model[3][1] += _this->velocity.y * _this->loader->callbackhandler->deltatime_update;
    _this->model[3][0] += _this->velocity.x * _this->loader->callbackhandler->deltatime_update;
    if (_this->model[3][1] < _this->radius)
    {
        _this->model[3][1] = _this->radius;
        _this->velocity.y *= -1;
    }
    if (_this->model[3][0] < _this->radius)
    {
        _this->model[3][0] = _this->radius;
        _this->velocity.x *= -1;
    }
    else if (_this->model[3][0] > _this->loader->window.x - _this->radius)
    {
        _this->model[3][0] = _this->loader->window.x - _this->radius;
        _this->velocity.x *= -1;
    }
}

extern "C" Scene *_INIT_(SceneLoader *loader)
{
    return new Circle_Bounce(loader);
}

extern "C" void _EXIT_(Scene *scene)
{
    delete scene;
}