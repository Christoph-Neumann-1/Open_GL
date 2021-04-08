#include <Scene.hpp>
#include <imgui/imgui.h>
#include <SceneLoader.hpp>
#include <Data.hpp>
#include <Circle.hpp>
#include <Renderer.hpp>
#include <glm/gtc/matrix_transform.hpp>

class CircleScene final : public Scene
{
    static void OnResize(void *_this_, void *)
    {
        auto _this = reinterpret_cast<CircleScene *>(_this_);
        _this->renderer.proj = _this->loader->proj;
    }
    using cbt = CallbackHandler::CallbackType;

    SceneLoader *loader;
    Circle circle;
    Renderer renderer;
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));
    glm::mat4 model = glm::translate(glm::mat4(1.0f), {0, 0, 0});
    static void Render(void *, void *);

public:
    explicit CircleScene(SceneLoader *loader);
    ~CircleScene();
};

CircleScene::CircleScene(SceneLoader *loader) : loader(loader), circle(100, 50, {500, 400})
{
    loader->callbackhandler->Register(cbt::Render, this, Render);
    loader->callbackhandler->Register(cbt::OnWindowResize2, this, OnResize);
    float color[] = {1, 0.5, 0, 1};

    circle.SetColor(color);

    renderer.proj = loader->proj;
    renderer.view = view;
}
CircleScene::~CircleScene()
{
    loader->callbackhandler->RemoveAll(this);
}

void CircleScene::Render(void *_this_, void *)
{
    auto _this = reinterpret_cast<CircleScene *>(_this_);
    _this->renderer.Draw(&_this->circle, _this->model);
}

extern "C" Scene *_INIT_(SceneLoader *loader)
{
    return new CircleScene(loader);
}

extern "C" void _EXIT_(Scene *scene)
{
    delete scene;
}