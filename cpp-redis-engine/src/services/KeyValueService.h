#pragma once

#include "../storage/MemoryStore.h"
#include "../eviction/LRUEviction.h"
#include <memory>
#include <optional>
#include <string>

namespace redis_engine::services {

class KeyValueService {
public:
    KeyValueService(std::shared_ptr<storage::MemoryStore> store, 
                    std::shared_ptr<eviction::LRUEviction> evictor,
                    size_t max_keys);

    void Set(const std::string& key, const std::string& value);
    std::optional<std::string> Get(const std::string& key);
    bool Delete(const std::string& key);

private:
    std::shared_ptr<storage::MemoryStore> store_;
    std::shared_ptr<eviction::LRUEviction> evictor_;
    size_t max_keys_;
};


class ExpirationService {
public:
    explicit ExpirationService(std::shared_ptr<storage::MemoryStore> store);

    bool Expire(const std::string& key, int seconds);

private:
    std::shared_ptr<storage::MemoryStore> store_;
};

} // namespace redis_engine::services
