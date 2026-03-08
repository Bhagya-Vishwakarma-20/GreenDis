#include "TTLManager.h"
#include "../utils/Logger.h"

namespace redis_engine::eviction {

TTLManager::TTLManager(std::shared_ptr<storage::MemoryStore> store, std::chrono::milliseconds interval)
    : store_(std::move(store)), interval_(interval) {}

TTLManager::~TTLManager() {
    Stop();
}

void TTLManager::Start() {
    if (running_.exchange(true)) return;
    worker_ = std::thread(&TTLManager::Run, this);
    LOG_INFO("TTLManager background thread started.");
}

void TTLManager::Stop() {
    if (!running_.exchange(false)) return;
    if (worker_.joinable()) {
        worker_.join();
    }
    LOG_INFO("TTLManager background thread stopped.");
}

void TTLManager::Run() {
    while (running_) {
        PurgeExpiredKeys();
        std::this_thread::sleep_for(interval_);
    }
}

void TTLManager::PurgeExpiredKeys() {
    // Like Redis, we can sample keys to find expired ones instead of full scan.
    // For simplicity, we get all keys and check. This is O(N).
    // In production, we'd maintain a priority queue of expirations.
    // For this engine limit, we'll iterate a subset or full scan safely.
    
    auto keys = store_->GetAllKeys();
    int expired_count = 0;
    
    for (const auto& key : keys) {
        storage::Entry entry("");
        if (store_->GetEntrySnapshot(key, entry)) {
            if (entry.IsExpired()) {
                if (store_->Delete(key)) {
                    expired_count++;
                }
            }
        }
    }

    if (expired_count > 0) {
        LOG_TRACE("Purged {} expired keys implicitly.", expired_count);
    }
}

} // namespace redis_engine::eviction
