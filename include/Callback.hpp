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
     * Used in CallbackHandler.
     */
    class CallbackList
    {
        ///@brief A function + its id for removal.
        struct Callback
        {
            uint id;                        //Function id assigned by CallbackList
            uint obj_id;                    //Id of object or group of functions. Lets you remove them together.
            std::function<void()> function; //The function which gets called. NOTE please destroy before unloading the scene.

            void operator()() { function(); } //For convinience

            Callback &operator=(Callback &&cb)
            {
                id = cb.id;
                obj_id = cb.obj_id;
                function = std::move(cb.function);
                return *this;
            }

            Callback &operator=(const Callback &cb)
            {
                id = cb.id;
                obj_id = cb.obj_id;
                function = cb.function;
                return *this;
            }

            Callback(uint _id, uint _obj_id, const std::function<void()> &func) : id(_id), obj_id(_obj_id), function(func) {}
            Callback(uint _id, uint _obj_id, std::function<void()> &&func) : id(_id), obj_id(_obj_id), function(std::move(func)) {}

            Callback(Callback &&old) : id(old.id), obj_id(old.obj_id), function(std::move(old.function)) {}
        };

        std::mutex mutex; //Gets locked when modifying the callback list or during Call list.
        std::vector<Callback> functions;
        uint current_id = 1; //Keeps track of the function ids, that have been assigned.

        struct RemoveCallback
        {
            uint id;        //Either a function id or an id from CallbackHandler.
            bool caller_id; //Whether the id is from CallbackHandler.

            RemoveCallback(uint _id, bool _cid) : id(_id), caller_id(_cid) {}
        };

        std::mutex mutex_queue; //Locked when adding or removing callbacks and when those modifications get applied.
        std::vector<RemoveCallback> remove_queue;
        std::vector<Callback> add_queue;

        void ProcessQueues();

    public:
        ///@brief Call every function
        void Call()
        {
            std::lock_guard lock(mutex);
            ProcessQueues();
            for (auto &func : functions)
            {
                func();
            }
        }

        template <typename F, typename... Args>
        uint Add(F &&function, uint caller_id = 0, Args... args)
        {
            std::lock_guard lock(mutex_queue);
            add_queue.emplace_back(current_id, caller_id, std::bind(std::move(function), args...));
            return current_id++;
        }

        template <typename F, typename... Args>
        uint Add(const F &function, uint caller_id = 0, Args... args)
        {
            std::lock_guard lock(mutex_queue);
            add_queue.emplace_back(current_id, caller_id, std::bind(function, args...));
            return current_id++;
        }

        ///@brief Removes a callback by its id. Gets applied during the next Call()
        void Remove(uint id);

        ///@brief Removes a callback by the id from CallbackHandler. Gets applied during the next Call()
        void RemoveAll(uint caller_id);

        ///@see Call()
        void operator()()
        {
            Call();
        }

        /**
         * @brief Process add and remove now.
         * 
         * Does not work while calling. Only use outside of calls.
         * 
         */
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
        std::mutex list_m;
        std::function<void(const std::function<void()> &)> SyncFunc;

    public:
        CallbackHandler(const std::function<void(const std::function<void()> &)> &sync) : SyncFunc(sync) {}

        ///@brief Returns the CallbackList associated with type.
        CallbackList &GetList(CallbackType type)
        {
            std::scoped_lock lk(list_m);
            return lists[type];
        }

        ///@brief Get a unique id.
        uint GenId() { return current_id++; }

        ///@brief Calls RemoveAll on every CallbackList
        void RemoveAll(uint callerid);

        ///Same as CallbackList.
        void ProcessNow()
        {
            std::scoped_lock lk(list_m);
            for (auto &list : lists)
                list.second.ProcessNow();
        }

        /**
         * @brief Clear all callbacks
         * 
         */
        void Terminate()
        {
            std::scoped_lock lk(list_m);
            lists.clear();
        }

        /**
         * @brief Callfunction without worrying about threads.
         * 
         * Calls the function provided when creating object.
         */
        void SynchronizedCall(const std::function<void()> &func)
        {
            SyncFunc(func);
        }
    };
}