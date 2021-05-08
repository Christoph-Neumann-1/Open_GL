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
    CircleScene(SceneLoader *_loader) : Scene(_loader), circle_mod(ROOT_Directory + "/modules/bin/Primitives/Circle.module")
    {
        circle = circle_mod.RETRIEVE(void *, CreateCircle, float, uint)(0.5, 6);
        float color[4]{1, 0.5, 0.8, 1};
        circle_mod.RETRIEVE(void, SetColor, void *, float *)(circle, color);
        Draw = circle_mod.RETRIEVE(void, Draw, void *, glm::mat4);
        RegisterFunc(std::bind(&CircleScene::Render, this), CallbackType::Render);
    }
    ~CircleScene()
    {
        circle_mod.RETRIEVE(void, DeleteCircle, void *)(circle);
    }
};

SCENE_LOAD_FUNC(CircleScene)