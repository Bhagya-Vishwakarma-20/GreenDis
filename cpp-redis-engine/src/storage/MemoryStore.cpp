#include "MemoryStore.h"
#include <mutex>

namespace redis_engine::storage {

void MemoryStore::Set(const std::string& key, const std::string& value) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    auto it = table_.find(key);
    if (it != table_.end()) {
        it->second.value = value;
        it->second.UpdateAccessTime();
        // NOTE: we retain the existing TTL behavior similar to Redis unless specified
        // But typical SET overrides TTL. For simplicity here, we override TTL.
        it->second.has_ttl = false; 
    } else {
        table_.emplace(key, Entry(value));
    }
}

std::optional<std::string> MemoryStore::Get(const std::string& key) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    auto it = table_.find(key);
    if (it == table_.end()) {
        return std::nullopt;
    }

    if (it->second.IsExpired()) {
        // We technically shouldn't delete in a shared_lock.
        // Lazy expiration happens here but we just return nullopt.
        // The background eviction thread handles actual cleanup, or we upgrade lock.
        // For safe concurrency, just return nullopt.
        return std::nullopt;
    }

    // Access time update needs atomics or write lock, but this breaks shared_lock.
    // For a real high-perf system, last_accessed is atomic or approximated.
    // Since it's std::chrono::time_point, we can't easily make it std::atomic out of the box. 
    // We will bypass updating LRU on GET temporarily unless thread-safe, or upgrade lock.
    // Given the prompt "track key usage", we might need a write lock for GET if we want exact LRU,
    // or we accept an approximated LRU updated only on SET/EXPIRE for now. Let's cast away const
    // and rely on a lightweight mutex just for LRU in the entry? No, data race.
    // To fix: require unique_lock for GET if updating LRU. We will do unique_lock for now to be safe.
    lock.unlock(); 

    std::unique_lock<std::shared_mutex> write_lock(mutex_);
    // re-check because state might have changed during lock upgrade
    auto write_it = table_.find(key);
    if (write_it == table_.end() || write_it->second.IsExpired()) {
        return std::nullopt;
    }
    
    write_it->second.UpdateAccessTime();
    return write_it->second.value;
}

bool MemoryStore::Delete(const std::string& key) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    return table_.erase(key) > 0;
}

bool MemoryStore::Exists(const std::string& key) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    auto it = table_.find(key);
    if (it == table_.end() || it->second.IsExpired()) {
        return false;
    }
    return true;
}

bool MemoryStore::Expire(const std::string& key, std::chrono::seconds ttl) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    auto it = table_.find(key);
    if (it != table_.end()) {
        it->second.has_ttl = true;
        it->second.expiration = std::chrono::steady_clock::now() + ttl;
        it->second.UpdateAccessTime();
        return true;
    }
    return false;
}

size_t MemoryStore::Size() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return table_.size();
}

void MemoryStore::Clear() {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    table_.clear();
}

std::vector<std::string> MemoryStore::GetAllKeys() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    std::vector<std::string> keys;
    keys.reserve(table_.size());
    for (const auto& [k, v] : table_) {
        keys.push_back(k);
    }
    return keys;
}

bool MemoryStore::GetEntrySnapshot(const std::string& key, Entry& out_entry) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    auto it = table_.find(key);
    if (it != table_.end()) {
        out_entry = it->second;
        return true;
    }
    return false;
}

} // namespace redis_engine::storage
