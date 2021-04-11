///@file
#pragma once

#include <Data.hpp>
#include <Scene.hpp>
#include <Log.hpp>
#include <string>
#include <Window.hpp>
#include <Callback.hpp>
#include <glm/gtc/matrix_transform.hpp>

/**
 * @brief Dynamically loads @ref Scene "Scenes"
 * 
 * Multiple instances can be created, but copies are forbidden
 */
class SceneLoader
{

    void *loaded;
    Logger logger;
    Scene *s;

public:
    CallbackHandler *callbackhandler;
    Window &window;                                                                         ///<Necessary to get window size.
    glm::mat4 proj = glm::ortho(0.0f, (float)window.x, 0.0f, (float)window.y, -1.0f, 1.0f); ///<Up to date matrix

    /**
     * @brief Construct a SceneLoader and set the window
     * 
     * @param _window Window
     */
    explicit SceneLoader(Window &_window, CallbackHandler *cbh) : loaded(nullptr), s(nullptr), callbackhandler(cbh), window(_window)
    {
        callbackhandler->Register(CallbackHandler::CallbackType::OnWindowResize, this, OnResize);
    }

    /**
     * @brief Deleted because copying is forbidden
     */
    SceneLoader(const SceneLoader &) = delete;

    /**
     * @brief Destroy the Scene Loader object and its Scene, if loaded.
     * 
     * Will Unload the Scene once it is no longer in use.
     * 
     */
    ~SceneLoader();

    /**
     * @brief Load a Scene or wsitch if one is loaded.I
     * 
     * Will throw a runtime error if the path or the Scene is invalid.
     * 
     * @param path The path where the scene is located
     * 
     * @see Scene
     */
    void Load(const char *path);

    /**
     * @brief UnLoad the Scene and destroy it.
     * 
     * Will throw a runtime error if the _EXIT_ Method does not exist.
     * 
     * @see Scene
     */
    void UnLoad();

    std::unordered_map<std::string, int> &GetFlags()
    {
        if (s == nullptr)
            throw std::runtime_error("No scene loaded when accessing flags");
        return s->flags;
    }
    bool hasScene() { return s; }

private:
    static void OnResize(void *_this_, void *)
    {
        auto _this = reinterpret_cast<SceneLoader *>(_this_);
        _this->proj = glm::ortho(0.0f, (float)_this->window.x, 0.0f, (float)_this->window.y, -1.0f, 1.0f);
    };
};