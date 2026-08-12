// ============================================================================
// std::call_once / once_flag — thread-safe lazy initialization
// ----------------------------------------------------------------------------
// 8 threads all call getResource(). The initializer runs EXACTLY ONCE:
// the first thread creates the resource, the rest wait then reuse it.
// Watch: "Initializing resource..." prints only ONE time.
//
// Build & run:
//   g++ -std=c++23 -Wall -Wextra call_once_demo.cpp -o call_once_demo
//   .\call_once_demo
// (add -pthread if your toolchain needs it)
// ============================================================================

#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
using namespace std;

once_flag flag;
int* resource = nullptr;
atomic<int> initCount = 0;      // counts how many times the initializer actually ran

void initResource() {
    initCount++;
    cout << "Initializing resource... (this should print ONCE)\n";
    resource = new int(42);
}

int getResource(int threadId) {
    call_once(flag, initResource);           // runs initResource EXACTLY once, across all threads
    cout << "  [Thread " << threadId << "] using resource = " << *resource << "\n";
    return *resource;
}

int main() {
    vector<thread> threads;
    for (int i = 1; i <= 8; ++i)
        threads.emplace_back(getResource, i);   // 8 threads race to be "first"

    for (auto& t : threads) t.join();

    cout << "\nInitializer ran " << initCount << " time(s) (expected: 1)\n";
    cout << "The first thread created the resource; the other 7 reused it.\n";

    delete resource;
}
