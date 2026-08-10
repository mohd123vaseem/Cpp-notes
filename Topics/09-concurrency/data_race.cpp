// ============================================================================
// Data race — see it fail with your own eyes
// ----------------------------------------------------------------------------
// Two threads each increment a SHARED counter 1,000,000 times.
// Expected total: 2,000,000. Actual (racy) total: usually LESS — lost updates.
//
// counter++ is really read-modify-write (3 steps); with no synchronization the
// two threads interleave and clobber each other → increments vanish.
//
// Build & run (run it a FEW times — the wrong number changes each run):
//   g++ -std=c++23 -Wall -Wextra data_race.cpp -o data_race
//   .\data_race
// (add -pthread if your toolchain needs it)
//
// NOTE: this is INTENTIONALLY buggy to demonstrate the race. The fix (mutex /
// atomic) comes in the next sub-topics.
// ============================================================================

#include <iostream>
#include <thread>
using namespace std;

int counter = 0;                       // SHARED, unprotected → the race

void addOneMillion() {
    for (int i = 0; i < 1'000'000; ++i) {
        counter++;                     // ⚠️ read-modify-write, no synchronization
    }
}

int main() {
    thread t1(addOneMillion);
    thread t2(addOneMillion);
    t1.join();
    t2.join();

    cout << "Expected: 2000000\n";
    cout << "Actual:   " << counter << "\n";

    if (counter != 2'000'000)
        cout << "The LOST " << (2'000'000 - counter)
             << " increments to the data race!\n";
    else
        cout << "→ Got lucky this run — run again, it'll usually be wrong.\n";
}
