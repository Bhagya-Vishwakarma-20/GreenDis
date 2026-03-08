#pragma once

#include <asio.hpp>
#include <memory>
#include <string>
#include <functional>

namespace redis_engine::server {

class Connection : public std::enable_shared_from_this<Connection> {
public:
    using MessageHandler = std::function<void(std::shared_ptr<Connection>, const std::string&)>;

    explicit Connection(asio::ip::tcp::socket socket, MessageHandler handler);
    
    void Start();
    void Send(const std::string& message);
    void Close();

private:
    void DoRead();
    void DoWrite();

    asio::ip::tcp::socket socket_;
    MessageHandler handler_;
    
    // asio::streambuf read_buffer_; // For delimiter based
    std::string read_msg_;
    std::string write_msg_;
    
    asio::streambuf buffer_;
};

} // namespace redis_engine::server
