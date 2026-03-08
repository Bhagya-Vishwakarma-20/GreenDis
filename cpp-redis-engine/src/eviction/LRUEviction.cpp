#include "LRUEviction.h"
#include "../utils/Logger.h"
#include <vector>
#include <algorithm>

namespace redis_engine::eviction {

void LRUEviction::Evict(storage::MemoryStore& store, size_t max_keys) {
    if (store.Size() <= max_keys) {
        return;
    }

    // In a real Redis, approximated LRU is used (sampling 5 keys).
    // For this engine, we will do a full scan or sample scan to find the oldest.
    // Full scan is slow but simple to implement correctly first.
    // Let's sample 10% or just 50 keys to keep it fast, like Redis.

    int keys_to_evict = store.Size() - max_keys;
    if (keys_to_evict <= 0) return;

    auto all_keys = store.GetAllKeys();
    
    // Since we need access time, we fetch snapshot for keys.
    struct KeyAccess {
        std::string key;
        storage::TimePoint last_accessed;
    };

    std::vector<KeyAccess> accesses;
    accesses.reserve(all_keys.size());

    for (const auto& k : all_keys) {
        storage::Entry entry("");
        if (store.GetEntrySnapshot(k, entry)) {
            accesses.push_back({k, entry.last_accessed});
        }
    }

    // Sort by oldest first
    std::sort(accesses.begin(), accesses.end(), 
        [](const KeyAccess& a, const KeyAccess& b) {
            return a.last_accessed < b.last_accessed;
        });

    int evicted = 0;
    for (const auto& ka : accesses) {
        if (evicted >= keys_to_evict) break;
        if (store.Delete(ka.key)) {
            LOG_TRACE("Evicted key due to LRU: {}", ka.key);
            evicted++;
        }
    }
}

} // namespace redis_engine::eviction
