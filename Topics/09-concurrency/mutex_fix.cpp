// ============================================================================
// mutex + lock_guard — FIXING the data race from data_race.cpp
// ----------------------------------------------------------------------------
// Same two-threads-increment-a-million-times program, but now the shared
// counter is protected by a mutex via lock_guard. Result: EXACTLY 2,000,000,
// every single run (no lost updates).
//
// Build & run:
//   g++ -std=c++23 -Wall -Wextra mutex_fix.cpp -o mutex_fix
//   .\mutex_fix
// (add -pthread if your toolchain needs it)
// ============================================================================

#include <iostream>
#include <thread>
#include <mutex>
using namespace std;

int counter = 0;
mutex m;                                   // protects `counter`

void addOneMillion() {
    for (int i = 0; i < 1'000'000; ++i) {
        lock_guard<mutex> guard(m);        // lock (ctor) — only one thread inside at a time
        counter++;                         // critical section — now safe
    }                                      // unlock (dtor) at end of each iteration
}

int main() {
    thread t1(addOneMillion);
    thread t2(addOneMillion);
    t1.join();
    t2.join();

    cout << "Expected: 2000000\n";
    cout << "Actual:   " << counter << "\n";
    cout << (counter == 2'000'000
             ? "→ CORRECT every run — the mutex serialized access, race gone.\n"
             : "→ (unexpected — should be exactly 2000000)\n");
}
