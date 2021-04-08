#include <Scene.hpp>
#include <imgui/imgui.h>
#include <SceneLoader.hpp>
#include <Data.hpp>
#include <glm/glm.hpp>
#include <Quad.hpp>
#include <Renderer.hpp>
#include <glm/gtc/matrix_transform.hpp>

class QuadScene final : public Scene
{
    static void OnResize(void *_this_, void *)
    {
        auto _this = reinterpret_cast<QuadScene *>(_this_);
        _this->renderer.proj = _this->loader->proj;
    }
    using cbt = CallbackHandler::CallbackType;

    SceneLoader *loader;
    Quad quad;
    Renderer renderer;

    glm::vec2 vertices[4];

    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));
    glm::mat4 model = glm::translate(glm::mat4(1.0f), {0, 0, 0});
    static void Render(void *, void *);
    static void Interface(void *, void *);

public:
    explicit QuadScene(SceneLoader *loader);
    ~QuadScene();
};

QuadScene::QuadScene(SceneLoader *loader) : loader(loader), quad(true)
{
    loader->callbackhandler->Register(cbt::Render, this, Render);
    loader->callbackhandler->Register(cbt::ImGuiRender, this, Interface);
    loader->callbackhandler->Register(cbt::OnWindowResize2, this, OnResize);
    float color[] = {1, 0.5, 0, 1};

    quad[0] = {10.0f, 10.0f};
    quad[1] = {100.0f, 20.0f};
    quad[2] = {150.0f, 200.0f};
    quad[3] = {10.0f, 150.0f};

    vertices[0] = {10.0f, 10.0f};
    vertices[1] = {100.0f, 20.0f};
    vertices[2] = {150.0f, 200.0f};
    vertices[3] = {10.0f, 150.0f};

    quad.Update(color);

    renderer.proj = loader->proj;
    renderer.view = view;
}
QuadScene::~QuadScene()
{
    loader->callbackhandler->RemoveAll(this);
}

void QuadScene::Render(void *_this_, void *)
{
    auto _this = reinterpret_cast<QuadScene *>(_this_);
    _this->renderer.Draw(&_this->quad, _this->model);
}

void QuadScene::Interface(void *_this_, void *)
{
    auto _this = reinterpret_cast<QuadScene *>(_this_);
    ImGui::Begin("Quad");

    for (int i = 0; i < 4; i++)
    {
        ImGui::SliderFloat2((std::string("V") + std::to_string(i)).c_str(), (float *)&_this->vertices[i], -100, 1000);
        _this->quad[i] = _this->vertices[i];
    }
    ImGui::End();

    _this->quad.Update();
}

extern "C" Scene *_INIT_(SceneLoader *loader)
{
    return new QuadScene(loader);
}

extern "C" void _EXIT_(Scene *scene)
{
    delete scene;
}