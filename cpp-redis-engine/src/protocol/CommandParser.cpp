#include "CommandParser.h"
#include <sstream>
#include <algorithm>
#include <cctype>

namespace redis_engine::protocol {

std::string Response::Serialize() const {
    if (is_error) {
        return "ERROR " + data + "\n";
    }
    // Simple text response: OK or the value followed by newline
    // Real Redis uses RESP (e.g. +OK\r\n or $5\r\nvalue\r\n). We stick to simple text as per spec.
    return data + "\n";
}

Request CommandParser::Parse(const std::string& raw_input) {
    Request req;
    if (raw_input.empty()) return req;

    std::istringstream iss(raw_input);
    std::string token;
    
    if (iss >> token) {
        // Convert command to uppercase for case-insensitivity
        std::transform(token.begin(), token.end(), token.begin(), 
            [](unsigned char c){ return std::toupper(c); });
        req.command = token;
        
        while (iss >> token) {
            req.args.push_back(token);
        }
    }
    
    return req;
}

} // namespace redis_engine::protocol
