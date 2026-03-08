#pragma once

#include <string>
#include <cstdint>

namespace redis_engine::config {

struct ServerConfig {
    uint16_t port{6379};
    size_t max_memory{1024 * 1024 * 1024}; // 1GB default
    size_t thread_pool_size{4};
    size_t snapshot_interval{300};         // seconds
    std::string aof_file_path{"appendonly.aof"};
    std::string rdb_file_path{"dump.rdb"};
};

class ConfigLoader {
public:
    static ServerConfig Load(const std::string& config_file_path);
};

} // namespace redis_engine::config
