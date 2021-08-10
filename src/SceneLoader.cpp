/**
 * @file SceneLoader.cpp
 * @author Christoph Neumann
 * @copyright Copyright © 2021 Christoph Neumann - MIT License
 */
#include <SceneLoader.hpp>
#include <dlfcn.h>
#include <string>
#include <Scene.hpp>
namespace GL
{
    void SceneLoader::load_func(const std::string &str)
    {
        loaded = dlopen(str.c_str(), RTLD_LAZY);
        if (!loaded)
        {
            const char *error = dlerror();
            throw InvalidScene(std::string("Could not load Scene. Error: ") + (error ? error : ""));
        }
        auto init_func = reinterpret_cast<Scene *(*)(SceneLoader *)>(dlsym(loaded, "_LOAD_"));

        if (!init_func)
        {
            const char *error = dlerror();
            dlclose(loaded);
            throw InvalidScene(std::string("Could not find load function. Error: ") + (error ? error : ""));
        }
        s = init_func(this);
        flags["_VALID_"] = 1;
    };

    void SceneLoader::Load(const std::string &path)
    {
        bool t = true, f = false;
        if (!is_loading_or_unloading.compare_exchange_weak(f, t))
            return;

        if (loaded)
        {
            cbh.SynchronizedCall(std::bind([&](std::string _path) {
                if (OnUnload(this))
                {
                    delete s;
                    s = nullptr;
                    cbh.ProcessNow();
                    dlclose(loaded);
                    loaded = nullptr;
                    flags.clear();
                    if (OnLoad(this, path))
                        load_func(_path);
                }
                is_loading_or_unloading = false;
            },
                                           path));
        }
        else
        {
            cbh.SynchronizedCall(std::bind([&](std::string _path) {
                if (OnLoad(this, path))
                    load_func(_path);
                is_loading_or_unloading = false;
            },
                                           path));
        }
    }

    void SceneLoader::UnLoad()
    {
        if (!loaded)
            return;
        bool t = true, f = false;
        if (!is_loading_or_unloading.compare_exchange_weak(f, t))
            return;

        cbh.SynchronizedCall([&]() {
            if (OnUnload(this))
            {
                delete s;
                s = nullptr;
                cbh.ProcessNow();
                dlclose(loaded);
                loaded = nullptr;
                flags.clear();
            }
            is_loading_or_unloading = false;
        });
    }

    std::atomic_int &SceneLoader::GetFlag(const std::string &name)
    {
        std::lock_guard lk(flags_lock);
        return flags[name];
    }

    void SceneLoader::Terminate()
    {
        if (loaded)
        {
            delete s;
            s = nullptr;
            cbh.ProcessNow();
            dlclose(loaded);
            loaded = nullptr;
            flags.clear();
        }
    }
}