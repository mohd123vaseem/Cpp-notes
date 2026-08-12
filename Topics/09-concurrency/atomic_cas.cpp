// ============================================================================
// std::atomic + CAS — the lock-free fix for a single-variable race
// ----------------------------------------------------------------------------
// Part 1: the sub-topic-3 counter race, fixed with std::atomic (NO mutex).
// Part 2: a CAS (compare-and-swap) retry loop — atomically double a value.
//
// Build & run:
//   g++ -std=c++23 -Wall -Wextra atomic_cas.cpp -o atomic_cas
//   .\atomic_cas
// (add -pthread if your toolchain needs it)
// ============================================================================

#include <iostream>
#include <thread>
#include <atomic>
#include <vector>
using namespace std;

// ---------------------------------------------------------------------------
// PART 1 — atomic counter: no data race, no mutex, exactly 2,000,000 every run
// ---------------------------------------------------------------------------
atomic<int> counter = 0;                 // atomic → counter++ is indivisible

void addOneMillion() {
    for (int i = 0; i < 1'000'000; ++i)
        counter++;                       // one atomic hardware op — no lock needed
}

// ---------------------------------------------------------------------------
// PART 2 — CAS retry loop: atomically multiply `value` by 2, safely, even when
// many threads do it at once. (x *= 2 is NOT a single atomic op by itself.)
// ---------------------------------------------------------------------------
atomic<int> value = 1;

void doubleItOnce() {
    int old = value.load();              // read current value
    // "if value is still `old`, set it to old*2; else reload `old` and retry"
    while (!value.compare_exchange_strong(old, old * 2)) {
        // CAS failed → another thread changed `value` → `old` now holds the new
        // current value → loop retries with the fresh value until it sticks.
    }
}

int main() {
    // ---- Part 1 ----
    thread t1(addOneMillion), t2(addOneMillion);
    t1.join(); t2.join();
    cout << "PART 1 (atomic counter): " << counter
         << "  (expected 2000000, correct every run, no mutex)\n";

    // ---- Part 2 ----
    // 10 threads each double `value` once → 1 * 2^10 = 1024, with NO lost updates.
    vector<thread> pool;
    for (int i = 0; i < 10; ++i) pool.emplace_back(doubleItOnce);
    for (auto& t : pool) t.join();
    cout << "PART 2 (CAS retry loop): " << value
         << "  (expected 1024 = 1 * 2^10 — every doubling applied, none lost)\n";
}
