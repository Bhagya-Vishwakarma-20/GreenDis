#include "AOFWriter.h"
#include "../utils/Logger.h"

namespace redis_engine::persistence {

AOFWriter::AOFWriter(const std::string& filepath) : filepath_(filepath) {
    file_.open(filepath_, std::ios::out | std::ios::app);
    if (!file_.is_open()) {
        LOG_ERROR("Failed to open AOF file for writing: {}", filepath_);
    } else {
        LOG_INFO("AOFWriter initialized at {}", filepath_);
    }
}

AOFWriter::~AOFWriter() {
    Sync();
    if (file_.is_open()) {
        file_.close();
    }
}

void AOFWriter::WriteCommand(const std::string& cmd) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (file_.is_open()) {
        file_ << cmd << "\n";
    }
}

void AOFWriter::AppendSet(const std::string& key, const std::string& value) {
    // Basic text protocol for AOF just to keep it simple and human-readable.
    // E.g., SET key value
    WriteCommand("SET " + key + " " + value);
}

void AOFWriter::AppendDel(const std::string& key) {
    WriteCommand("DEL " + key);
}

void AOFWriter::AppendExpire(const std::string& key, int seconds) {
    WriteCommand("EXPIRE " + key + " " + std::to_string(seconds));
}

void AOFWriter::Sync() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (file_.is_open()) {
        file_.flush();
    }
}

} // namespace redis_engine::persistence
