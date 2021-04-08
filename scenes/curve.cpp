#include <Scene.hpp>
#include <imgui/imgui.h>
#include <SceneLoader.hpp>
#include <Data.hpp>
#include <glm/glm.hpp>
#include <Curve.hpp>
#include <Renderer.hpp>
#include <glm/gtc/matrix_transform.hpp>

//FIXME Not drawing everyting
class CurveScene final : public Scene
{
    using cbt = CallbackHandler::CallbackType;
    float vertfreq = 0.25;

    SceneLoader *loader;
    Curve curve;
    Renderer renderer;

    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));
    glm::mat4 model = glm::translate(glm::mat4(1.0f), {0, 0, 0});
    static void Render(void *, void *);

    float exp = 0;
    float fact = 1;
    float offx = 0;
    float offy = 0;
    float div = 1;
    static void Interface(void *, void *);

    void CalculatePositions(float exp, float mult, float offsetx, float offsety, float div);

    static void OnResize(void *_this_, void *)
    {
        auto _this = reinterpret_cast<CurveScene *>(_this_);
        _this->curve.Resize(_this->loader->window.x / _this->vertfreq);
        _this->CalculatePositions(_this->exp, _this->fact, _this->offx, _this->offy, _this->div);

        _this->renderer.proj = _this->loader->proj;
    }

public:
    explicit CurveScene(SceneLoader *loader);
    ~CurveScene();
};

CurveScene::CurveScene(SceneLoader *loader) : loader(loader), curve(loader->window.x / vertfreq, true)
{
    loader->callbackhandler->Register(cbt::Render, this, Render);
    loader->callbackhandler->Register(cbt::ImGuiRender, this, Interface);
    loader->callbackhandler->Register(cbt::OnWindowResize2, &renderer, OnResize, &loader->proj);
    loader->callbackhandler->Register(cbt::OnWindowResize2, this, OnResize);

    float color[] = {-5, 0.8, 0.9, 1};
    curve.UpdateColor(color);
    renderer.proj = loader->proj;
    renderer.view = view;
}
CurveScene::~CurveScene()
{
    loader->callbackhandler->RemoveAll(this);
    loader->callbackhandler->Remove(CallbackHandler::CallbackType::OnWindowResize2, {&renderer, OnResize, &loader->proj});
}

void CurveScene::Render(void *_this_, void *)
{
    auto _this = reinterpret_cast<CurveScene *>(_this_);
    _this->renderer.Draw(&_this->curve, _this->model);
}

void CurveScene::Interface(void *_this_, void *)
{
    auto _this = reinterpret_cast<CurveScene *>(_this_);
    ImGui::Begin("Curve");
    ImGui::SliderFloat("exponent", &_this->exp, -20, 20);
    ImGui::SliderFloat("factor", &_this->fact, -150, 150);
    ImGui::SliderFloat("offx", &_this->offx, -1000, 1000);
    ImGui::SliderFloat("offy", &_this->offy, -1000, 1000);
    ImGui::SliderFloat("divider", &_this->div, 0, 200);
    ImGui::End();
    _this->CalculatePositions(_this->exp, _this->fact, _this->offx, _this->offy, _this->div);
}

void CurveScene::CalculatePositions(float exp, float mult, float offsetx, float offsety, float div)
{
    for (int i = 0; i < loader->window.x / vertfreq; i++)
    {
        curve[i] = {vertfreq * i,
                    powf((vertfreq * (i + offsetx)) / div, exp) * mult + offsety};
    }
    curve.Update(10.0f);
}

extern "C" Scene *_INIT_(SceneLoader *loader)
{
    return new CurveScene(loader);
}

extern "C" void _EXIT_(Scene *scene)
{
    delete scene;
}