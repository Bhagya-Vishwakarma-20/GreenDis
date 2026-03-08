#include "storage/MemoryStore.h"
#include <iostream>
using namespace std;

int main() {
    redis_engine::storage::MemoryStore store;

    cout << "--- Testing GreenDis (cpp-redis-engine) ---" << endl;

    // 1. Testing SET
    store.Set("name", "bhagya");
    cout << "Set key 'name' to 'bhagya'" << endl;

    // 2. Testing GET
    auto value = store.Get("name");
    if(value.has_value()) {
        cout << "Get key 'name': " << value.value() << endl;
    }

    // 3. Testing newly added EXISTS method
    bool exists = store.Exists("name");
    cout << "Does key 'name' exist? " << (exists ? "Yes" : "No") << endl;

    // 4. Testing DELETE
    store.Delete("name");
    cout << "Deleted key 'name'" << endl;

    // 5. Verifying Deletion with EXISTS
    exists = store.Exists("name");
    cout << "Does key 'name' exist after deletion? " << (exists ? "Yes" : "No") << endl;

    return 0;
}