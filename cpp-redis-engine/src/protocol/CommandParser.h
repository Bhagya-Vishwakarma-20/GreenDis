#pragma once

#include <string>
#include <vector>

namespace redis_engine::protocol {

struct Request {
    std::string command;
    std::vector<std::string> args;
    
    bool IsEmpty() const { return command.empty(); }
};

struct Response {
    std::string data;
    bool is_error{false};
    
    std::string Serialize() const;
};

class CommandParser {
public:
    static Request Parse(const std::string& raw_input);
};

} // namespace redis_engine::protocol
