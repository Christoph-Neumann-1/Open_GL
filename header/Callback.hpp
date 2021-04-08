#pragma once

#include <vector>
#include <unordered_map>
#include <chrono>
#include <Renderer.hpp>

/**
 * @brief Manages Callbacks, mainly from scenes.
 * 
 * Keeps track of various types of callbacks and allows them to be called easily. 
 * 
 * @note I am not sure how reliable this thing is.
 */
class CallbackHandler
{
public:
    std::chrono::time_point<std::chrono::high_resolution_clock> last = std::chrono::high_resolution_clock::now();
    std::chrono::time_point<std::chrono::high_resolution_clock> now;

    float deltatime_update;
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
        OnWindowResize2, ///<Called when window size changes and the matrices of loaders are updated
    };

    ///@brief Contains all the information of the callback.
    struct Callback
    {
        ///Pointer to the Scene or other object if available.
        void *cb_this;
        ///The actual callback **ALL** Callback functions **MUST** be in this format.
        void (*f_ptr)(void *, void *);
        ///Additional data for the callback. @note Must be handled by the creator of the callback CallbackHandler does not delete heap objects.
        void *data = nullptr;
        ///Call the function with the specified parameters.
        void Call() { f_ptr(cb_this, data); }
    };

private:
    std::unordered_map<CallbackType, std::vector<Callback>> callbacks;

public:
    /**
     * @brief Add a new Callback.
     * 
     * The Callback will be called when CallbackType is passed to the Call(CallbackType) function.
     * 
     * @param type When the callback should be called
     * @param callback A callback with all the information needed for the call
     */
    void Register(CallbackType type, const Callback &callback);

    /**
     * @brief Removes unnecessary copy should be used when possible. 
     * @overload
     */
    template <typename... _Args>
    void Register(CallbackType type, _Args &&...args)
    {
        callbacks[type].emplace_back(std::forward<_Args>(args)...);
    }

    ///@brief Remove a callback from a specific type. The callbacks must be identical in every way, for them to be removed.
    void Remove(CallbackType type, const Callback &callback);

    ///@brief Remove all callbacks with matching type, this, and function. Data is ignored.
    void RemoveAllInstances(CallbackType type, const Callback &callback);

    ///@brief Remove all callbacks referencing the _this object.
    void RemoveAll(void *_this);

    ///@brief Call all callbacks of type.
    void Call(CallbackType type);
};

///@brief Compare two Callbacks. Used for removing a callback.
inline bool operator==(const CallbackHandler::Callback &other1, const CallbackHandler::Callback &other)
{
    return (other1.cb_this == other.cb_this) && (other1.f_ptr == other.f_ptr) && (other1.data == other.data);
}
