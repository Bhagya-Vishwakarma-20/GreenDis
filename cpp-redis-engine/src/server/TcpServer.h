#pragma once

#include "Connection.h"
#include <asio.hpp>
#include <memory>
#include <cstdint>
#include <thread>
#include <vector>

namespace redis_engine::server {

class TcpServer {
public:
    TcpServer(uint16_t port, Connection::MessageHandler handler, size_t io_threads = 1);
    ~TcpServer();

    void Start();
    void Stop();

private:
    void DoAccept();

    asio::io_context io_context_;
    asio::ip::tcp::acceptor acceptor_;
    Connection::MessageHandler handler_;
    
    size_t num_threads_;
    std::vector<std::thread> thread_pool_;
};

} // namespace redis_engine::server
