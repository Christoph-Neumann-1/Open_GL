#include <Callback.hpp>
#include <algorithm>

void CallbackHandler::Register(CallbackType type, const Callback &callback)
{
    callbacks[type].push_back(callback);
}

void CallbackHandler::Remove(CallbackType type, const Callback &callback)
{
    auto &vec = callbacks[type];
    vec.erase(std::remove(vec.begin(), vec.end(), callback), vec.end());
}

void CallbackHandler::RemoveAllInstances(CallbackType type, const Callback &callback)
{
    auto &vec = callbacks[type];
    vec.erase(std::remove_if(vec.begin(), vec.end(), [&callback](const Callback &cb) { return callback.cb_this == cb.cb_this && callback.f_ptr == cb.f_ptr; }), vec.end());
}

void CallbackHandler::RemoveAll(void *_this)
{
    for (auto &type : callbacks)
    {
        std::vector<Callback> &vec = callbacks[type.first];

        vec.erase(std::remove_if(vec.begin(), vec.end(), [&_this](const Callback &cb) { return cb.cb_this == _this; }), vec.end());
    }
}

void CallbackHandler::Call(CallbackType type)
{
    if (type == CallbackType::Update)
    {
        auto dt = std::chrono::high_resolution_clock::now() - last_update;
        deltatime_update = std::chrono::duration_cast<std::chrono::nanoseconds>(dt).count() / pow(10, 9);
        last_update = std::chrono::high_resolution_clock::now();
        }

    for (auto &cb : callbacks[type])
    {
        cb.Call();
    }
}