#pragma once

#include <chrono>
#include <Callback.hpp>
#include <cmath>
#include <thread>
#include <Logger.hpp>

namespace GL
{

    /**
     * @brief Sleeps until just before the end and the does busy waiting to ensure accuracy.
     * 
     * @param time nanoseconds to sleep.
     */
    void PreciseSleep(std::chrono::nanoseconds time)
    {
        auto exittime = std::chrono::steady_clock::now() + time;
        std::this_thread::sleep_for(time - std::chrono::nanoseconds(80000));

        while (std::chrono::steady_clock::now() < exittime)
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

        std::chrono::time_point<std::chrono::steady_clock> last_render;
        double deltatime_render{0.0f};

        uint objid{cbh.GenId()};

        function<void()> PreRender = [&]()
        {
            auto current_time = std::chrono::steady_clock::now();
            deltatime_render = std::chrono::duration_cast<std::chrono::nanoseconds>(current_time - last_render).count() / powf(10, 9);
            last_render = current_time;
        };

    public:
        static constexpr float default_interval = 1 / 100.0f;

        explicit TimeInfo(CallbackHandler &_cbh) : cbh(_cbh)
        {
            cbh.GetList(CallbackType::PreRender).Add(PreRender, objid);
        }

        ~TimeInfo() { cbh.RemoveAll(objid); }

        void SetUpdateInterval(float interval = default_interval) { deltatime_update = interval; }
        const float &UpdateInterval() const { return deltatime_update; }
        double RenderDeltaTime() const { return deltatime_render; }

        TimeInfo(const TimeInfo &) = delete;
        TimeInfo &operator=(const TimeInfo &) = delete;
    };

    /**
     * @brief This is a simple class used for profiling. 
     * 
     * It is scope based will display a user defined message followed by the time when leaving the scope.
     * This class uses the steady clock to reduce outside effects. 
     * The time is measured in nanoseconds but is displayed in seconds.
     */
    class PerformanceLogger
    {
        std::chrono::time_point<std::chrono::steady_clock> begin;
        PerformanceLogger(const PerformanceLogger &) = delete;
        PerformanceLogger &operator=(const PerformanceLogger &) = delete;
        std::string m_message;
        std::function<std::string(std::chrono::nanoseconds)> m_callback;

    public:
        ///@brief Outputs the string followed by " ", the time, and "seconds"
        PerformanceLogger(std::string message = std::string()) : m_message(message)
        {
            begin = std::chrono::steady_clock::now();
        }
        ///@brief Outputs whatever your function returns.
        PerformanceLogger(std::function<std::string(std::chrono::nanoseconds)> callback) : m_callback(callback)
        {
            begin = std::chrono::steady_clock::now();
        }
        ~PerformanceLogger()
        {
            auto end = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
            Logger logger;
            if (m_callback)
                logger << m_callback(duration);
            else
                logger << m_message << " " << duration.count() / 1e9 << " seconds";
            logger.print();
        }
    };
}