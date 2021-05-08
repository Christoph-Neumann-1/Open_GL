#pragma once

#include <Callback.hpp>
#include <SceneLoader.hpp>

#define SCENE_LOAD_FUNC(name) \
    extern "C" GL::Scene *_LOAD_(GL::SceneLoader *loader) { return new name(loader); }
namespace GL
{

    /**
     * @brief Base for all scenes.
     * Look at Scene.hpp for info about Loading and Unloading.
    */
    class Scene
    {
    protected:
        SceneLoader *loader; //All scenes will need this.
        uint callback_id = loader->GetCallback().GenId();
        float RenderDelta() { return loader->GetTimeInfo().RenderDeltaTime(); } //Purely for convenience.
        float PhysicsDelta() { return loader->GetTimeInfo().UpdateInterval(); } //Purely for convenience.

        /**
         * @brief Register a callback to this object
         * 
         * I forgot using callback_id and caused segfaults, so I added this.
         */
        uint RegisterFunc(std::function<void()> &&func, CallbackList &list)
        {
            return list.Add(func, callback_id);
        }

        ///@overload
        uint RegisterFunc(std::function<void()> &&func, CallbackType cbt)
        {
            return loader->GetCallback().GetList(cbt).Add(func, callback_id);
        }

        void RemoveFunctions()
        {
            loader->GetCallback().RemoveAll(callback_id);
        }

    public:
        explicit Scene(SceneLoader *_loader) : loader(_loader)
        {
        }

        virtual ~Scene() {}
    };
}

/**
 * @file
 * @brief Base file for all scenes.
 * 
 * @details A valid scene **must** have the following method: 
 * 
 * @code{.cpp}
 * 
 * //return a pointer to a new Scene instance.
 * extern "C" Scene *_LOAD_(SceneLoader *loader);
 * @endcode
 */