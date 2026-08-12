// ============================================================================
// std::shared_mutex — reader-writer lock
// ----------------------------------------------------------------------------
// Many READERS can hold the SHARED lock at once (they don't block each other).
// A WRITER takes the EXCLUSIVE lock — alone, no readers allowed — so nobody
// ever reads the data while it's being modified (no read↔write race).
//
// Build & run:
//   g++ -std=c++23 -Wall -Wextra shared_mutex_demo.cpp -o shared_mutex_demo
//   .\shared_mutex_demo
// (add -pthread if your toolchain needs it)
// ============================================================================

#include <iostream>
#include <thread>
#include <shared_mutex>
#include <vector>
#include <chrono>
using namespace std;

shared_mutex rw;
int sharedData = 0;

// READER — takes a SHARED lock; many readers run concurrently.
void reader(int id) {
    for (int i = 0; i < 3; ++i) {
        {
            shared_lock<shared_mutex> lock(rw);   // shared (read) lock
            cout << "  [Reader " << id << "] read value = " << sharedData << "\n";
        }
        this_thread::sleep_for(chrono::milliseconds(50));
    }
}

// WRITER — takes an EXCLUSIVE lock; alone, blocks all readers while writing.
void writer(int id) {
    for (int i = 0; i < 3; ++i) {
        {
            unique_lock<shared_mutex> lock(rw);   // exclusive (write) lock
            ++sharedData;
            cout << "[Writer " << id << "] WROTE value = " << sharedData
                 << " (had exclusive access — no readers during this)\n";
        }
        this_thread::sleep_for(chrono::milliseconds(80));
    }
}

int main() {
    vector<thread> threads;

    // Many readers (read-heavy) + one writer.
    for (int i = 1; i <= 4; ++i) threads.emplace_back(reader, i);
    threads.emplace_back(writer, 1);

    for (auto& t : threads) t.join();

    cout << "\nFinal value = " << sharedData << "\n";
    cout << "Readers shared the lock freely; the writer had exclusive access,\n"
            "so no reader ever saw half-written data.\n";
}
