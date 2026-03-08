#include <gtest/gtest.h>
#include "../src/services/KeyValueService.h"
#include "../src/storage/MemoryStore.h"
#include "../src/eviction/LRUEviction.h"

using namespace redis_engine::services;
using namespace redis_engine::storage;
using namespace redis_engine::eviction;

TEST(ServiceTest, KVSetAndEvict) {
    auto store = std::make_shared<MemoryStore>();
    auto evictor = std::make_shared<LRUEviction>();
    
    // Max 2 keys
    KeyValueService service(store, evictor, 2);
    
    service.Set("k1", "v1");
    service.Set("k2", "v2");
    service.Set("k3", "v3");
    
    EXPECT_EQ(store->Size(), 2);
    // k1 should be evicted
    EXPECT_FALSE(service.Get("k1").has_value());
    EXPECT_TRUE(service.Get("k2").has_value());
    EXPECT_TRUE(service.Get("k3").has_value());
}

TEST(ServiceTest, ExpirationService) {
    auto store = std::make_shared<MemoryStore>();
    KeyValueService kv(store, nullptr, 100);
    ExpirationService exp(store);
    
    kv.Set("key1", "val");
    EXPECT_TRUE(exp.Expire("key1", 0)); // TTL 0 implies delete
    EXPECT_FALSE(kv.Get("key1").has_value());
}
