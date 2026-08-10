// ============================================================================
// Multi-mutex DEADLOCK, and the scoped_lock fix
// ----------------------------------------------------------------------------
// Two threads lock two mutexes in OPPOSITE orders → circular wait → deadlock.
// scoped_lock locks both atomically with a deadlock-avoidance algorithm.
//
// Build & run:
//   g++ -std=c++23 -Wall -Wextra deadlock_scoped_lock.cpp -o deadlock_scoped_lock
//   .\deadlock_scoped_lock
// (add -pthread if your toolchain needs it)
// ============================================================================

#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
using namespace std;

mutex m1, m2;

// ---------------------------------------------------------------------------
// THE BUG (guarded — it HANGS forever). Two threads, opposite lock order.
// ---------------------------------------------------------------------------
void threadA_bad() {
    lock_guard<mutex> g1(m1);                         // A locks m1
    this_thread::sleep_for(chrono::milliseconds(10)); // (let B grab m2 first)
    lock_guard<mutex> g2(m2);                         // A waits for m2 (B has it)
    cout << "A got both (never happens)\n";
}
void threadB_bad() {
    lock_guard<mutex> g1(m2);                         // B locks m2
    this_thread::sleep_for(chrono::milliseconds(10)); // (let A grab m1 first)
    lock_guard<mutex> g2(m1);                         // B waits for m1 (A has it)
    cout << "B got both (never happens)\n";
}
// A holds m1 waiting m2, B holds m2 waiting m1 → both stuck forever = DEADLOCK.

// ---------------------------------------------------------------------------
// THE FIX — scoped_lock locks BOTH at once, no ordering deadlock.
// ---------------------------------------------------------------------------
void threadA_good() {
    scoped_lock lock(m1, m2);   // both together, deadlock-free
    cout << "A got both safely\n";
}
void threadB_good() {
    scoped_lock lock(m2, m1);   // even in a different order — scoped_lock handles it
    cout << "B got both safely\n";
}

int main() {
    constexpr bool RUN_THE_DEADLOCK = false;   // ⚠️ set true to watch it HANG forever

    if (RUN_THE_DEADLOCK) {
        cout << "Running the buggy version (will hang)...\n";
        thread t1(threadA_bad);
        thread t2(threadB_bad);
        t1.join();   // never returns — deadlocked
        t2.join();
    } else {
        cout << "Deadlock version disabled. Running the scoped_lock fix:\n";
        thread t1(threadA_good);
        thread t2(threadB_good);
        t1.join();
        t2.join();
        cout << "Both finished — no deadlock. scoped_lock locked both safely.\n";
    }
}
