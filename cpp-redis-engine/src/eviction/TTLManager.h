#pragma once

#include "../storage/MemoryStore.h"
#include <memory>
#include <thread>
#include <atomic>
#include <chrono>

namespace redis_engine::eviction {

class TTLManager {
public:
    explicit TTLManager(std::shared_ptr<storage::MemoryStore> store, std::chrono::milliseconds interval = std::chrono::milliseconds(100));
    ~TTLManager();

    void Start();
    void Stop();

private:
    void Run();
    void PurgeExpiredKeys();

    std::shared_ptr<storage::MemoryStore> store_;
    std::chrono::milliseconds interval_;
    std::atomic<bool> running_{false};
    std::thread worker_;
};

} // namespace redis_engine::eviction
