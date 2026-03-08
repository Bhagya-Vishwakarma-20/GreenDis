#pragma once

#include "Entry.h"
#include <unordered_map>
#include <string>
#include <shared_mutex>
#include <optional>
#include <vector>

namespace redis_engine::storage {

class MemoryStore {
public:
    MemoryStore() = default;

    // Core operations
    void Set(const std::string& key, const std::string& value);
    std::optional<std::string> Get(const std::string& key);
    bool Delete(const std::string& key);
    bool Exists(const std::string& key) const;

    // Expiration operations
    bool Expire(const std::string& key, std::chrono::seconds ttl);
    
    // Internal state management
    size_t Size() const;
    void Clear();
    
    // Get all keys (useful for snapshot/replication)
    std::vector<std::string> GetAllKeys() const;

    // Direct entry access for advanced persistence, needs explicit locking outside if not careful,
    // usually we just return a snapshot or use internal iterators, but mapping GetAllKeys is safe enough
    bool GetEntrySnapshot(const std::string& key, Entry& out_entry) const;

private:
    std::unordered_map<std::string, Entry> table_;
    mutable std::shared_mutex mutex_;
};

} // namespace redis_engine::storage
