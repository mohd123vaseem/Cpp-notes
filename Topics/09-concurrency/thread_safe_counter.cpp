// ============================================================================
// Thread-safe design — a self-protecting Counter class
// ----------------------------------------------------------------------------
// Principles shown:
//   - private mutex encapsulated inside the class (callers don't manage it)
//   - lock in EVERY method that touches shared state — including const reads
//   - `mutable std::mutex` so const methods can lock
//   - whole operations protected as one critical section
//
// 10 threads each call increment() 100,000 times → final value exactly
// 1,000,000, because the class protects itself.
//
// Build & run:
//   g++ -std=c++23 -Wall -Wextra thread_safe_counter.cpp -o thread_safe_counter
//   .\thread_safe_counter
// (add -pthread if your toolchain needs it)
// ============================================================================

#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
using namespace std;

class Counter {
    long value = 0;
    mutable mutex m;                    // private + mutable → lockable inside const methods

public:
    // whole operation under one lock
    void increment() {
        lock_guard<mutex> lock(m);
        value++;
    }

    // a combined atomic operation (check + act as ONE critical section) —
    // avoids the interface-level check-then-act race (Principle 6).
    bool decrementIfPositive() {
        lock_guard<mutex> lock(m);
        if (value > 0) { value--; return true; }
        return false;
    }

    // reads need the lock too — const method, so the mutex must be `mutable`
    long get() const {
        lock_guard<mutex> lock(m);
        return value;
    }
};

int main() {
    Counter counter;

    // 10 threads each increment 100,000 times
    vector<thread> threads;
    for (int i = 0; i < 10; ++i)
        threads.emplace_back([&counter] {
            for (int j = 0; j < 100'000; ++j) counter.increment();
        });
    for (auto& t : threads) t.join();

    cout << "Final value = " << counter.get()
         << "  (expected 1000000 — the class protected itself)\n";
}
