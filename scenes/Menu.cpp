#include <Scene.hpp>
#include <imgui/imgui.h>
#include <Data.hpp>
#include <vector>
#include <string>
#include <Input.hpp>

using namespace GL;

class SceneMenu final : public Scene
{
    using cbt = CallbackType;
    SceneLoader scene{loader->GetWindow(), loader->GetCallback(), loader->GetTimeInfo()};

    void ImGui();

    std::string path;
    std::vector<std::pair<std::string, std::string>> scenes{
        {"Example", "Example"},
        {"Solar", "Solar"},
        {"Batch", "Batch"},
        {"Voxel Test", "Voxel/Test"},
        {"Model Test", "Model"},
        {"Stars", "Stars"}};

    InputHandler::KeyCallback escape{*loader->GetWindow().inputptr, glfwGetKeyScancode(GLFW_KEY_ESCAPE), InputHandler::Action::Press, [&](int)
                                                    {
                                                        if (scene.HasScene())
                                                            scene.UnLoad();
                                                        else
                                                            glfwSetWindowShouldClose(loader->GetWindow(), 2);
                                                    }};

public:
    explicit SceneMenu(SceneLoader *_loader);
    ~SceneMenu();
};

SceneMenu::SceneMenu(SceneLoader *_loader) : Scene(_loader)
{
    RegisterFunc(cbt::ImGuiRender, &SceneMenu::ImGui, this);
    scene.SetUnloadCb([&](SceneLoader *)
                      {
                          if (scene.GetFlag("Close Window"))
                          {
                              RegisterFunc([&]()
                                           { scene.UnLoad(); },
                                           cbt::PreRender);
                              RegisterFunc([&]()
                                           { loader->UnLoad(); },
                                           cbt::PreRender);
                          }
                          return true;
                      });
}

SceneMenu::~SceneMenu()
{
    RemoveFunctions();
}

void SceneMenu::ImGui()
{
    if (!(scene.HasScene() && scene.GetFlag("hide_menu")))
    {
        ImGui::Begin("Scenes");

        for (unsigned int i = 0; i < scenes.size(); i++)
        {
            if (ImGui::Button(scenes[i].first.c_str()) || glfwGetKey(loader->GetWindow(), GLFW_KEY_1 + i))
            {
                path = ROOT_Directory + "/scenes/bin/" + scenes[i].second + ".scene";
                scene.Load(path);
            }
        }
        if (ImGui::Button("Close"))
        {
            if (scene.HasScene())
                scene.UnLoad();
            else
                glfwSetWindowShouldClose(loader->GetWindow(), 2);
        }
        if (ImGui::Button("Switch FS"))
        {

            loader->GetWindow().SetFullscreen(!loader->GetWindow().IsFullscreen());
        }
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }
}

SCENE_LOAD_FUNC(SceneMenu)