#pragma once

#include <future>
#include <mutex>
#include <atomic>
#include <thread>
#include <queue>
#include <vector>
#include <functional>

class ThreadPool
{
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> queue;

    bool stop = false;
    bool terminate = false;
    std::condition_variable cv;
    std::mutex queue_mutex;

    void Loop()
    {
        std::function<void()> task;

        while (1)
        {
            {
                std::unique_lock lk(queue_mutex);
                cv.wait(lk, [&]() {return stop || terminate || !queue.empty(); });
                if (stop && !queue.size())
                    return;
                if (terminate)
                    return;
                task = std::move(queue.front());
                queue.pop();
            }
            task();
        }
    }

public:
    ThreadPool(uint nThreads)
    {
        threads.reserve(nThreads);
        for (uint i = 0; i < nThreads; i++)
        {
            threads.emplace_back(std::bind(&ThreadPool::Loop,this));
        }
    }

    ~ThreadPool()
    {
        {
            std::scoped_lock lk(queue_mutex);
            stop = true;
        }
        cv.notify_all();
        for (std::thread &thread : threads)
            thread.join();
    }

    ///@brief Add new item to queue
    template <typename Func, typename... Args>
    auto Add(Func &&f, Args &&...args)
        -> std::future<typename std::result_of<Func(Args...)>::type>
    {
        using return_type = typename std::result_of<Func(Args...)>::type;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<Func>(f), std::forward<Args>(args)...));

        std::future<return_type> res = task->get_future();

        {
            std::unique_lock<std::mutex> lock(queue_mutex);

            if (!(stop || terminate))
                queue.emplace([task]() { (*task)(); });
        }
        cv.notify_one();
        return res;
    }

    ///@brief terminate once all tasks are finished. Does not accept new tasks anymore.
    void Stop()
    {
        std::scoped_lock lk(queue_mutex);
        stop = true;
    }

    ///@brief Stop as fast as possible
    void Terminate()
    {
        std::scoped_lock lk(queue_mutex);
        terminate = true;
    }

};
