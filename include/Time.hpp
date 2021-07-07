#pragma once

#include <chrono>
#include <Callback.hpp>
#include <cmath>
#include <thread>

namespace GL
{

    /**
     * @brief Sleeps until just before the end and the does busy waiting to ensure accuracy.
     * 
     * @param time nanoseconds to sleep.
     */
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
     * @brief Provides a time delta between Render call and the frequency of Update calls.
     */
    class TimeInfo
    {
        CallbackHandler &cbh;
        float deltatime_update = 0.0f;

        std::chrono::time_point<std::chrono::high_resolution_clock> last_render;
        double deltatime_render{0.0f};

        uint objid{cbh.GenId()};

        function<void()> PreRender = [&]() {
            auto current_time = std::chrono::high_resolution_clock::now();
            deltatime_render = std::chrono::duration_cast<std::chrono::nanoseconds>(current_time - last_render).count() / powf(10, 9);
            last_render = current_time;
        };

    public:
        
        static constexpr float default_interval=1/100.0f;

        explicit TimeInfo(CallbackHandler &_cbh) : cbh(_cbh)
        {
            cbh.GetList(CallbackType::PreRender).Add(PreRender, objid);
        }

        ~TimeInfo() { cbh.RemoveAll(objid); }

        void SetUpdateInterval(float interval=default_interval) { deltatime_update = interval; }
        const float &UpdateInterval() const { return deltatime_update; }
        double RenderDeltaTime() const { return deltatime_render; }
    };
}