/**
 * @file SceneLoader.cpp
 * @author Christoph Neumann
 * @copyright Copyright © 2021 Christoph Neumann - MIT License
 */
#include <SceneLoader.hpp>
#include <dlfcn.h>
#include <string>
#include <Scene.hpp>
#include <filesystem>
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
        //For some reason gdb fails to find the library when using relative paths. This is really annoying because it
        //makes the debugger useless. Therefore this ugly hack is used. It should be unnecessary as dlopen handles relative paths anyway.
        //I can't think of a reason why this doesn't work.
        auto absolutepath = std::filesystem::absolute(path);
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
                    if (OnLoad(this, _path))
                        load_func(_path);
                }
                is_loading_or_unloading = false;
            },
                                           absolutepath));
        }
        else
        {
            cbh.SynchronizedCall(std::bind([&](std::string _path) {
                if (OnLoad(this, _path))
                    load_func(_path);
                is_loading_or_unloading = false;
            },
                                           absolutepath));
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