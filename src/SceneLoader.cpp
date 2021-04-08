///@file
#include <SceneLoader.hpp>
#include <dlfcn.h>
#include <stdexcept>

SceneLoader::~SceneLoader()
{
    if (loaded != nullptr)
        UnLoad();
    callbackhandler->RemoveAll(this);
}

void SceneLoader::Load(const char *path)
{
    if (loaded != nullptr)
        UnLoad();
    loaded = dlopen(path, RTLD_LAZY);
    if (!loaded)
    {
        loaded = nullptr;
        logger << "Could not Load Scene at " << path << "\t" << dlerror();
        logger.print();
        throw std::runtime_error("Invalid Scene Path.");
    }
    auto f_init = reinterpret_cast<Scene *(*)(SceneLoader *)>(dlsym(loaded, "_INIT_"));

    if (!f_init)
    {
        const char *err = dlerror();
        if (err)
        {
            logger << err;
            logger.print();
        }
        throw std::runtime_error("Invalid Scene");
    }
    s = f_init(this);
}

void SceneLoader::UnLoad()
{
    if (loaded == nullptr)
        return;
    auto f_exit = reinterpret_cast<void (*)(Scene *)>(dlsym(loaded, "_EXIT_"));
    f_exit(s);
    s = nullptr;

    dlclose(loaded);
    loaded = nullptr;
}