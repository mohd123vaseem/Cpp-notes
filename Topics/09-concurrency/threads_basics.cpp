// ============================================================================
// std::thread basics — launching, arguments, join vs detach
// ----------------------------------------------------------------------------
// Shows:
//   1. launching a thread with a function + with a lambda
//   2. passing arguments to a thread
//   3. join() — waiting for a thread to finish
//   4. the interleaving proof that threads run CONCURRENTLY
//   5. (commented) what happens if you forget join/detach → std::terminate
//
// Build & run  (MinGW may need -pthread; try without first):
//   g++ -std=c++23 -Wall -Wextra threads_basics.cpp -o threads_basics
//   .\threads_basics
// ============================================================================

#include <iostream>
#include <thread>
#include <chrono>
using namespace std;

// A plain function to run on a thread.
void countUp(const string& name, int n) {
    for (int i = 1; i <= n; ++i) {
        cout << name << ": " << i << "\n";
        this_thread::sleep_for(chrono::milliseconds(100));  // simulate work / let them interleave
    }
}

int main() {
    cout << "main: starting\n";

    // --- 1) launch a thread running a function, passing arguments ---------
    thread t1(countUp, "Thread-A", 3);   // countUp("Thread-A", 3) on a new thread

    // --- 2) launch another thread with a lambda --------------------------
    thread t2([] {
        for (int i = 1; i <= 3; ++i) {
            cout << "Thread-B (lambda): " << i << "\n";
            this_thread::sleep_for(chrono::milliseconds(100));
        }
    });

    // main, t1, and t2 are now ALL running at the same time.
    // Their output will INTERLEAVE (order varies run to run) — proof of concurrency.

    // --- 3) join: wait for both to finish before main continues ----------
    t1.join();   // block until Thread-A done
    t2.join();   // block until Thread-B done

    cout << "main: both threads finished\n";

    // --- 4) detach example (fire-and-forget) -----------------------------
    thread t3([] { cout << "Thread-C: detached, running on my own\n"; });
    t3.detach();                                   // let it run independently
    this_thread::sleep_for(chrono::milliseconds(50));  // give it a moment to print

    // --- 5) THE crash trap (leave commented) -----------------------------
    // thread t4(countUp, "Thread-D", 2);
    // // forgot t4.join() / t4.detach()
    // // → at the end of main, t4's destructor runs while still joinable
    // //   → std::terminate() → CRASH. Always join or detach!

    cout << "main: done\n";
}
