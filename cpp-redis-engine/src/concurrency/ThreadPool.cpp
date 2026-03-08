#include "ThreadPool.h"
#include "../utils/Logger.h"

namespace redis_engine::concurrency {

ThreadPool::ThreadPool(size_t num_threads) {
    LOG_INFO("Initializing ThreadPool with {} threads", num_threads);
    for (size_t i = 0; i < num_threads; ++i) {
        workers_.emplace_back([this] { WorkerThread(); });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        stop_ = true;
    }
    condition_.notify_all();
    for (std::thread& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    LOG_INFO("ThreadPool shut down.");
}

void ThreadPool::Enqueue(std::function<void()> task) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        if (stop_) {
            LOG_WARN("Enqueue called on stopped ThreadPool");
            return;
        }
        tasks_.push(std::move(task));
    }
    condition_.notify_one();
}

void ThreadPool::WorkerThread() {
    while (true) {
        std::function<void()> task;
        
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            condition_.wait(lock, [this] { return stop_ || !tasks_.empty(); });
            
            if (stop_ && tasks_.empty()) {
                return;
            }
            
            task = std::move(tasks_.front());
            tasks_.pop();
        }
        
        // Execute the task
        try {
            task();
        } catch (const std::exception& e) {
            LOG_ERROR("Exception in ThreadPool worker: {}", e.what());
        } catch (...) {
            LOG_ERROR("Unknown exception in ThreadPool worker");
        }
    }
}

} // namespace redis_engine::concurrency
