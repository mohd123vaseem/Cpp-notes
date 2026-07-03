// ============================================================================
// const with pointers — the three combinations + the key insight
// ----------------------------------------------------------------------------
// The illegal lines are commented out (they'd be compile errors). Uncomment one
// at a time to see the compiler enforce const.
//
// Build & run:
//   g++ -std=c++23 -Wall -Wextra const_pointers.cpp -o const_pointers
//   .\const_pointers
// ============================================================================

#include <iostream>

int main() {
    int a = 10, b = 5;

    // ------------------------------------------------------------------
    // Case 1: const int* — pointer to const int (DATA locked, ptr moves)
    // ------------------------------------------------------------------
    const int* p1 = &a;
    // *p1 = 99;              // ❌ compile error — can't write through p1
    p1 = &b;                  // ✅ can repoint
    // *p1=120                // ❌ compile error — can't write through p1
    std::cout << "Case 1  const int*  : *p1 read-only, can repoint. *p1=" << *p1 << '\n';

    // KEY INSIGHT: const-ness is part of p1's TYPE, not tied to its target.
    // Even after repointing to b, we STILL can't write through p1:
    // *p1 = 42;              // ❌ still a compile error, even though p1 -> b now
    b = 42;                   // ✅ but b itself isn't const — change it directly
    std::cout << "        after repoint to b, *p1 still read-only; b changed directly to "
              << *p1 << '\n';

    // ------------------------------------------------------------------
    // Case 2: int* const — const pointer to int (ptr LOCKED, data moves)
    // ------------------------------------------------------------------
    int* const p2 = &a;       // must initialize now
    *p2 = 99;                 // ✅ can change the value (a becomes 99)
    // p2 = &b;               // ❌ compile error — pointer is locked
    std::cout << "\nCase 2  int* const  : *p2 editable, pointer locked. a=" << a << '\n';

    // ------------------------------------------------------------------
    // Case 3: const int* const — both locked
    // ------------------------------------------------------------------
    const int* const p3 = &a;
    // *p3 = 1;               // ❌ value is const
    // p3 = &b;               // ❌ pointer is const
    std::cout << "Case 3  const int* const : both locked. *p3=" << *p3 << '\n';

    std::cout << "\nRule: const LEFT of * -> data locked; const RIGHT of * -> pointer locked.\n";
}
