#pragma once

#include <Callback.hpp>
#include <SceneLoader.hpp>

/**
 * @brief This macro creates a function as described at the end of the file. The function will be called when the scene is loaded
 * and creates a new instance of the scene class.
 * 
 */
#define SCENE_LOAD_FUNC(name) \
    extern "C" GL::Scene *_LOAD_(GL::SceneLoader *loader) { return new name(loader); }
namespace GL
{

    /**
     * @brief Base for all scenes.
     * 
     * Look at Scene.hpp for info about Loading and Unloading.
    */
    class Scene
    {
    protected:
        SceneLoader *loader; //All scenes will need this.
        CallbackGroupId callback_id = loader->GetCallback().GenId();
        float RenderDelta() { return loader->GetTimeInfo().RenderDeltaTime(); } //Purely for convenience.
        float PhysicsDelta() { return loader->GetTimeInfo().UpdateInterval(); } //Purely for convenience.

        /**
         * @brief Register a callback to this object
         * 
         * I forgot using callback_id and caused segfaults, so I added this.
         */
        CallbackId RegisterFunc(std::function<void()> &&func, CallbackList &list)
        {
            return list.Add(func, callback_id);
        }

        ///@overload
        CallbackId RegisterFunc(std::function<void()> &&func, CallbackType cbt)
        {
            return loader->GetCallback().GetList(cbt).Add(func, callback_id);
        }

        ///@overload
        template <typename F, typename... Args>
        CallbackId RegisterFunc(CallbackType cbt, F &&func, Args... args)
        {
            return loader->GetCallback().GetList(cbt).Add(func, callback_id, args...);
        }

        ///@brief Remove all callback register to this class.
        void RemoveFunctions()
        {
            loader->GetCallback().RemoveAll(callback_id);
        }

        ///@brief Also just for convenience.
        std::atomic_int &GetFlag(const std::string &name)
        {
            return loader->GetFlag(name);
        };

        ///@brief Also just for convienience and to make the lines shorter.
        InputHandler &GetInputHandler() { return *loader->GetWindow().inputptr; }

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
 * 
 * I recommend just using the macro at the beginning of the file as the function will look the same for every file.
 */