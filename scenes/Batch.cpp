//I used this file for trying out a few new ideas. It will be deleted soon.

#include <Scene.hpp>
#include <ModuleLoader.hpp>
#include <glm/glm.hpp>
#include <Data.hpp>

class Batch final : public GL::Scene
{
    void *handle;
    GL::ModuleLoader mod;
    void (*DrawFunc)(void *, glm::mat4);

    void Render()
    {
        DrawFunc(handle, glm::mat4(1.0f));
    }

public:
    Batch(GL::SceneLoader *_loader) : Scene(_loader), mod(ROOT_Directory + "/modules/bin/Primitives/QuadBatch.module")
    {
        handle = mod.RETRIEVE(void *, SetupBatch, uint, bool)(1, true);
        glm::vec3 vertices[6]{
            {-0.5, -0.5, 0},
            {0, -0.5, 0},
            {0, 0, 0},
            {0, 0, 0},
            {-0.5, 0, 0},
            {-0.5, -0.5, 0},
        };

        glm::vec4 colors[6]{
            {1, 0, 1, 1},
            {1, 0, 1, 1},
            {1, 0, 1, 1},
            {1, 0, 1, 1},
            {1, 0, 1, 1},
            {1, 0, 1, 1},
        };

        mod.RETRIEVE(void, AddQuad, void *, glm::vec3 *, glm::vec4 *)(handle, vertices, colors);
        mod.RETRIEVE(void, FillData, void *)(handle);
        DrawFunc = mod.RETRIEVE(void, Draw, void *, glm::mat4);

        RegisterFunc(GL::CallbackType::Render, &Batch::Render, this);
    }
    ~Batch()
    {
        RemoveFunctions();
    }
};

SCENE_LOAD_FUNC(Batch)