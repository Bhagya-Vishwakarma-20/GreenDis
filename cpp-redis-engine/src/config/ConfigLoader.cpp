#include "ConfigLoader.h"
#include "../utils/Logger.h"
#include <nlohmann/json.hpp>
#include <fstream>

namespace redis_engine::config {

ServerConfig ConfigLoader::Load(const std::string& config_file_path) {
    ServerConfig config;
    
    std::ifstream file(config_file_path);
    if (!file.is_open()) {
        LOG_WARN("Could not open config file: {}, using defaults.", config_file_path);
        return config;
    }

    try {
        nlohmann::json j;
        file >> j;
        
        if (j.contains("port")) config.port = j["port"];
        if (j.contains("max_memory")) config.max_memory = j["max_memory"];
        if (j.contains("thread_pool_size")) config.thread_pool_size = j["thread_pool_size"];
        if (j.contains("snapshot_interval")) config.snapshot_interval = j["snapshot_interval"];
        if (j.contains("aof_file_path")) config.aof_file_path = j["aof_file_path"];
        if (j.contains("rdb_file_path")) config.rdb_file_path = j["rdb_file_path"];
        
        LOG_INFO("Configuration loaded from {}", config_file_path);
    } catch (const nlohmann::json::exception& e) {
        LOG_ERROR("JSON parse error in config file {}: {}", config_file_path, e.what());
    }
    
    return config;
}

} // namespace redis_engine::config
