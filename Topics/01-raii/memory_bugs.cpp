// ============================================================================
// The Four Classic Memory Bugs — and their fixes
// ----------------------------------------------------------------------------
// The BAD patterns are shown but commented out / guarded, because running them
// is undefined behavior (crashes, corruption). Each one is paired with the
// correct version that actually runs.
//
// Build & run (normal):
//   g++ -std=c++23 -Wall -Wextra memory_bugs.cpp -o memory_bugs
//   .\memory_bugs
//
// Want to SEE the bugs caught? Build with AddressSanitizer and uncomment a
// bad block (one at a time):
//   g++ -std=c++23 -g -fsanitize=address memory_bugs.cpp -o memory_bugs
// ============================================================================

#include <iostream>
#include <memory>

int main() {
    // ========================================================================
    // BUG 1 — MEMORY LEAK: new without delete
    // ========================================================================
    std::cout << "1) Memory leak\n";
    // BAD:
    //   int* p = new int(42);
    //   ...forgot delete p;  -> leaked, nobody can free it now
    // FIX A: pair every new with a delete
    {
        int* p = new int(42);
        std::cout << "  used " << *p << ", now freeing\n";
        delete p;                          // matched -> no leak
    }
    // FIX B (preferred): RAII, no manual delete at all
    {
        auto p = std::make_unique<int>(42);
        std::cout << "  unique_ptr holds " << *p << " (auto-freed)\n";
    }

    // ========================================================================
    // BUG 2 — DANGLING POINTER: pointer outlives its target
    // ========================================================================
    std::cout << "\n2) Dangling pointer\n";
    int* p = new int(7);
    delete p;                              // target freed; p now dangles
    // BAD: using p here would be use-after-free
    p = nullptr;                           // FIX: null it so it points at nothing
    std::cout << "  after delete, p set to nullptr (safe)\n";

    // ========================================================================
    // BUG 3 — USE-AFTER-FREE: reading/writing freed memory
    // ========================================================================
    std::cout << "\n3) Use-after-free\n";
    // BAD:
    //   int* q = new int(5);
    //   delete q;
    //   std::cout << *q;   // ❌ reads freed memory (UB)
    //   *q = 99;           // ❌ writes freed memory (UB)
    // FIX: don't touch memory after freeing; let RAII own the lifetime
    {
        auto q = std::make_unique<int>(5);
        std::cout << "  *q = " << *q << " (valid; freed only at scope end)\n";
    } // q's int freed here, and q is gone too — impossible to use after free

    // ========================================================================
    // BUG 4 — DOUBLE-FREE: delete the same pointer twice
    // ========================================================================
    std::cout << "\n4) Double-free\n";
    // BAD:
    //   int* r = new int(1);
    //   delete r;
    //   delete r;          // ❌ frees already-freed memory (UB, corrupts heap)
    // FIX: clear ownership — exactly one owner frees exactly once
    {
        int* r = new int(1);
        delete r;
        r = nullptr;                       // delete nullptr is a safe no-op...
        delete r;                          // ...so even this second delete is OK
        std::cout << "  freed once; nulled; second delete(nullptr) is harmless\n";
    }
    // FIX (preferred): unique_ptr — single owner, frees once, can't double-free
    {
        auto r = std::make_unique<int>(1);
        std::cout << "  unique_ptr: one owner, freed exactly once\n";
    }

    std::cout << "\nAll four bugs come from manual memory management.\n"
                 "RAII / smart pointers make them structurally impossible.\n";
}
