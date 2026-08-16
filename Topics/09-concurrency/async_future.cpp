// ============================================================================
// std::async / std::future / std::promise — getting a result back from a thread
// ----------------------------------------------------------------------------
// Part 1: std::async + future — run a task, collect its return value (1 line).
// Part 2: exceptions propagate through the future (.get() re-throws).
// Part 3: std::promise/future — the manual producer/consumer channel.
//
// Build & run:
//   g++ -std=c++23 -Wall -Wextra async_future.cpp -o async_future
//   .\async_future
// (add -pthread if your toolchain needs it)
// ============================================================================

#include <iostream>
#include <future>
#include <thread>
#include <chrono>
#include <stdexcept>
using namespace std;

// A slow task that returns a value.
int slowAdd(int a, int b) {
    this_thread::sleep_for(chrono::milliseconds(300));   // pretend it's expensive
    return a + b;
}

int mightThrow() {
    throw runtime_error("task failed!");
}

int main() {
    // ---- Part 1: async + future — run and collect a return value ----
    cout << "Part 1: std::async\n";
    future<int> fut = async(launch::async, slowAdd, 3, 4);   // starts on a new thread
    cout << "  (main is free to do other work while slowAdd runs...)\n";
    int result = fut.get();                                   // blocks until ready
    cout << "  slowAdd(3,4) = " << result << "\n\n";

    // ---- Part 2: exceptions propagate through the future ----
    cout << "Part 2: exception through a future\n";
    future<int> f2 = async(launch::async, mightThrow);
    try {
        f2.get();                     // the exception thrown in the task is re-thrown HERE
    } catch (const exception& e) {
        cout << "  caught from the task: " << e.what() << "\n\n";
    }

    // ---- Part 3: promise / future — manual result channel ----
    cout << "Part 3: std::promise / future\n";
    promise<int> p;
    future<int> f3 = p.get_future();          // reading end linked to the promise
    thread producer([&p] {
        this_thread::sleep_for(chrono::milliseconds(200));
        p.set_value(99);                      // writing end fulfills the promise
    });
    cout << "  waiting for the promise...\n";
    cout << "  got value = " << f3.get() << "\n";   // waits, then 99
    producer.join();
}
