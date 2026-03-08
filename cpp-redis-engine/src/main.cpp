#include "utils/Logger.h"
#include "config/ConfigLoader.h"
#include "storage/MemoryStore.h"
#include "eviction/LRUEviction.h"
#include "eviction/TTLManager.h"
#include "services/KeyValueService.h"

#include "persistence/AOFWriter.h"
#include "persistence/SnapshotManager.h"
#include "concurrency/ThreadPool.h"
#include "server/TcpServer.h"
#include "controllers/CommandDispatcher.h"
#include <iostream>
#include <csignal>
#include <atomic>
#include <thread>

using namespace redis_engine;

std::atomic<bool> g_Running{true};

void SignalHandler(int signum) {
    LOG_INFO("Received signal {}, shutting down...", signum);
    g_Running = false;
}

int main(int argc, char* argv[]) {
    utils::Logger::Init();
    LOG_INFO("Starting cpp-redis-engine...");

    std::signal(SIGINT, SignalHandler);
    std::signal(SIGTERM, SignalHandler);

    std::string config_path = "config/server_config.json";
    if (argc > 1) {
        config_path = argv[1];
    }

    auto config = config::ConfigLoader::Load(config_path);
    LOG_INFO("Server configured to run on port: {}", config.port);
    LOG_INFO("Max memory allowed: {} bytes", config.max_memory);

    // 1. Storage Layer
    auto store = std::make_shared<storage::MemoryStore>();
    
    // Recovery Phase (Persistence)
    persistence::RecoveryManager::Recover(store, config.rdb_file_path, config.aof_file_path);

    // 2. Eviction & TTL Layer
    auto lru = std::make_shared<eviction::LRUEviction>();
    auto ttl_manager = std::make_shared<eviction::TTLManager>(store);
    ttl_manager->Start();

    // 3. Service Layer (Approximating 100 byte overhead per key for max_keys limit)
    size_t max_keys = config.max_memory / 100; 
    auto kv_service = std::make_shared<services::KeyValueService>(store, lru, max_keys);
    auto exp_service = std::make_shared<services::ExpirationService>(store);

    // 4. Persistence Layer Writer
    auto aof_writer = std::make_shared<persistence::AOFWriter>(config.aof_file_path);
    auto snapshot_manager = std::make_shared<persistence::SnapshotManager>(
        store, config.rdb_file_path, std::chrono::seconds(config.snapshot_interval));
    snapshot_manager->Start();

    // 5. Concurrency ThreadPool
    auto thread_pool = std::make_shared<concurrency::ThreadPool>(config.thread_pool_size);

    // 6. Controller / Dispatcher
    auto dispatcher = std::make_shared<controllers::CommandDispatcher>(kv_service, exp_service, aof_writer);

    // Connection Handler callback
    server::Connection::MessageHandler handler = [thread_pool, dispatcher](std::shared_ptr<server::Connection> conn, const std::string& input) {
        // Parse on network thread or worker thread? 
        // We push the processing to the worker thread to free up IO threads
        auto req = protocol::CommandParser::Parse(input);
        thread_pool->Enqueue([conn, dispatcher, req]() {
            dispatcher->Dispatch(conn, req);
        });
    };

    // 7. Network Server
    server::TcpServer server(config.port, handler, 2); // 2 IO threads
    server.Start();

    LOG_INFO("Engine setup complete. Ready to accept connections.");
    
    // Main thread waits until asked to stop
    while (g_Running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    LOG_INFO("Shutting down engine components...");
    server.Stop();
    ttl_manager->Stop();
    snapshot_manager->Stop();
    
    // Force final snapshot on graceful exit
    snapshot_manager->TakeSnapshot();
    aof_writer->Sync();

    LOG_INFO("Shutdown complete.");
    return 0;
}
