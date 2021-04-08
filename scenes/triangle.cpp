#include <Scene.hpp>
#include <imgui/imgui.h>
#include <SceneLoader.hpp>
#include <Data.hpp>
#include <glm/glm.hpp>
#include <Triangle.hpp>
#include <Renderer.hpp>
#include <glm/gtc/matrix_transform.hpp>

class TriangleScene final : public Scene
{
    static void OnResize(void *_this_, void *)
    {
        auto _this = reinterpret_cast<TriangleScene *>(_this_);
        _this->renderer.proj = _this->loader->proj;
    }
    using cbt = CallbackHandler::CallbackType;

    SceneLoader *loader;
    Triangle tri;
    Renderer renderer;

    glm::vec2 vertices[3];

    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));
    glm::mat4 model = glm::translate(glm::mat4(1.0f), {0, 0, 0});
    static void Render(void *, void *);
    static void Interface(void *, void *);

public:
    explicit TriangleScene(SceneLoader *loader);
    ~TriangleScene();
};

TriangleScene::TriangleScene(SceneLoader *loader) : loader(loader), tri(true)
{
    loader->callbackhandler->Register(cbt::Render, this, Render);
    loader->callbackhandler->Register(cbt::ImGuiRender, this, Interface);
    loader->callbackhandler->Register(cbt::OnWindowResize2, this, OnResize);
    float color[] = {1, 0.5, 0, 1};
    vertices[0] = {10.0f, 10.0f};
    vertices[1] = {100.0f, 20.0f};
    vertices[2] = {150.0f, 200.0f};

    tri.Update(color);

    renderer.proj = loader->proj;
    renderer.view = view;
}
TriangleScene::~TriangleScene()
{
    loader->callbackhandler->RemoveAll(this);
}

void TriangleScene::Render(void *_this_, void *)
{
    auto _this = reinterpret_cast<TriangleScene *>(_this_);
    _this->renderer.Draw(&_this->tri, _this->model);
}

void TriangleScene::Interface(void *_this_, void *)
{
    auto _this = reinterpret_cast<TriangleScene *>(_this_);
    ImGui::Begin("Triangle");

    for (int i = 0; i < 3; i++)
    {
        ImGui::SliderFloat2((std::string("V") + std::to_string(i)).c_str(), (float *)&_this->vertices[i], -100, 1000);
        _this->tri[i] = _this->vertices[i];
    }
    ImGui::End();

    _this->tri.Update();
}

extern "C" Scene *_INIT_(SceneLoader *loader)
{
    return new TriangleScene(loader);
}

extern "C" void _EXIT_(Scene *scene)
{
    delete scene;
}