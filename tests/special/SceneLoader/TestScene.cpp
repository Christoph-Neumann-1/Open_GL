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

SCENE_LOAD_FUNC(t_Scene)