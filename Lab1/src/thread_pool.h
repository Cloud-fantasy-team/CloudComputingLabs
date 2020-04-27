#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <iostream>
#include <functional>
#include <vector>
#include <queue>
#include <mutex>
#include <future>
#include <thread>
#include <condition_variable>


/// A very simple thread pool implementation.
class thread_pool
{
public:
    thread_pool(size_t size)
    :   done(false)
    {
        // Initialize each worker.
        for (int i = 0; i < size; i++)
        {
            workers.emplace_back(
                // Thread object.
                [this]
                {
                    for (;;)
                    {
                        std::function<void()> task;

                        {
                            // Acquire lock.
                            std::unique_lock<std::mutex> lock(this->m);
                            this->condition.wait(
                                lock, 
                                [this]{ return !this->tasks.empty() || this->done; }
                            );

                            // When done is set by the destructor, exit this worker thread.
                            if (this->done)
                                return;

                            // Pull task from the queue.
                            task = std::move(this->tasks.front());
                            this->tasks.pop();
                        }

                        // Run a single task.
                        task();
                    }
                }
            );
        }
    }

    ~thread_pool()
    {
        // Set done.
        {
            std::unique_lock<std::mutex> lock(m);
            done = true;
        }

        // Reap child threads.
        condition.notify_all();
        for (auto &t : workers)
            t.join();
    }

    /// Add a single task to the pool.
    template<
        typename F, 
        typename ...Args,
        typename ret_type = typename std::result_of<F(Args...)>::type
    >
    std::future<ret_type> add_task(F&& f, Args&&... args)
    {
        // Turn [ret_type F(Args...)] into [ret_type F2()] and wrap it into
        // a shared ptr.
        auto task = std::make_shared<std::packaged_task<ret_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<ret_type> p = task->get_future();
        // Acquire lock.
        {
            std::unique_lock<std::mutex> lock(m);

            if (done)
                throw std::runtime_error("added task to stopped pool");

            tasks.emplace([task]{ (*task)(); });
        }

        condition.notify_one();
        return p;
    }

private:
    bool done;

    /// Workers themselves.
    std::vector<std::thread> workers;

    /// Task queue.
    std::queue<std::function<void()>> tasks;

    /// Synchronization primitives.
    std::mutex m;
    std::condition_variable condition;
};

#endif