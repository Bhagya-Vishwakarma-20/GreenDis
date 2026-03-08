#include <gtest/gtest.h>
#include "../src/persistence/AOFWriter.h"
#include "../src/persistence/SnapshotManager.h"
#include "../src/storage/MemoryStore.h"
#include <cstdio>

using namespace redis_engine::persistence;
using namespace redis_engine::storage;

class PersistenceTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::remove("test.aof");
        std::remove("test.rdb");
    }
    void TearDown() override {
        std::remove("test.aof");
        std::remove("test.rdb");
    }
};

TEST_F(PersistenceTest, AOFWriteAndReplay) {
    {
        AOFWriter aof("test.aof");
        aof.AppendSet("k1", "v1");
        aof.AppendSet("k2", "v2");
        aof.AppendDel("k1");
    } // Flush and close

    auto store = std::make_shared<MemoryStore>();
    RecoveryManager::Recover(store, "nonexistent.rdb", "test.aof");
    
    EXPECT_FALSE(store->Get("k1").has_value());
    EXPECT_TRUE(store->Get("k2").has_value());
    EXPECT_EQ(store->Get("k2").value(), "v2");
}

TEST_F(PersistenceTest, SnapshotSaveAndLoad) {
    auto store1 = std::make_shared<MemoryStore>();
    store1->Set("key1", "val1");
    store1->Set("key2", "val2");
    store1->Expire("key2", std::chrono::seconds(100)); // Should survive load
    
    SnapshotManager snap(store1, "test.rdb", std::chrono::seconds(100));
    EXPECT_TRUE(snap.TakeSnapshot());
    
    auto store2 = std::make_shared<MemoryStore>();
    RecoveryManager::Recover(store2, "test.rdb", "nonexistent.aof");
    
    EXPECT_EQ(store2->Size(), 2);
    EXPECT_EQ(store2->Get("key1").value(), "val1");
    EXPECT_EQ(store2->Get("key2").value(), "val2");
}
