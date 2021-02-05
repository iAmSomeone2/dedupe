//
// Created by bdavidson on 2/5/21.
//

#include "ThreadPool.hpp"

using namespace DeDupe;

ThreadPool::ThreadPool(const uint32_t& threads) : shutdown(false)
{
    for (uint32_t i = 0; i < threads; ++i)
    {
        // Set up wait loops for each thread.
        this->workers.emplace_back(
            [this]
            {
                while(!this->shutdown)
                {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->queueMutex);
                        this->condition.wait(lock,
                                             [this]{ return this->shutdown || !this->tasks.empty(); });
                        if (this->shutdown && this->tasks.empty())
                        {
                            return;
                        }

                        // De-queue task
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }

                    // Run next task.
                    task();
                }
            }
        );
    }
}

ThreadPool::~ThreadPool() noexcept
{
    {
        std::unique_lock<std::mutex> lock(this->queueMutex);
        this->shutdown = true;
    }

    condition.notify_all();
    for (auto& worker : this->workers)
    {
        worker.join();
    }
}
