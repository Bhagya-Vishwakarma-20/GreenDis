#include "SnapshotManager.h"
#include "../utils/Logger.h"
#include <fstream>
#include <sstream>

namespace redis_engine::persistence {

SnapshotManager::SnapshotManager(std::shared_ptr<storage::MemoryStore> store, 
                                 const std::string& filepath,
                                 std::chrono::seconds interval)
    : store_(std::move(store)), filepath_(filepath), interval_(interval) {}

SnapshotManager::~SnapshotManager() {
    Stop();
}

void SnapshotManager::Start() {
    if (running_.exchange(true)) return;
    worker_ = std::thread(&SnapshotManager::Run, this);
    LOG_INFO("SnapshotManager background thread started. Interval: {}s", interval_.count());
}

void SnapshotManager::Stop() {
    if (!running_.exchange(false)) return;
    if (worker_.joinable()) {
        worker_.join();
    }
    LOG_INFO("SnapshotManager background thread stopped.");
}

void SnapshotManager::Run() {
    while (running_) {
        // Sleep in small increments to allow quick shutdown
        for (int i = 0; i < interval_.count() * 10 && running_; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        if (running_) {
            TakeSnapshot();
        }
    }
}

bool SnapshotManager::TakeSnapshot() {
    LOG_INFO("Taking snapshot to {}", filepath_);
    
    // Write to a temporary file first, then rename for atomicity
    std::string temp_filepath = filepath_ + ".tmp";
    std::ofstream file(temp_filepath, std::ios::out | std::ios::binary);
    
    if (!file.is_open()) {
        LOG_ERROR("Failed to open temp snapshot file: {}", temp_filepath);
        return false;
    }

    auto keys = store_->GetAllKeys();
    size_t count = 0;

    for (const auto& key : keys) {
        storage::Entry entry("");
        if (store_->GetEntrySnapshot(key, entry)) {
            if (!entry.IsExpired()) {
                // Simple format: KEY_LEN KEY VAL_LEN VAL HAS_TTL TTL
                uint32_t klen = key.size();
                uint32_t vlen = entry.value.size();
                file.write(reinterpret_cast<const char*>(&klen), sizeof(klen));
                file.write(key.data(), klen);
                
                file.write(reinterpret_cast<const char*>(&vlen), sizeof(vlen));
                file.write(entry.value.data(), vlen);
                
                uint8_t has_ttl = entry.has_ttl ? 1 : 0;
                file.write(reinterpret_cast<const char*>(&has_ttl), sizeof(has_ttl));
                
                if (has_ttl) {
                    auto duration = entry.expiration.time_since_epoch().count();
                    file.write(reinterpret_cast<const char*>(&duration), sizeof(duration));
                }
                count++;
            }
        }
    }
    
    file.close();
    
    if (std::rename(temp_filepath.c_str(), filepath_.c_str()) != 0) {
        LOG_ERROR("Failed to rename temp snapshot to target: {}", filepath_);
        return false;
    }

    LOG_INFO("Snapshot completed successfully. Saved {} keys.", count);
    return true;
}

// ---------------- RecoveryManager ----------------

bool RecoveryManager::Recover(std::shared_ptr<storage::MemoryStore> store,
                              const std::string& rdb_filepath,
                              const std::string& aof_filepath) {
    bool rdb_success = LoadSnapshot(store, rdb_filepath);
    bool aof_success = ReplayAOF(store, aof_filepath);
    return rdb_success || aof_success;
}

bool RecoveryManager::LoadSnapshot(std::shared_ptr<storage::MemoryStore> store, const std::string& filepath) {
    std::ifstream file(filepath, std::ios::in | std::ios::binary);
    if (!file.is_open()) return false;

    LOG_INFO("Loading RDB snapshot from {}", filepath);
    size_t loaded = 0;

    while (file.peek() != EOF) {
        uint32_t klen;
        if (!file.read(reinterpret_cast<char*>(&klen), sizeof(klen))) break;
        
        std::string key(klen, '\0');
        file.read(&key[0], klen);
        
        uint32_t vlen;
        file.read(reinterpret_cast<char*>(&vlen), sizeof(vlen));
        
        std::string val(vlen, '\0');
        file.read(&val[0], vlen);
        
        uint8_t has_ttl;
        file.read(reinterpret_cast<char*>(&has_ttl), sizeof(has_ttl));
        
        store->Set(key, val);
        
        if (has_ttl) {
            decltype(storage::TimePoint().time_since_epoch().count()) duration;
            file.read(reinterpret_cast<char*>(&duration), sizeof(duration));
            
            storage::TimePoint expiration{std::chrono::steady_clock::duration(duration)};
            auto now = std::chrono::steady_clock::now();
            
            if (expiration > now) {
                auto ttl_sec = std::chrono::duration_cast<std::chrono::seconds>(expiration - now);
                store->Expire(key, ttl_sec);
            } else {
                store->Delete(key); // Already expired
            }
        }
        loaded++;
    }

    LOG_INFO("Loaded {} keys from RDB.", loaded);
    return true;
}

bool RecoveryManager::ReplayAOF(std::shared_ptr<storage::MemoryStore> store, const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) return false;

    LOG_INFO("Replaying AOF from {}", filepath);
    std::string line;
    size_t cmds = 0;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        if (cmd == "SET") {
            std::string key, val;
            iss >> key;
            // Value might have spaces, so we read the rest
            std::getline(iss, val);
            if (!val.empty() && val[0] == ' ') val = val.substr(1);
            store->Set(key, val);
        } else if (cmd == "DEL") {
            std::string key;
            iss >> key;
            store->Delete(key);
        } else if (cmd == "EXPIRE") {
            std::string key;
            int ttl;
            iss >> key >> ttl;
            store->Expire(key, std::chrono::seconds(ttl));
        }
        cmds++;
    }

    LOG_INFO("Replayed {} commands from AOF.", cmds);
    return true;
}

} // namespace redis_engine::persistence
