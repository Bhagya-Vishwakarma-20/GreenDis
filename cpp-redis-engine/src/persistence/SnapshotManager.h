#pragma once

#include "../storage/MemoryStore.h"
#include "AOFWriter.h"
#include <string>
#include <memory>
#include <thread>
#include <atomic>
#include <chrono>

namespace redis_engine::persistence {

class SnapshotManager {
public:
    SnapshotManager(std::shared_ptr<storage::MemoryStore> store, 
                    const std::string& filepath,
                    std::chrono::seconds interval = std::chrono::seconds(300));
    ~SnapshotManager();

    void Start();
    void Stop();

    // Trigger manual snapshot
    bool TakeSnapshot();

private:
    void Run();

    std::shared_ptr<storage::MemoryStore> store_;
    std::string filepath_;
    std::chrono::seconds interval_;
    std::atomic<bool> running_{false};
    std::thread worker_;
};


class RecoveryManager {
public:
    static bool Recover(std::shared_ptr<storage::MemoryStore> store,
                        const std::string& rdb_filepath,
                        const std::string& aof_filepath);
private:
    static bool LoadSnapshot(std::shared_ptr<storage::MemoryStore> store, const std::string& filepath);
    static bool ReplayAOF(std::shared_ptr<storage::MemoryStore> store, const std::string& filepath);
};

} // namespace redis_engine::persistence
