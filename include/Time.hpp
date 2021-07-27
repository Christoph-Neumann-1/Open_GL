#pragma once

#include <chrono>
#include <Callback.hpp>
#include <cmath>
#include <thread>
#include <Logger.hpp>

//TODO: split file

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

        CallbackGroupId objid{cbh.GenId()};

        function<void()> PreRender = [&]()
        {
            auto current_time = std::chrono::steady_clock::now();
            deltatime_render = std::chrono::duration_cast<std::chrono::nanoseconds>(current_time - last_render).count() / powf(10, 9);
            last_render = current_time;
        };

        const std::chrono::time_point<std::chrono::steady_clock> starttime{std::chrono::steady_clock::now()};

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

        std::chrono::nanoseconds NsSinceStart()
        {
            auto current_time = std::chrono::steady_clock::now();
            return std::chrono::duration_cast<std::chrono::nanoseconds>(current_time - starttime);
        }

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
    class PerformanceLoggerScoped
    {
        std::chrono::time_point<std::chrono::steady_clock> begin;
        PerformanceLoggerScoped(const PerformanceLoggerScoped &) = delete;
        PerformanceLoggerScoped &operator=(const PerformanceLoggerScoped &) = delete;

        //I hope I am using this right
        union
        {
            std::string m_message;
            std::function<std::string(std::chrono::nanoseconds)> m_callback;
        };
        bool use_callback;

    public:
        ///@brief Outputs the string followed by " ", the time, and "seconds"
        PerformanceLoggerScoped(std::string message = std::string()) : m_message(message), use_callback(false)
        {
            begin = std::chrono::steady_clock::now();
        }
        ///@brief Outputs whatever your function returns.
        PerformanceLoggerScoped(std::function<std::string(std::chrono::nanoseconds)> callback) : m_callback(callback), use_callback(true)
        {
            begin = std::chrono::steady_clock::now();
        }
        ~PerformanceLoggerScoped()
        {
            auto end = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
            Logger logger;
            if (use_callback)
            {
                logger << m_callback(duration);
                m_callback.~function<std::string(std::chrono::nanoseconds)>();
            }
            else
            {
                logger << m_message << " " << duration.count() / 1e9 << " seconds";
                m_message.~basic_string();
            }
            logger.print();
        }
    };

    /**
     * @brief This is the same as PerformanceLoggerScoped but has explicit begin and end functions, making
     * it useful for repeated logging or just outputting many different timestamps.
     */
    class PerformanceLoggerManual
    {
        std::chrono::time_point<std::chrono::steady_clock> begin;
        PerformanceLoggerManual(const PerformanceLoggerManual &) = delete;
        PerformanceLoggerManual &operator=(const PerformanceLoggerManual &) = delete;

        //I hope I am using this right
        union
        {
            std::string m_message;
            std::function<std::string(std::chrono::nanoseconds)> m_callback;
        };
        bool use_callback;
        Logger logger;

    public:
        ///@brief Record the start time
        void Begin()
        {
            begin = std::chrono::steady_clock::now();
        }

        ///@brief Record the current timea and either output the duration or call the function provided
        void End()
        {
            auto end = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
            if (use_callback)
            {
                logger << m_callback(duration);
            }
            else
            {
                logger << m_message << " " << duration.count() / 1e9 << " seconds";
            }
            logger.print();
        }
        ///@brief Outputs the string followed by " ", the time, and "seconds" during the end function.
        PerformanceLoggerManual(std::string message = std::string()) : m_message(message), use_callback(false) {}
        ///@brief Outputs whatever your function returns during the end function.
        PerformanceLoggerManual(std::function<std::string(std::chrono::nanoseconds)> callback) : m_callback(callback), use_callback(true) {}
        ~PerformanceLoggerManual()
        {
            //Since I am using an union, I need to destroy the correct member manually
            if (use_callback)
            {
                m_callback.~function<std::string(std::chrono::nanoseconds)>();
            }
            else
            {
                m_message.~basic_string();
            }
        }
    };
}