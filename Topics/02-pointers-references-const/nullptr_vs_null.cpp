// ============================================================================
// nullptr vs NULL vs 0 — the overload-resolution difference
// ----------------------------------------------------------------------------
// NULL and 0 are really integers, so they pick the int overload by mistake.
// nullptr has pointer type, so it picks the pointer overload correctly.
//
// Build & run:
//   g++ -std=c++23 -Wall -Wextra nullptr_vs_null.cpp -o nullptr_vs_null
//   .\nullptr_vs_null
// ============================================================================

#include <iostream>

// Two overloads: one takes an int, one takes a pointer.
void f(int n)  { std::cout << "  f(int)  called with " << n << '\n'; }
void f(int* p) { std::cout << "  f(int*) called with "
                           << (p ? "a pointer" : "nullptr") << '\n'; }

int main() {
    std::cout << "f(0):       "; f(0);        // 0 is an int  -> f(int)
    std::cout << "f(NULL):    "; f(NULL);     // NULL is 0    -> f(int)  (surprise!)
    std::cout << "f(nullptr): "; f(nullptr);  // real pointer -> f(int*) (correct)

    // Type safety: nullptr will NOT convert to int.
    // int n = nullptr;   // ❌ compile error (uncomment to see)
    int n = NULL;         // compiles, because NULL is just 0
    std::cout << "\nint n = NULL; compiled fine -> n = " << n
              << "  (that's the trap: NULL is really an int)\n";

    std::cout << "\nTakeaway: NULL/0 pick f(int); nullptr picks f(int*). "
                 "Always use nullptr.\n";
}
