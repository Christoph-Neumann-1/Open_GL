#include <Scene.hpp>

using namespace GL;
class t_Scene final : public Scene
{
public:
    t_Scene(SceneLoader *_loader) : Scene(_loader)
    {
        RegisterFunc([&]() {}, CallbackType::PreUpdate);
    }
    ~t_Scene()
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