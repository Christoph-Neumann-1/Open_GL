#pragma once

#include <Window.hpp>
#include <DeltaTime.hpp>
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
            explicit InvalidScene(const std::string &reason) : _what(reason) {}
        };

    private:
        void *loaded = nullptr; //Dl Handle
        Scene *s = nullptr;
        Window &window;
        CallbackHandler &cbh;
        TimeInfo &timeinfo;
        std::mutex flags_lock;
        std::unordered_map<std::string, std::atomic_int> flags;

        std::atomic_bool update_ready = false, render_ready = false;
        std::atomic_bool update_ready2 = false, render_ready2 = false;
        std::atomic_bool is_loading_or_unloading = false;
        u_int update_id, render_id;
        u_int update_id2, render_id2;

        void load_func(const std::string &);

    public:
        SceneLoader(Window &_window, CallbackHandler &_cbh, TimeInfo &_timeinfo) : window(_window), cbh(_cbh), timeinfo(_timeinfo) {}
        ~SceneLoader()
        {
            if (loaded)
            {
                Terminate();
            }
        }

        ///@brief Load a new Scene. Unloads old if necessary.
        void Load(const std::string &path);

        ///@brief Call the _UNLOAD_ Function and Unload the scene.
        void UnLoad();

        ///@brief int values used to communicate state.
        ///
        /// Undefined behaviour if used after unloading. To check if flags may be used check the _VALID_ flag
        std::atomic_int &GetFlag(const std::string &name);
        bool HasScene() const { return s ? 1 : 0; }
        const Window &GetWindow() const { return window; }
        const TimeInfo &GetTimeInfo() const { return timeinfo; }
        CallbackHandler &GetCallback() const { return cbh; }

        void Terminate();
    };
}