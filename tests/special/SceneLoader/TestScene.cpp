#include <Scene.hpp>

using namespace GL;
class t_Scene final : public Scene
{
public:
    t_Scene(SceneLoader *_loader) : Scene(_loader)
    {
        loader->GetCallback().GetList(CallbackType::PreUpdate).Add([&]() { callback_id = callback_id; }, callback_id);
    }
    ~t_Scene()
    {
    }

    void PrepareUnload() final override
    {
        loader->GetCallback().RemoveAll(callback_id);
    }

    void Terminate() final override
    {
        loader->GetCallback().RemoveAll(callback_id);
    }
};

extern "C"
{
    Scene *_LOAD_(SceneLoader *loader)
    {
        return new t_Scene(loader);
    }
}