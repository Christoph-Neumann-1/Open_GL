#pragma once

#include <vector>
#include <unordered_map>
#include <chrono>
#include <Renderer.hpp>

/**
 * @brief Manages Events
 * 
 * Keeps track of various types of callbacks and allows them to be called easily. 
 * 
 * @note Has frequent errors. I still don't know if I fixed all of them
 */
class CallbackHandler
{
public:
    std::chrono::time_point<std::chrono::high_resolution_clock> last_update = std::chrono::high_resolution_clock::now(); ///<Time of the last update

    float deltatime_update; ///<Time in seconds since the last update. Can be changed by the pause button.
    ///@brief The kinds of callbacks that are supported.
    enum class CallbackType
    {
        PreUpdate,       ///< Before Update
        Update,          ///<When updating logic
        PostUpdate,      ///<After Update
        PreRender,       ///<Before Render
        Render,          ///<While doing draw calls
        PostRender,      ///<After Render
        ImGuiRender,     ///<For rendering debug interfaces
        OnActive,        ///<Will be called when the scene is activated. @note Not implemented.
        OnInactive,      ///<Will be called when the scene is deactivated, but not destroyed. @note Not implemented.
        OnWindowResize,  ///<Called when window size changes
        OnWindowResize2, ///<Called after OnWindowResize. Usefull because you may want to update some data before updating positions.
    };

    ///@brief Contains all the information of the callback.
    struct Callback
    {
        ///Usually a pointer to a Scene or an other object, but use it however you want.
        void *cb_this;
        ///The function parameters.
        void (*f_ptr)(void *, void *);
        ///Additional data for the callback. @note Must be deleted by the creator of the callback CallbackHandler does not delete heap objects.
        void *data = nullptr;

        ///Call the function.
        void Call() { f_ptr(cb_this, data); }
    };

private:
    std::unordered_map<CallbackType, std::vector<Callback>> callbacks; ///<Contains all callbacks

public:
    /**
     * @brief Add a new Callback to callbacks
     * 
     * @param type When the callback should be called
     * @param callback A callback with all the information needed for the call
     */
    void Register(CallbackType type, const Callback &callback);

    /**
     * @brief Construct the callback in place. 
     * @overload
     */
    template <typename... _Args>
    void Register(CallbackType type, _Args &&...args)
    {
        callbacks[type].emplace_back(std::forward<_Args>(args)...);
    }

    ///@brief Remove a callback of a specific type. The callbacks must be identical in every way, for them to be removed.
    void Remove(CallbackType type, const Callback &callback);

    ///@brief Remove all callbacks with matching type, cb_this, and function. Data is ignored.
    void RemoveAllInstances(CallbackType type, const Callback &callback);

    ///@brief Remove all callbacks with the _this object.
    void RemoveAll(void *_this);

    ///@brief Call all callbacks of type.
    void Call(CallbackType type);
};

///@brief Compare two Callbacks. Used for removing a callback.
inline bool operator==(const CallbackHandler::Callback &other1, const CallbackHandler::Callback &other)
{
    return (other1.cb_this == other.cb_this) && (other1.f_ptr == other.f_ptr) && (other1.data == other.data);
}
