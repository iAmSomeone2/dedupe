//
// Created by bdavidson on 2/5/21.
//

#pragma once

#include <vector>
#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

namespace DeDupe
{
    class ThreadPool
    {
    public:
        /**
         * \brief Constructs a new ThreadPool with the specified number of threads.
         * \param threads number of hardware threads to use.
         */
        explicit ThreadPool(const uint32_t& threads = std::thread::hardware_concurrency());

        template<class Func, class... Args>
        auto enqueue(Func&& func, Args&&... args)
            -> std::future<typename std::result_of<Func(Args...)>::type>;

        /**
         * \brief Shuts down threads and cleans up
         */
        ~ThreadPool() noexcept;

    private:
        /**
         * \brief Threads used in pool
         */
        std::vector<std::thread> workers;

        /**
         * \brief The task queue
         */
        std::queue<std::function<void()>> tasks;

        /**
         * \brief Queue sync mutex
         */
        std::mutex queueMutex;

        /**
         * \brief Queue locking condition
         */
        std::condition_variable condition;

        bool shutdown;
    };

}

#include "ThreadPool.tpp"
