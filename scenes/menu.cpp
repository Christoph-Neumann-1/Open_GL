#include <Scene.hpp>
#include <imgui/imgui.h>
#include <SceneLoader.hpp>
#include <Data.hpp>
#include <malloc.h>
#include <array>
#include <string>

using namespace std::string_literals;

class SceneMenu final : public Scene
{
    using cbt = CallbackHandler::CallbackType;
    SceneLoader *loader;
    SceneLoader scene;

    static void ImGui(void *, void *);
    static void Close(void *, void *);
    static void Switch(void *, void *);

    std::string path;
    std::array<std::pair<std::string, std::string>, 9> scenes{
        std::make_pair("Picture"s, "S1"s),
        std::make_pair("Curve"s, "curve"s),
        std::make_pair("Quad"s, "quad"s),
        std::make_pair("Triangle"s, "triangle"s),
        std::make_pair("Circle"s, "circle"s),
        std::make_pair("Circle_dyn"s, "dyncirc"s),
        std::make_pair("bouncyballs"s, "instbounce"s),
        std::make_pair("Model"s, "Model"s),
        std::make_pair("Stars"s, "starsim"s),
    };

public:
    explicit SceneMenu(SceneLoader *_loader);
    ~SceneMenu();
};

SceneMenu::SceneMenu(SceneLoader *_loader) : loader(_loader), scene(loader->window, loader->callbackhandler)
{
    loader->callbackhandler->Register(cbt::ImGuiRender, this, ImGui);
}

SceneMenu::~SceneMenu()
{
    loader->callbackhandler->RemoveAll(this);
}

void SceneMenu::ImGui(void *_this_, void *)
{
    auto _this = reinterpret_cast<SceneMenu *>(_this_);
    if (!(_this->scene.hasScene() && _this->scene.GetFlags()["hide_menu"]))
    {
        ImGui::Begin("Scenes");

        for (unsigned int i = 0; i < _this->scenes.size(); i++)
        {
            if (ImGui::Button(_this->scenes[i].first.c_str()) || glfwGetKey(_this->loader->window, GLFW_KEY_1 + i))
            {
                _this->path = ROOT_Directory + "/scenes/bin/" + _this->scenes[i].second + ".so";
                _this->loader->callbackhandler->Register(cbt::PreRender, _this_, Switch);
            }
        }
        if (ImGui::Button("Close") || glfwGetKey(_this->loader->window, GLFW_KEY_ESCAPE))
        {
            _this->loader->callbackhandler->Register(cbt::PostRender, _this_, Close);
        }
        if (ImGui::Button("Pause"))
        {
            if (_this->loader->window.paused)
            {
                _this->loader->window.paused = false;
                _this->loader->callbackhandler->last_update = std::chrono::high_resolution_clock::now();
            }
            else
            {
                _this->loader->window.paused = true;
            }
        }
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }
    else if (glfwGetKey(_this->loader->window, GLFW_KEY_ESCAPE))
    {
        _this->loader->callbackhandler->Register(cbt::PostRender, _this_, Close);
    }
}

void SceneMenu::Close(void *_this_, void *)
{
    auto _this = reinterpret_cast<SceneMenu *>(_this_);
    _this->scene.UnLoad();
    _this->loader->callbackhandler->RemoveAllInstances(cbt::PostRender, {_this_, Close});
    malloc_trim(100);
}

void SceneMenu::Switch(void *_this_, void *)
{
    auto _this = reinterpret_cast<SceneMenu *>(_this_);
    _this->scene.Load(_this->path.c_str());
    _this->loader->callbackhandler->Remove(cbt::PreRender, {_this_, Switch});
    malloc_trim(100);
}

extern "C" Scene *_INIT_(SceneLoader *loader)
{
    return new SceneMenu(loader);
}

extern "C" void _EXIT_(Scene *scene)
{
    delete scene;
}