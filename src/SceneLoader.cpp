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
            throw InvalidScene(std::string("Could not load Scene. Error: ") + dlerror());
        }
        auto init_func = reinterpret_cast<Scene *(*)(SceneLoader *)>(dlsym(loaded, "_LOAD_"));

        if (!init_func)
        {
            dlclose(loaded);
            throw InvalidScene(std::string("Could not find load function. Error: ") + dlerror());
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

            update_ready = false;
            render_ready = false;

            std::unique_lock lk(flags_lock);
            flags.clear();
            flags["_VALID_"] = 0;
            lk.unlock();

            update_id = cbh.GetList(CallbackType::PreUpdate).Add([&]() {
                update_ready = true;
                if (render_ready.load())
                {
                    delete s;
                    s = nullptr;
                    dlclose(loaded);
                    loaded = nullptr;
                }
                cbh.GetList(CallbackType::PreUpdate).Remove(update_id);
            });

            render_id = cbh.GetList(CallbackType::PreRender).Add([&]() {
                render_ready = true;
                if (update_ready.load())
                {
                    delete s;
                    s = nullptr;
                    dlclose(loaded);
                    loaded = nullptr;
                }
                cbh.GetList(CallbackType::PreRender).Remove(render_id);
            });

            update_ready2 = false;
            render_ready2 = false;

            load_func(path);

            update_id2 = cbh.GetList(CallbackType::PreUpdate).Add([&]() {
                update_ready2 = true;
                if (render_ready2.load())
                {
                    load_func(path);
                    is_loading_or_unloading = false;
                }
                cbh.GetList(CallbackType::PreUpdate).Remove(update_id2);
            });

            render_id2 = cbh.GetList(CallbackType::PreRender).Add([&]() {
                render_ready2 = true;
                if (update_ready2.load())
                {
                    load_func(path);
                    is_loading_or_unloading = false;
                }
                cbh.GetList(CallbackType::PreRender).Remove(render_id2);
            });
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

        update_ready = false;
        render_ready = false;

        std::unique_lock lk(flags_lock);
        flags.clear();
        flags["_VALID_"] = 0;
        lk.unlock();

        update_id = cbh.GetList(CallbackType::PreUpdate).Add([&]() {
            update_ready = true;
            if (render_ready.load())
            {
                delete s;
                s = nullptr;
                dlclose(loaded);
                loaded = nullptr;
                is_loading_or_unloading = false;
            }
            cbh.GetList(CallbackType::PreUpdate).Remove(update_id);
        });

        render_id = cbh.GetList(CallbackType::PreRender).Add([&]() {
            render_ready = true;
            if (update_ready.load())
            {
                delete s;
                s = nullptr;
                dlclose(loaded);
                loaded = nullptr;
                is_loading_or_unloading = false;
            }
            cbh.GetList(CallbackType::PreRender).Remove(render_id);
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