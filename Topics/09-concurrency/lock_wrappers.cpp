// ============================================================================
// lock_guard vs unique_lock vs scoped_lock — each in action
// ----------------------------------------------------------------------------
// Build & run:
//   g++ -std=c++23 -Wall -Wextra lock_wrappers.cpp -o lock_wrappers
//   .\lock_wrappers
// (add -pthread if your toolchain needs it)
// ============================================================================

#include <iostream>
#include <thread>
#include <mutex>
using namespace std;

mutex m;
int counter = 0;

// ---------------------------------------------------------------------------
// 1) lock_guard — the default: lock a scope, auto-unlock. Simplest.
// ---------------------------------------------------------------------------
void useLockGuard() {
    lock_guard<mutex> g(m);      // lock
    counter++;                   // critical section
}                                // auto-unlock

// ---------------------------------------------------------------------------
// 2) unique_lock — flexible: unlock EARLY, do non-shared work, re-lock.
// ---------------------------------------------------------------------------
void useUniqueLock() {
    unique_lock<mutex> lk(m);    // lock
    counter++;                   // critical section
    lk.unlock();                 // release EARLY — other threads can proceed now

    // ... do work that does NOT touch shared data, WITHOUT holding the lock ...
    volatile long busy = 0;
    for (int i = 0; i < 1000; ++i) busy += i;

    lk.lock();                   // re-acquire when we need shared data again
    counter++;
}                                // auto-unlock

int main() {
    // ---- lock_guard + unique_lock across threads ----
    {
        thread t1(useLockGuard);
        thread t2(useUniqueLock);
        t1.join();
        t2.join();
        cout << "counter after lock_guard + unique_lock = " << counter << "\n";
    }

    // ---- scoped_lock: lock TWO mutexes at once, deadlock-free ----
    mutex a, b;
    int shared = 0;

    auto worker = [&] {
        for (int i = 0; i < 100000; ++i) {
            scoped_lock lock(a, b);   // locks BOTH atomically — no ordering deadlock
            shared++;                 // safe under both locks
        }
    };

    thread t3(worker);
    thread t4(worker);
    t3.join();
    t4.join();
    cout << "shared after scoped_lock (two mutexes) = " << shared
         << " (expected 200000)\n";
}
