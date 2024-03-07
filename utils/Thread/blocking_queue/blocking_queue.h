#ifndef BLOCKING_QUEUE_BLOCKING_QUEUE_H
#define BLOCKING_QUEUE_BLOCKING_QUEUE_H


template <typename T>
class LimitedBlockingQueue {
public:
    explicit LimitedBlockingQueue(size_t capacity)
            : capacity_(capacity) {}

    void push(const T& item) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            notFullCV_.wait(lock, [&] { return queue_.size() < capacity_; });

            queue_.push(item);
        }
        notEmptyCV_.notify_one();
    }

    T pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        notEmptyCV_.wait(lock, [this] { return !queue_.empty(); });
        T item = queue_.front();
        queue_.pop();
        notFullCV_.notify_one();
        return item;
    }

    bool empty() const {
        std::unique_lock<std::mutex> lock(mutex_);
        return queue_.empty();
    }

private:
    size_t capacity_;
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable notEmptyCV_;
    std::condition_variable notFullCV_;
};

#endif //BLOCKING_QUEUE_BLOCKING_QUEUE_H