#include <Scene.hpp>
#include <imgui/imgui.h>
#include <Data.hpp>
#include <vector>
#include <string>

using namespace GL;

class SceneMenu final : public Scene
{
    using cbt = CallbackType;
    SceneLoader scene;

    void ImGui();

    std::string path;
    std::vector<std::pair<std::string, std::string>> scenes{
        {"Example", "Example"},
        {"Model Test", "Model"}};

public:
    explicit SceneMenu(SceneLoader *_loader);
    ~SceneMenu();
};

SceneMenu::SceneMenu(SceneLoader *_loader) : Scene(_loader), scene(_loader->GetWindow(), _loader->GetCallback(), _loader->GetTimeInfo())
{
    RegisterFunc(std::bind(&SceneMenu::ImGui, this), cbt::ImGuiRender);
    scene.SetUnloadCb([&](SceneLoader *) { if (scene.GetFlag("Close Window")){
         RegisterFunc([&](){scene.UnLoad();},cbt::PreRender);
         RegisterFunc([&](){loader->UnLoad();},cbt::PreRender);
         }
          return true; });
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
        if (ImGui::Button("Close") || glfwGetKey(loader->GetWindow(), GLFW_KEY_ESCAPE))
        {
            scene.UnLoad();
        }
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }
    else if (glfwGetKey(loader->GetWindow(), GLFW_KEY_ESCAPE))
    {
        scene.UnLoad();
    }
}

SCENE_LOAD_FUNC(SceneMenu)