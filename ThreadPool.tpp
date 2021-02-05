#pragma once

using namespace DeDupe;

template<class Func, class... Args>
auto ThreadPool::enqueue(Func&& func, Args&&... args)
-> std::future<typename std::result_of<Func(Args...)>::type>
{
    using returnType = typename std::result_of<Func(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<returnType()>>(
        std::bind(std::forward<Func>(func), std::forward<Args>(args)...)
    );

    std::future<returnType> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queueMutex);

        // Don't allow enqueueing after stopping pool
        if (this->shutdown)
        {
            throw std::runtime_error("Enqueue on stopped ThreadPool");
        }

        tasks.emplace([task]{ (*task)(); });
    }

    this->condition.notify_one();
    return res;
}