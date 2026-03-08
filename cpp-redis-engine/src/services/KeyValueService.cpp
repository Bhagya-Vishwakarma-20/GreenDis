#include "KeyValueService.h"
#include "../utils/Logger.h"

namespace redis_engine::services {

KeyValueService::KeyValueService(std::shared_ptr<storage::MemoryStore> store, 
                                 std::shared_ptr<eviction::LRUEviction> evictor,
                                 size_t max_keys)
    : store_(std::move(store)), evictor_(std::move(evictor)), max_keys_(max_keys) {}

void KeyValueService::Set(const std::string& key, const std::string& value) {
    store_->Set(key, value);
    
    // Check if we need to evict
    if (evictor_ && store_->Size() > max_keys_) {
        evictor_->Evict(*store_, max_keys_);
    }
}

std::optional<std::string> KeyValueService::Get(const std::string& key) {
    return store_->Get(key);
}

bool KeyValueService::Delete(const std::string& key) {
    return store_->Delete(key);
}

ExpirationService::ExpirationService(std::shared_ptr<storage::MemoryStore> store)
    : store_(std::move(store)) {}

bool ExpirationService::Expire(const std::string& key, int seconds) {
    if (seconds <= 0) {
        return store_->Delete(key);
    }
    return store_->Expire(key, std::chrono::seconds(seconds));
}

} // namespace redis_engine::services
