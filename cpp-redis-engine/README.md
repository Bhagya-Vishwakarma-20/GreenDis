# GreenDis (C++ Redis-Like Key-Value Store)

A production-grade, distributed-ready, in-memory key-value database written in Modern C++ (C++17). 

This project aims to replicate the core mechanics of Redis with a robust, layered architecture, thread-safe memory handling, multi-threading, and persistent storage mechanisms.

## 🌊 Architecture & Flow

The inner workings of GreenDis follow a structured, multi-layered MVC (Model-View-Controller) pattern, separating networking, protocol parsing, and storage.

```mermaid
graph TD
    Client[Client Connection] --> Server[TCP Server (Boost.Asio)]
    Server --> Parser[Command Parser]
    Parser --> Dispatcher[Command Dispatcher]
    
    Dispatcher --> Service[Key-Value & Expiration Services]
    
    Service --> Storage[Memory Store (Hash Table)]
    Service -.-> Eviction[Eviction Policy (LRU)]
    
    Storage --> Snapshot[RDB Snapshot Manager]
    Storage --> AOF[AOF Writer]
```

### Flow Breakdown
1. **Network Layer:** Accepts TCP client connections asynchronously using Asio.
2. **Protocol & Parsers:** Translates incoming string streams (e.g., standard TCP messages, RESP format) into structured `Request` objects.
3. **Dispatcher & Services:** Routes verified commands like `SET` and `GET` into the core services, managing validations and active background TTL (Time-To-Live) sweeping.
4. **Storage Engine:** Values are stored in a highly concurrent `std::unordered_map`. Read and write locks (`std::shared_mutex`) ensure safe multi-threaded access.
5. **Persistence Engine:** Maintains durability across restarts by writing periodic background binary dumps (RDB) and maintaining an Append-Only File (AOF) for crash recovery.

---

## 🚀 Setup & Build Instructions

GreenDis uses CMake and `FetchContent` to intelligently pull all dependencies (spdlog, nlohmann/json, Asio, GoogleTest, Google Benchmark) during configuration.

### Prerequisites
- A C++17 compliant compiler (GCC/Clang/MSVC)
- CMake 3.16+
- Threading library support

### Steps to Build

```bash
# 1. Clone the repository and navigate to the project directory
git clone https://github.com/yourusername/cpp-redis-engine.git
cd cpp-redis-engine

# 2. Create a build directory
mkdir build && cd build

# 3. Configure and build the project
cmake ..
cmake --build . -j 4
```

### Running the Server

Start the compiled engine by providing the server configuration file:

```bash
./cpp-redis-engine ../config/server_config.json
```

---

## ⚡ How to Use It as Redis

Once the GreenDis server is active, it runs quietly as a background daemon or terminal process on the configured TCP port (default is usually `6379`).

You can connect and interact with it exactly like a typical key-value store using any raw TCP terminal (like `netcat`, `nc` or `telnet`):

```bash
# Connect to the local server
nc 127.0.0.1 6379
```

### Basic Commands
- **`SET <key> <value>`** - Stores the value associated with the key.
- **`GET <key>`** - Retrieves the stored value.
- **`EXPIRE <key> <seconds>`** - Sets an expiration TTL.
- **`DEL <key>`** - Deletes the key from memory.

### Example Session
```text
> SET user:1 Bhagya
OK
> GET user:1
Bhagya
> EXPIRE user:1 60
1
> DEL user:1
1
```

---

## 🧩 Integration in Other Projects

You can use the high-performance memory components of GreenDis directly inside other C++ codebases without running the standalone TCP server.

### As a Library

1. **Include in CMake:** Treat `cpp-redis-engine` as a subdirectory in your own `CMakeLists.txt` file and link against its core target:
  ```cmake
  add_subdirectory(path/to/cpp-redis-engine)
  
  # Link your executable with the core library parts
  target_link_libraries(YourApplication PRIVATE redis_engine_lib)
  ```

2. **Programmatic Usage:** Include the necessary headers (e.g., `MemoryStore.h`) and interact directly with the storage engine programmatically. You can instantiate `MemoryStore` locally directly within your logic and skip the Network and Dispatcher layers entirely.

```cpp
#include "storage/MemoryStore.h"
#include <iostream>

int main() {
    MemoryStore store; // Thread-safe memory container
    
    // Usage as a library
    store.set("app_state", "running");
    
    auto state = store.get("app_state");
    if (state.has_value()) {
        std::cout << "State is: " << state.value() << "\n";
    }
    
    return 0;
}
```

## Testing & Benchmarks

The engine comes with strong unit test coverage and Google Benchmarks for core functionalities.
```bash
./unit_tests 
./kv_benchmark
```
