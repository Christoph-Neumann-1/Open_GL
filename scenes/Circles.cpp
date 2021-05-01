#include <Scene.hpp>
#include <ModuleLoader.hpp>
#include <Data.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace GL;

class CircleScene final : public Scene
{
    void *circle;
    ModuleLoader circle_mod;
    void (*Draw)(void *, glm::mat4);

    void Render()
    {
        Draw(circle, glm::mat4(1.0f));
    }

public:
    CircleScene(SceneLoader *_loader) : Scene(_loader), circle_mod(ROOT_Directory + "/modules/bin/Circle.module")
    {
        circle = circle_mod.RETRIEVE(void *, CreateCircle, float, uint)(0.5, 6);
        float color[4]{1, 0.5, 0.8, 1};
        circle_mod.RETRIEVE(void, SetColor, void *, float *)(circle, color);
        Draw = circle_mod.RETRIEVE(void, Draw, void *, glm::mat4);
        loader->GetCallback().GetList(CallbackType::Render).Add(std::bind(&CircleScene::Render, this));
    }
    ~CircleScene()
    {
        circle_mod.RETRIEVE(void, DeleteCircle, void *)(circle);
    }
};

extern "C" Scene *_LOAD_(SceneLoader *loader)
{
    return new CircleScene(loader);
}