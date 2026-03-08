#pragma once

#include <string>
#include <chrono>
#include <atomic>

namespace redis_engine::storage {

using TimePoint = std::chrono::steady_clock::time_point;

struct Entry {
    std::string value;
    TimePoint expiration;
    bool has_ttl{false};
    TimePoint last_accessed; // For LRU

    Entry(const std::string& val) 
        : value(val), 
          last_accessed(std::chrono::steady_clock::now()) {}

    void UpdateAccessTime() {
        last_accessed = std::chrono::steady_clock::now();
    }

    bool IsExpired() const {
        if (!has_ttl) return false;
        return std::chrono::steady_clock::now() > expiration;
    }
};

} // namespace redis_engine::storage
