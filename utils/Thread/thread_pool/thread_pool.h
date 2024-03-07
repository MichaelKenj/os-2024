#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include "../blocking_queue/blocking_queue.h"

class ThreadPool {
public:
    ThreadPool(std::size_t num_threads = std::thread::hardware_concurrency(), size_t queue_capacity = 10)
            : stop(false), task_queue(queue_capacity) {

        num_threads = (num_threads == 0) ? 2 : num_threads;

        for (size_t i = 0; i < num_threads; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex);
                        condition.wait(lock, [this] { return stop || !task_queue.empty(); });

                        if (stop && task_queue.empty()) {
                            return;
                        }

                        task = std::move(task_queue.pop());
                    }

                    task();
                }
            });
        }
    }

    template <class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
        using return_type = decltype(f(args...));
        auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        {
            std::unique_lock<std::mutex> lock(queue_mutex);

            if (stop) {
                throw std::runtime_error("ThreadPool is stopped. Cannot enqueue more tasks.");
            }

            task_queue.push([task]() { (*task)(); });
        }

        condition.notify_one();
        return task->get_future();
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }

        condition.notify_all();

        for (std::thread &worker : workers) {
            worker.join();
        }
    }

private:
    std::vector<std::thread> workers;
    LimitedBlockingQueue<std::function<void()>> task_queue;

    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};
