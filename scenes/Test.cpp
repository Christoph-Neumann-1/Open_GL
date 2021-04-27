#include <Scene.hpp>
#include <imgui/imgui.h>
#include <Logger.hpp>

using namespace GL;
class t_Scene final : public Scene
{
    void Imgui()
    {
        ImGui::Begin("Test");

        ImGui::End();
    }

public:
    t_Scene(SceneLoader *_loader) : Scene(_loader)
    {
        loader->GetCallback().GetList(CallbackType::ImGuiRender).Add(std::bind(&t_Scene::Imgui, this), callback_id);
        loader->GetCallback().GetList(CallbackType::OnWindowResize).Add([](){static Logger log; log("Resizing");}, callback_id);
        loader->GetCallback().GetList(CallbackType::Update).Add([]() {
            static auto last=std::chrono::high_resolution_clock::now();

            auto now=std::chrono::high_resolution_clock::now();
            auto diff=now-last;
            last=now;

            static Logger log;
            // log(diff.count());
        },
                                                                callback_id);
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