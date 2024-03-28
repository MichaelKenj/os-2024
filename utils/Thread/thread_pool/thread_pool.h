#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <atomic>
#include "../blocking_queue/blocking_queue.h"

/*
 A ThreadPool class manages a pool of threads for executing tasks asynchronously.
 It provides a way to enqueue tasks and get futures representing their results.
 The ThreadPool ensures efficient utilization of resources by reusing threads and
 managing a task queue.
*/

class ThreadPool
{
public:
    explicit ThreadPool(std::size_t num_threads = std::thread::hardware_concurrency(), size_t queue_capacity = 10)
            : stop_flag(false), task_queue(queue_capacity)
    {

        num_threads = (num_threads == 0) ? 2 : num_threads;

        /// Create worker threads and assign tasks to them.
        for (size_t i = 0; i < num_threads; ++i)
        {
            workers.emplace_back([this] {
                /// Each worker thread runs in a loop, waiting for tasks to execute.
                while (true)
                {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex);
                        /// Wait until there's a task in the queue or the stop flag is set.
                        condition.wait(lock, [this] { return stop_flag.test() || !task_queue.empty(); });

                        if (stop_flag.test() && task_queue.empty())
                            return;

                        task = std::move(task_queue.pop());
                    }

                    /// Execute the task.
                    task();
                }
            });
        }
    }

    /// Function to enqueue a task into the ThreadPool.
    template <class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<decltype(f(args...))>
    {
        using return_type = decltype(f(args...));
        auto task = std::make_shared<std::packaged_task<return_type()>>([=]() mutable {
            return f(args...);
        });

        {
            std::unique_lock<std::mutex> lock(queue_mutex);

            /// Check if the ThreadPool is stopped.
            if (stop_flag.test())
                throw std::runtime_error("ThreadPool is stopped. Cannot enqueue more tasks.");

            /// Enqueue the task into the task queue.
            task_queue.push([task]() { (*task)(); });
        }

        /// Notify one thread to start executing the task.
        condition.notify_one();
        return task->get_future();
    }

    /// Waits for all threads to finish executing and cleans up resources.
    ~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            /// Set the stop flag to signal threads to exit.
            stop_flag.test_and_set();
        }

        /// Notify all threads to wake up and check the stop flag.
        condition.notify_all();

        /// Join all worker threads.
        for (std::thread &worker : workers)
        {
            worker.join();
        }
    }

private:
    /// Vector to store worker threads.
    std::vector<std::thread> workers;

    /// Task queue for storing tasks.
    LimitedBlockingQueue<std::function<void()>> task_queue;

    /// Mutex for protecting access to the task queue.
    std::mutex queue_mutex;

    /// Condition variable for synchronizing thread activities.
    std::condition_variable condition;

    /// Atomic flag to signal threads to stop.
    std::atomic_flag stop_flag;
};