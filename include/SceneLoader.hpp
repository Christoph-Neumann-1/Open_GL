/**
 * @file SceneLoader.hpp
 * @author Christoph Neumann
 * @copyright Copyright © 2021 Christoph Neumann - MIT License
 */
#pragma once

#include <Window.hpp>
#include <Time.hpp>
#include <stdexcept>
#include <unordered_map>
#include <string>

namespace GL
{
    using namespace GL;
    class Scene;

    ///@brief Loads Scenes at runtime and provides a way for them to communicate.
    class SceneLoader
    {
    public:
        class InvalidScene final : public std::exception
        {
            std::string _what;

        public:
            const char *what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW override { return _what.c_str(); }
            explicit InvalidScene(std::string_view reason) : _what(reason) {}
        };

    private:
        void *loaded = nullptr; //Dl Handle
        Scene *s = nullptr;
        Window &window;
        CallbackHandler &cbh;
        TimeInfo &timeinfo;
        std::mutex flags_lock;
        std::unordered_map<std::string, std::atomic_int> flags;

        std::atomic_bool is_loading_or_unloading = false;

        void load_func(const std::string &, const std::string &);

        std::function<bool(SceneLoader *, const std::string &, const std::string &)> OnLoad = [](SceneLoader *, const std::string &, const std::string &)
        { return true; };
        std::function<bool(SceneLoader *)> OnUnload = [](SceneLoader *)
        { return true; };

    public:
        SceneLoader(Window &_window, CallbackHandler &_cbh, TimeInfo &_timeinfo) : window(_window), cbh(_cbh), timeinfo(_timeinfo) {}
        ~SceneLoader()
        {
            Terminate();
        }

        SceneLoader(const SceneLoader &) = delete;
        SceneLoader &operator=(const SceneLoader &) = delete;

        ///@brief Load a new Scene. Unloads old if necessary.
        void Load(const std::string &path);

        ///@brief Same as Load(const std::string &) but works with multiple scenes in the same library.
        void Load(const std::string &path, const std::string &name);

        //TODO: a function that awoids the constant looking up of the load function
        //This also means a unload function that keeps the library loaded

        ///@brief Delete the current scene and unload the library.
        void UnLoad();

        //TODO: add a callback to notify the user that a flag has changed
        //TODO: Add a system to allow for passing data between scenes
        ///@brief int values used to communicate state.
        ///
        /// Do not use after unloading. To check if the scene is valid use _VALID_
        std::atomic_int &GetFlag(const std::string &name);
        bool HasScene() const { return s ? 1 : 0; }
        Window &GetWindow() const { return window; }
        TimeInfo &GetTimeInfo() const { return timeinfo; }
        CallbackHandler &GetCallback() const { return cbh; }

        ///Close the scene immediately. Ignores the unload callback.
        void Terminate();

        /**
         * @brief This function will be called whenever a scene is loaded.
         * 
         * If it returns false, the scene will not be loaded.
         * The first parameter is this object, the second is the path to the library containing the scene. 
         * The third is the name of the scene, or an empty string if there is only one scene in the library.
         * 
         */
        void SetLoadCb(const std::function<bool(SceneLoader *, const std::string &, const std::string &)> &cb) { OnLoad = cb; }

        /**
         * @brief This function will be called whenever a scene is unloaded.
         * 
         * If it returns false, the scene will not be unloaded.
         * The terminate function bypasses this callback.
         */
        void SetUnloadCb(const std::function<bool(SceneLoader *)> &cb) { OnUnload = cb; }
    };
}