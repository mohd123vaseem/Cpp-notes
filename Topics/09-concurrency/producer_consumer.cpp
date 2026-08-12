// ============================================================================
// Producer–Consumer with condition_variable  (the classic pattern)
// ----------------------------------------------------------------------------
// Restaurant analogy:
//   CHEF  (producer) cooks dishes and puts them on a shared counter (queue).
//   WAITER(consumer) takes dishes off the counter and serves them.
//   When the counter is empty, the waiter SLEEPS (0% CPU) until the chef
//   rings the bell (condition_variable notify). No busy-waiting.
//
// This version uses ONE producer + ONE consumer, plus a "done" flag so the
// consumer knows to stop once the chef has finished cooking everything.
//
// Build & run:
//   g++ -std=c++23 -Wall -Wextra producer_consumer.cpp -o producer_consumer
//   .\producer_consumer
// (add -pthread if your toolchain needs it)
// ============================================================================

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>
using namespace std;

queue<int> counter;            // the shared counter (dishes waiting to be served)
mutex m;                       // protects `counter` and `done`
condition_variable cv;         // the "bell"
bool done = false;             // set true when the chef has cooked everything

const int TOTAL_DISHES = 5;

// ---------------------------------------------------------------------------
// PRODUCER — cooks TOTAL_DISHES dishes, one every 400ms, notifying each time.
// ---------------------------------------------------------------------------
void chef() {
    for (int dish = 1; dish <= TOTAL_DISHES; ++dish) {
        this_thread::sleep_for(chrono::milliseconds(400));   // cooking takes time

        {
            lock_guard<mutex> lock(m);          // lock the counter
            counter.push(dish);                 // put the dish on the counter
            cout << "[Chef]   cooked & placed dish " << dish << "\n";
        }                                       // unlock
        cv.notify_one();                        // RING THE BELL: "a dish is ready!"
    }

    // Tell the waiter no more dishes are coming.
    {
        lock_guard<mutex> lock(m);
        done = true;
    }
    cv.notify_one();                            // wake the waiter so it can see `done`
    cout << "[Chef]   finished cooking everything.\n";
}

// ---------------------------------------------------------------------------
// CONSUMER — serves dishes as they appear; sleeps while the counter is empty;
// stops once `done` is true AND the counter is drained.
// ---------------------------------------------------------------------------
void waiter() {
    while (true) {
        unique_lock<mutex> lock(m);             // unique_lock — required by cv.wait

        // Sleep until: there's a dish to serve, OR the chef is done.
        cv.wait(lock, [] { return !counter.empty() || done; });

        // Drain every dish currently on the counter.
        while (!counter.empty()) {
            int dish = counter.front();
            counter.pop();
            lock.unlock();                      // release while "serving" (no shared data touched)
            cout << "   [Waiter] served dish " << dish << "\n";
            this_thread::sleep_for(chrono::milliseconds(150));  // serving takes time
            lock.lock();                        // re-lock before checking the queue again
        }

        // If the chef is done and the counter is empty, we're finished.
        if (done && counter.empty()) {
            cout << "   [Waiter] no more dishes — going home.\n";
            break;
        }
    }
}

int main() {
    cout << "Restaurant opens.\n";

    thread t1(chef);       // producer thread
    thread t2(waiter);     // consumer thread

    t1.join();
    t2.join();

    cout << "Restaurant closed. All " << TOTAL_DISHES << " dishes served.\n";
}
