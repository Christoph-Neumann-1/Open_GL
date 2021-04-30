#pragma once

#include <Callback.hpp>
#include <SceneLoader.hpp>

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

    public:
        explicit Scene(SceneLoader *_loader) : loader(_loader)
        {
        }

        virtual ~Scene() {}

        /**
         * @brief Called when unloading.
         * 
         * Remove callbacks here. After Update and Render are finished this object will be deleted.
         */
        virtual void PrepareUnload() {}

        /**
         * @brief Called to imidiatelly close the scene.
         * 
         * Should only be called outside of callbacks, and when no callbacks are currently executed.
         * Basically when destroying the scene. I couldn't find a good way to remove a scene while running callbacks, so this will propably
         * break the CallbackHandler.
         * 
         */
        virtual void Terminate() {}
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