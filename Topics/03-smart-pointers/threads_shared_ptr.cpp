// ============================================================================
// shared_ptr across threads — the SAFE pattern vs the DATA RACE
// ----------------------------------------------------------------------------
// SAFE  : each thread gets its OWN copy of the shared_ptr. The atomic refcount
//         handles concurrent copy/destroy correctly.
// RACE  : multiple threads read/WRITE the SAME shared_ptr instance. The atomic
//         count does NOT protect the shared_ptr variable itself → undefined
//         behavior. It is guarded behind a flag because it's genuinely UB
//         (may crash, corrupt, or deceptively "look fine").
//
// Build & run:
//   g++ -std=c++23 -Wall -Wextra threads_shared_ptr.cpp -o threads_shared_ptr
//   .\threads_shared_ptr
// (If your toolchain needs it, add -pthread.)
// ============================================================================

#include <iostream>
#include <memory>
#include <thread>
#include <vector>

// Flip to true to run the racy version (UNDEFINED BEHAVIOR — for demonstration).
constexpr bool RUN_THE_RACE = false;

// ----------------------------------------------------------------------------
// SAFE: the parameter is taken BY VALUE, so each thread receives its own copy.
// Copying/destroying that copy touches the atomic refcount — thread-safe.
// ----------------------------------------------------------------------------
void worker_safe(std::shared_ptr<int> myCopy, int id) {   // <-- own copy per thread
    // Reading the pointed-to int (42) is fine: we only READ, nobody writes it.
    std::cout << "  [safe] thread " << id << " sees value " << *myCopy
              << ", use_count ~= " << myCopy.use_count() << '\n';
    // myCopy is destroyed when this function returns → atomic decrement.
}

// ----------------------------------------------------------------------------
// RACE: both threads capture the SAME shared_ptr by REFERENCE and mutate it.
// Two threads writing/reading the same shared_ptr instance = data race = UB.
// (The atomic refcount protects the COUNT, not the shared_ptr variable.)
// ----------------------------------------------------------------------------
void worker_race(std::shared_ptr<int>& shared, int id) {   // <-- SAME instance (ref)
    for (int i = 0; i < 100000; ++i) {
        shared = std::make_shared<int>(id);  // ❌ concurrent WRITE to `shared`
        volatile int v = *shared;            // ❌ concurrent READ of `shared`
        (void)v;
    }
}

int main() {
    // ===== SAFE demo =====
    std::cout << "SAFE: each thread gets its own copy\n";
    auto p = std::make_shared<int>(42);        // count = 1
    std::cout << "  before threads, use_count = " << p.use_count() << '\n';

    std::vector<std::thread> pool;
    for (int i = 0; i < 4; ++i)
        pool.emplace_back(worker_safe, p, i);  // pass p BY VALUE → each gets a copy

    for (auto& t : pool) t.join();
    std::cout << "  after join, use_count = " << p.use_count()
              << " (back to 1 — all copies destroyed cleanly)\n";

    // ===== RACE demo (guarded) =====
    std::cout << "\nRACE: threads share ONE shared_ptr instance\n";
    if (RUN_THE_RACE) {
        std::shared_ptr<int> shared = std::make_shared<int>(0);
        std::thread t1(worker_race, std::ref(shared), 1);  // both take the SAME
        std::thread t2(worker_race, std::ref(shared), 2);  // instance by reference
        t1.join();
        t2.join();
        std::cout << "  (if it didn't crash, that's luck — it's still UB)\n";
    } else {
        std::cout << "  (disabled — set RUN_THE_RACE = true to see the UB;\n"
                     "   it may crash, corrupt memory, or misleadingly 'work')\n";
    }
}
