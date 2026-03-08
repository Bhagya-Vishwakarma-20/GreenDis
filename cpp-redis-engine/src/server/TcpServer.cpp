#include "TcpServer.h"
#include "../utils/Logger.h"

namespace redis_engine::server {

TcpServer::TcpServer(uint16_t port, Connection::MessageHandler handler, size_t io_threads)
    : acceptor_(io_context_, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
      handler_(std::move(handler)),
      num_threads_(io_threads) {
    LOG_INFO("TcpServer configured on port {}", port);
}

TcpServer::~TcpServer() {
    Stop();
}

void TcpServer::Start() {
    LOG_INFO("Starting TcpServer with {} IO threads", num_threads_);
    DoAccept();

    for (size_t i = 0; i < num_threads_; ++i) {
        thread_pool_.emplace_back([this]() {
            try {
                io_context_.run();
            } catch (std::exception& e) {
                LOG_ERROR("Exception in IO thread: {}", e.what());
            }
        });
    }
}

void TcpServer::Stop() {
    io_context_.stop();
    for (auto& t : thread_pool_) {
        if (t.joinable()) {
            t.join();
        }
    }
    LOG_INFO("TcpServer stopped.");
}

void TcpServer::DoAccept() {
    acceptor_.async_accept(
        [this](std::error_code ec, asio::ip::tcp::socket socket) {
            if (!ec) {
                LOG_INFO("Accepted new connection.");
                auto conn = std::make_shared<Connection>(std::move(socket), handler_);
                conn->Start();
            } else {
                LOG_ERROR("Accept error: {}", ec.message());
            }
            
            // Continue accepting
            DoAccept();
        });
}

} // namespace redis_engine::server
