#pragma once

#include <chrono>
#include <Callback.hpp>
#include <cmath>
#include <thread>

namespace GL
{

    void PreciseSleep(std::chrono::nanoseconds time)
    {
        auto exittime = std::chrono::high_resolution_clock::now() + time;
        std::this_thread::sleep_for(time - std::chrono::nanoseconds(80000));

        while (std::chrono::high_resolution_clock::now() < exittime)
        {

        }
    }

    using std::function;

    /**
     * @brief Provides a time delta between Render calls, how long an Update took, as well as the frequency of Update calls.
     */
    class TimeInfo
    {
        CallbackHandler &cbh;
        float deltatime_update = 0.0f;

        std::chrono::time_point<std::chrono::high_resolution_clock> last_render;
        float deltatime_render;

        uint objid;

        function<void()> PreRender = [&]() {
            auto current_time = std::chrono::high_resolution_clock::now();
            deltatime_render = std::chrono::duration_cast<std::chrono::nanoseconds>(current_time - last_render).count() / powf(10, 9);
            last_render = current_time;
        };

    public:
        explicit TimeInfo(CallbackHandler &_cbh) : cbh(_cbh), deltatime_render(0.0f), objid(cbh.GenId())
        {
            cbh.GetList(CallbackType::PreRender).Add(PreRender, objid);
        }

        ~TimeInfo() { cbh.RemoveAll(objid); }

        ///@attention Should only be called from physics thread
        void SetUpdateInterval(float interval) { deltatime_update = interval; }
        float UpdateInterval() const { return deltatime_update; }
        float RenderDeltaTime() const { return deltatime_render; }
    };
}