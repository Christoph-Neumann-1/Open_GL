#pragma once

#include <unordered_map>
#include <vector>
#include <functional>
#include <mutex>
#include <atomic>

namespace GL
{
    ///@brief a list of possible callbacks
    enum class CallbackType
    {
        PreUpdate,      ///< Before Update
        Update,         ///<When updating logic
        PostUpdate,     ///<After Update
        PreRender,      ///<Before Render
        Render,         ///<While doing draw calls
        PostRender,     ///<After Render
        ImGuiRender,    ///<For rendering debug interfaces
        OnWindowResize, ///<Called when window size changes
    };

    /**
     * @brief Contains a list of functions.
     * Used in CallbackHandler. Has little use on its own.
     */
    class CallbackList
    {
        ///@brief A function + its id for removal.
        struct Callback
        {
            u_int id;     //Function id assigned by CallbackList
            u_int obj_id; //Id of object or group of functions. Lets you remove them together.
            std::function<void()> function;

            void operator()() { function(); }
        };

        std::mutex mutex; //Gets locked when modifying the callback list or during Call list.
        std::vector<Callback> functions;
        u_int current_id = 1; //Keeps track of the function ids, that have been asigned.

        struct RemoveCallback
        {
            u_int id;       //Either a function id or an id from CallbackHandler.
            bool caller_id; //Whether the id is from CallbackHandler.
        };

        std::mutex mutex_queue; //Locked when adding or removing callbacks or when those modifications get applied.
        std::vector<RemoveCallback> remove_queue;
        std::vector<Callback> add_queue;

        void ProcessQueues();

    public:
        ///@brief Call every function
        void Call();

        ///@brief Add a new Callback. Gets applied during the next Call()
        u_int Add(const std::function<void()> &function, u_int caller_id = 0);

        ///@brief Removes a callback by its id. Gets applied during the next Call()
        void Remove(u_int id);

        ///@brief Removes a callback by the id from CallbackHandler. Gets applied during the next Call()
        void RemoveAll(u_int caller_id);

        ///@see Call()
        void operator()()
        {
            Call();
        }

        void ProcessNow()
        {
            std::lock_guard lk(mutex);
            ProcessQueues();
        }
    };

    ///@brief Contains multiple CallbackLists.
    class CallbackHandler
    {
        std::unordered_map<CallbackType, CallbackList> lists;
        std::atomic_uint current_id = 1; //Keeps track of which ids have been assigned.

    public:
        ///@brief Returns the CallbackList associated with type.
        CallbackList &GetList(CallbackType type) { return lists[type]; }

        ///@brief Get a unique id.
        u_int GenId() { return current_id++; }

        ///@brief Calls RemoveAll on every CallbackList
        void RemoveAll(u_int callerid);

        void ProcessNow()
        {
            for (auto &list : lists)
                list.second.ProcessNow();
        }
    };
}