#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <vector>

namespace redis_engine::persistence {

class AOFWriter {
public:
    explicit AOFWriter(const std::string& filepath);
    ~AOFWriter();

    void AppendSet(const std::string& key, const std::string& value);
    void AppendDel(const std::string& key);
    void AppendExpire(const std::string& key, int seconds);

    // Forces a flush to disk
    void Sync();

private:
    void WriteCommand(const std::string& cmd);

    std::string filepath_;
    std::ofstream file_;
    std::mutex mutex_;
};

} // namespace redis_engine::persistence
