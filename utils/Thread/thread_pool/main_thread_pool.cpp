#include "thread_pool.h"

void demo1()
{

    ThreadPool pool;

    // Enqueue tasks to the thread pool
    auto task1 = pool.enqueue([] { std::cout << "Task 1 executed\n"; });
    auto task2 = pool.enqueue([] { std::cout << "Task 2 executed\n"; });

    // Wait for tasks to complete
    task1.get();
    task2.get();
    std::cout << "-----------------------------------------\n";

}

void demo2()
{
    ThreadPool pool(4);
    for(int i = 0; i < 5; ++i)
    {
        pool.enqueue([i]{
            std::cout << "Task " << i << " is running on thread "
                      << std::this_thread::get_id() << std::endl;
            std::this_thread::sleep_for(
                    std::chrono::milliseconds(100));

        });
    }

}

int main()
{
    demo1();
    demo2();
    return 0;
}
