#include <gtest/gtest.h>
#include "../src/storage/MemoryStore.h"
#include "../src/eviction/TTLManager.h"
#include "../src/eviction/LRUEviction.h"
#include <thread>
#include <chrono>

using namespace redis_engine::storage;
using namespace redis_engine::eviction;

class StorageTest : public ::testing::Test {
protected:
    void SetUp() override {
        store = std::make_shared<MemoryStore>();
    }
    std::shared_ptr<MemoryStore> store;
};

TEST_F(StorageTest, BasicSetGet) {
    store->Set("key1", "value1");
    auto val = store->Get("key1");
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(val.value(), "value1");
    
    // Non-existent key
    EXPECT_FALSE(store->Get("key2").has_value());
}

TEST_F(StorageTest, DeleteKey) {
    store->Set("key1", "value1");
    EXPECT_TRUE(store->Delete("key1"));
    EXPECT_FALSE(store->Get("key1").has_value());
    EXPECT_FALSE(store->Delete("key1")); // Second delete fails
}

TEST_F(StorageTest, ExpireLazy) {
    store->Set("key1", "value1");
    store->Expire("key1", std::chrono::seconds(1));
    
    // Should still exist
    EXPECT_TRUE(store->Get("key1").has_value());
    
    // Simulate time passing (lazy expire test is hard without sleep, so we sleep)
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    
    // Now it should be expired and return nullopt
    EXPECT_FALSE(store->Get("key1").has_value());
}

TEST_F(StorageTest, TTLManagerPurge) {
    store->Set("key1", "val1");
    store->Expire("key1", std::chrono::seconds(0));
    
    TTLManager ttl(store, std::chrono::milliseconds(20));
    ttl.Start();
    
    EXPECT_EQ(store->Size(), 1);
    
    // Wait enough time for background thread to purge it
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    
    EXPECT_EQ(store->Size(), 0);
    ttl.Stop();
}

TEST_F(StorageTest, LRUEviction) {
    LRUEviction lru;
    
    store->Set("k1", "v1");
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    store->Set("k2", "v2");
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    store->Set("k3", "v3");
    
    EXPECT_EQ(store->Size(), 3);
    
    // Evict down to 2 max items
    lru.Evict(*store, 2);
    
    EXPECT_EQ(store->Size(), 2);
    // k1 is the oldest, so it should be evicted
    EXPECT_FALSE(store->Get("k1").has_value());
    EXPECT_TRUE(store->Get("k2").has_value());
    EXPECT_TRUE(store->Get("k3").has_value());
    
    // Now access k2 so k3 becomes oldest
    store->Get("k2");
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    
    // Evict down to 1 item
    lru.Evict(*store, 1);
    EXPECT_EQ(store->Size(), 1);
    // k3 should be evicted since k2 was accessed
    EXPECT_FALSE(store->Get("k3").has_value());
    EXPECT_TRUE(store->Get("k2").has_value());
}
