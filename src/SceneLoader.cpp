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
            const char* error=dlerror();
            throw InvalidScene(std::string("Could not load Scene. Error: ") + (error ? error : ""));
        }
        auto init_func = reinterpret_cast<Scene *(*)(SceneLoader *)>(dlsym(loaded, "_LOAD_"));

        if (!init_func)
        {
            const char* error=dlerror();
            dlclose(loaded);
            throw InvalidScene(std::string("Could not find load function. Error: ") + (error ? error : ""));
        }
        s = init_func(this);

        std::lock_guard lk(this->flags_lock);
        flags["_VALID_"] = 1;
    };

    void SceneLoader::Load(const std::string &path)
    {
        bool t = true, f = false;
        if (!is_loading_or_unloading.compare_exchange_weak(f, t))
            return;

        if (loaded)
        {
            s->PrepareUnload();

            update_ready = 0;

            std::unique_lock lk(flags_lock);
            flags.clear();
            flags["_VALID_"] = 0;
            lk.unlock();

            update_id = cbh.GetList(CallbackType::PreUpdate).Add(std::bind([&](std::string path) {
                update_ready++;
                if (update_ready >= 2)
                    cbh.GetList(CallbackType::PreUpdate).Remove(update_id);
            },path));

            render_id = cbh.GetList(CallbackType::PreRender).Add(std::bind([&](std::string path) {
                if (update_ready >= 2)
                {
                    delete s;
                    s = nullptr;
                    dlclose(loaded);
                    loaded = nullptr;
                    load_func(path);
                    is_loading_or_unloading = false;
                    cbh.GetList(CallbackType::PreRender).Remove(render_id);
                }
            },path));

        }
        else
        {
            load_func(path);
            is_loading_or_unloading = false;
        }
    }

    void SceneLoader::UnLoad()
    {
        if (!loaded)
            return;
        bool t = true, f = false;
        if (!is_loading_or_unloading.compare_exchange_weak(f, t))
            return;

        s->PrepareUnload();

        update_ready = 0;

        std::unique_lock lk(flags_lock);
        flags.clear();
        flags["_VALID_"] = 0;
        lk.unlock();

        update_id = cbh.GetList(CallbackType::PreUpdate).Add([&]() {
            update_ready++;
            if (update_ready >= 2)
                cbh.GetList(CallbackType::PreUpdate).Remove(update_id);
        });

        render_id = cbh.GetList(CallbackType::PreRender).Add([&]() {
            if (update_ready >= 2)
            {
                delete s;
                s = nullptr;
                dlclose(loaded);
                loaded = nullptr;
                is_loading_or_unloading = false;
                cbh.GetList(CallbackType::PreRender).Remove(render_id);
            }
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
            s->Terminate();
            cbh.ProcessNow();
            delete s;
            s = nullptr;

            dlclose(loaded);
            loaded = nullptr;
        }
    }
}