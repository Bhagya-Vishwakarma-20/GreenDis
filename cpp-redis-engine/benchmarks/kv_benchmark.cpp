#include <benchmark/benchmark.h>
#include "../src/storage/MemoryStore.h"
#include "../src/services/KeyValueService.h"
#include <string>

using namespace redis_engine::storage;
using namespace redis_engine::services;

static void BM_MemoryStoreSet(benchmark::State& state) {
    auto store = std::make_shared<MemoryStore>();
    int i = 0;
    for (auto _ : state) {
        store->Set("key" + std::to_string(i), "value_payload_data_testing");
        i++;
    }
}
BENCHMARK(BM_MemoryStoreSet);

static void BM_MemoryStoreGet(benchmark::State& state) {
    auto store = std::make_shared<MemoryStore>();
    store->Set("key1", "value1");
    for (auto _ : state) {
        benchmark::DoNotOptimize(store->Get("key1"));
    }
}
BENCHMARK(BM_MemoryStoreGet);

static void BM_KeyValueServiceSetDetailed(benchmark::State& state) {
    auto store = std::make_shared<MemoryStore>();
    // No evictor overhead test
    KeyValueService service(store, nullptr, 100000000);
    int i = 0;
    for (auto _ : state) {
        service.Set("k" + std::to_string(i), "v");
        i++;
    }
}
BENCHMARK(BM_KeyValueServiceSetDetailed);

static void BM_ConcurrentOperations(benchmark::State& state) {
    auto store = std::make_shared<MemoryStore>();
    store->Set("shared_key", "base_val");

    for (auto _ : state) {
        // Just simulating the load. Real concurrent benchmarks 
        // would use state.range for threading.
        benchmark::DoNotOptimize(store->Get("shared_key"));
        store->Set("shared_key", "new_val");
    }
}
BENCHMARK(BM_ConcurrentOperations)->Threads(4);
BENCHMARK(BM_ConcurrentOperations)->Threads(8);

BENCHMARK_MAIN();
