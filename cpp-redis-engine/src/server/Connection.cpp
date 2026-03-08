#include "Connection.h"
#include "../utils/Logger.h"
#include <iostream>

namespace redis_engine::server {

Connection::Connection(asio::ip::tcp::socket socket, MessageHandler handler)
    : socket_(std::move(socket)), handler_(std::move(handler)) {}

void Connection::Start() {
    LOG_INFO("New connection from {}", socket_.remote_endpoint().address().to_string());
    DoRead();
}

void Connection::Send(const std::string& message) {
    bool write_in_progress = !write_msg_.empty();
    write_msg_ += message;
    if (!write_in_progress) {
        DoWrite();
    }
}

void Connection::Close() {
    asio::error_code ec;
    socket_.close(ec);
    if (ec) {
        LOG_WARN("Error closing socket: {}", ec.message());
    }
}

void Connection::DoRead() {
    auto self(shared_from_this());
    
    // Read until newline (simple line-based protocol)
    asio::async_read_until(socket_, buffer_, '\n',
        [this, self](std::error_code ec, std::size_t /*length*/) {
            if (!ec) {
                std::istream is(&buffer_);
                std::string line;
                std::getline(is, line);
                
                // Handle CR if present
                if (!line.empty() && line.back() == '\r') {
                    line.pop_back();
                }

                // Pass to handler
                if (handler_) {
                    handler_(self, line);
                }

                DoRead(); // Keep reading
            } else if (ec != asio::error::operation_aborted) {
                LOG_INFO("Connection closed by peer or read error: {}", ec.message());
                Close();
            }
        });
}

void Connection::DoWrite() {
    auto self(shared_from_this());
    
    // For simplicity, we create a copy to write and clear the pending buffer.
    // In high perf, use scatter-gather or queues.
    auto out_msg = std::make_shared<std::string>(std::move(write_msg_));
    write_msg_.clear();

    asio::async_write(socket_, asio::buffer(*out_msg),
        [this, self, out_msg](std::error_code ec, std::size_t /*length*/) {
            if (!ec) {
                if (!write_msg_.empty()) {
                    DoWrite();
                }
            } else {
                LOG_ERROR("Write error: {}", ec.message());
                Close();
            }
        });
}

} // namespace redis_engine::server
