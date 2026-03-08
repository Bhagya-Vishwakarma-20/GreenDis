# System Architecture

The `cpp-redis-engine` is designed following a clean MVC-style architecture. This ensures high modularity, easy testing, and clear separation of concerns. The codebase is divided into several layers.

## High-Level Flow Diagram

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

## Layer Breakdown

### Network Layer (`src/server`)
Responsible for managing TCP client connections efficiently using `Boost.Asio` (or standalone ASIO). It abstracts the complexities of asynchronous socket programming.
- `TcpServer`: Manages the acceptor and initial connection setup.
- `Connection`: Represents a single client session, handling async reads and writes.

### Protocol Layer (`src/protocol`)
- `CommandParser`: Translates raw string incoming messages into structured `Request` objects, normalizing commands for downstream processing.

### Controller Layer (`src/controllers`)
- `CommandDispatcher`: Acts as the glue between the raw parsed request and the business logic. It validates argument counts and formats responses.

### Service Layer (`src/services`)
Contains the core business logic.
- `KeyValueService`: Coordinates between storage and eviction policies (e.g., evicting if max capacity is reached upon a `SET`).
- `ExpirationService`: Translates TTL commands into time points.

### Storage Layer (`src/storage`)
The core in-memory engine.
- `MemoryStore`: A thread-safe wrapper around `std::unordered_map`. Read/write locks (`std::shared_mutex`) ensure safe concurrent access.

### Persistence Layer (`src/persistence`)
Durability mechanisms to survive crashes.
- `AOFWriter`: Appends every mutating command to disk.
- `SnapshotManager`: Periodically dumps the entire memory state to disk in a compact binary format.
- `RecoveryManager`: Reconstructs state on startup from RDB and subsequently replays AOF logs.

### Eviction and TTL Layer (`src/eviction`)
Memory Management.
- `LRUEviction`: Evicts the oldest accessed keys when capacity hits a threshold.
- `TTLManager`: A background thread that routinely scans for and purges expired keys.

### Concurrency Layer (`src/concurrency`)
- `ThreadPool`: Allows decoupling of network IO from command execution, ensuring slow commands do not block the acceptance of new connections.
