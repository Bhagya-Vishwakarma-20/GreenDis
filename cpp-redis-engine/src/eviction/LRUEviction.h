#pragma once

#include "../storage/MemoryStore.h"
#include <memory>
#include <thread>
#include <atomic>

namespace redis_engine::eviction {

class EvictionPolicy {
public:
    virtual ~EvictionPolicy() = default;
    
    // Evicts keys until current memory or size is below threshold
    virtual void Evict(storage::MemoryStore& store, size_t max_keys) = 0;
};

class LRUEviction : public EvictionPolicy {
public:
    LRUEviction() = default;
    
    void Evict(storage::MemoryStore& store, size_t max_keys) override;
};

} // namespace redis_engine::eviction
