#include <Callback.hpp>
#include <algorithm>
namespace GL
{
    void CallbackList::ProcessQueues()
    {
        std::lock_guard lk(mutex_queue);

        for (auto &func : add_queue)
        {
            functions.push_back(func);
        }

        for (auto item : remove_queue)
        {
            if (item.caller_id)
            {
                functions.erase(std::remove_if(functions.begin(), functions.end(), [&](const Callback &cb) { return item.id == cb.obj_id; }), functions.end());
            }
            else
            {
                functions.erase(std::remove_if(functions.begin(), functions.end(), [&](const Callback &cb) { return item.id == cb.id; }), functions.end());
            }
        }

        remove_queue.clear();
        add_queue.clear();
    }

    void CallbackList::Call()
    {
        std::lock_guard lock(mutex);
        ProcessQueues();
        for (auto &func : functions)
        {
            func();
        }
    }

    u_int CallbackList::Add(const std::function<void()> &function, u_int caller_id)
    {
        std::lock_guard lock(mutex_queue);
        add_queue.push_back({current_id, caller_id, function});
        return current_id++;
    }

    void CallbackList::Remove(u_int id)
    {
        if (!id)
            return;
        std::lock_guard lock(mutex_queue);
        remove_queue.push_back({id, false});
    }

    void CallbackList::RemoveAll(u_int caller_id)
    {
        if (!caller_id)
            return;
        std::lock_guard lock(mutex_queue);
        remove_queue.push_back({caller_id, true});
    }

    void CallbackHandler::RemoveAll(u_int caller_id)
    {
        for (auto &i : lists)
        {
            i.second.RemoveAll(caller_id);
        }
    }
}