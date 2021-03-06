/**
 * @file Menu.cpp
 * @brief Opens the other scenes.
 * @author Christoph Neumann
 * @copyright Copyright © 2021 Christoph Neumann - MIT License
 */
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
    //The first entry is the name shown, the second one the file path relative to the scenes binary directory without the .scene extension
    std::vector<std::pair<std::string, std::string>> scenes{
        {"Example", "Example"},
        {"Breakout", "Breakout"},
        {"Voxel Test", "Voxel/Test"},
        {"Gas", "Gas"},
        {"FBO","FrameBufferDemo"},
        {"Ball", "BallInBox"},
        {"Stars", "Stars"}};

    InputHandler::KeyCallback escape{GetInputHandler(), glfwGetKeyScancode(GLFW_KEY_ESCAPE), InputHandler::Action::Press, [&](int)
                                                    {
                                                        if (scene.HasScene())
                                                            scene.UnLoad();
                                                        else
                                                            glfwSetWindowShouldClose(loader->GetWindow(), 2);
                                                    }};

public:
    explicit SceneMenu(SceneLoader *_loader);
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

void SceneMenu::ImGui()
{
    if (!(scene.HasScene() && scene.GetFlag("hide_menu")))
    {
        ImGui::Begin("Scenes");

        for (unsigned int i = 0; i < scenes.size(); i++)
        {
            if (ImGui::Button(scenes[i].first.c_str()))
            {
                path = "scenes/bin/" + scenes[i].second + ".scene";
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