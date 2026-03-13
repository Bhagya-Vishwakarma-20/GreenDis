<div align="center">
  <h1>GreenDis</h1>
  <p><b>A Lightweight, Embeddable Redis-Like Engine in Modern C++</b></p>
</div>

Welcome to **GreenDis**, an educational and highly performant in-memory key-value store. This project aims to replicate the core functionalities of Redis natively in C++, allowing developers to embed it directly into their applications without needing external server setups.

---

## Engine Features & Architecture

At the heart of the project sits the **`cpp-redis-engine`**. Let's break down its core capabilities:

- **Memory-Mapped Storage**: Instead of creating a giant, bloated database, `cpp-redis-engine` stores your keys and values under the hood in a lightning-fast hash map (`std::unordered_map`). 
- **Metadata Management**: Each value isn't just a raw string. It's wrapped in an `Entry` object that keeps track of metadata (like *Time-To-Live / TTL* and *Last Accessed Time*).
- **Concurrency & Thread Safety**: Multiple threads can `Get` (read) values at the same time using a shared lock (`std::shared_mutex`), but only one thread can `Set` (write) or `Delete` using an exclusive lock. This guarantees no data races while maintaining blazing-fast read operations.
- **TCP Networking**: Built on top of standalone **ASIO**, GreenDis acts as an asynchronous TCP server. It listens on port **6379** by default (the exact same port as Redis). It efficiently handles client connections utilizing dedicated, non-blocking I/O threads and dispatches command execution to a separate worker thread pool.

It behaves exactly like Redis, but natively in your C++ memory block, and seamlessly serves over the network!

---

##  How We Test It (`main.cpp`)

To get started quickly, we use `src/main.cpp` as our primary testing ground. Think of it as a lightweight client that instantiates the engine natively, bypassing any network or socket overhead. 

Running the engine locally in `main.cpp` allows us to verify data structures and memory manipulation quickly.

```cpp
#include "storage/MemoryStore.h"
#include <iostream>

using namespace std;

int main() {
    redis_engine::storage::MemoryStore store;

    store.Set("name", "bhagya");

    auto value = store.Get("name");
    if (value.has_value()) {
        cout << "Value retrieved: " << value.value() << endl;
    }

    return 0;
}
```

---



## Building and Running

You can easily compile and test this engine yourself using CMake! 
*(Note: If you are on Windows, we recommend doing this inside **WSL (Windows Subsystem for Linux)** for the best experience).*

```bash
mkdir build && cd build
cmake ..
cmake --build .
./MyCacheApp
```

> **Note:** Running testing via `main.cpp` like this is purely educational to verify memory behaviors. **GreenDis is fundamentally designed to run as a standalone TCP Server**, listening for concurrent socket connections.

### Expected Output

*(You can place your screenshot of the console output here)*  
![Execution Output](./output.png)
