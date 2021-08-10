/**
 * @file Callback.cpp
 * @author Christoph Neumann
 * @copyright Copyright © 2021 Christoph Neumann - MIT License
 */
#include <Callback.hpp>
#include <algorithm>
namespace GL
{
    void CallbackList::ProcessQueues()
    {
        std::scoped_lock lk(mutex_queue);

        for (auto &func : add_queue)
        {
            functions.emplace_back(std::move(func));
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


    void CallbackList::Remove(CallbackId id)
    {
        if (!id)
            return;
        std::scoped_lock lock(mutex_queue);
        remove_queue.emplace_back(id, false);
    }

    void CallbackList::RemoveAll(CallbackGroupId caller_id)
    {
        if (!caller_id)
            return;
        std::scoped_lock lock(mutex_queue);
        remove_queue.emplace_back(caller_id, true);
    }

    void CallbackHandler::RemoveAll(CallbackGroupId caller_id)
    {
        for (auto &i : lists)
        {
            i.second.RemoveAll(caller_id);
        }
    }
}