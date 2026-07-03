// ============================================================================
// References — alias behavior, the three rules, and pass-by-reference
// ----------------------------------------------------------------------------
// Build & run:
//   g++ -std=c++23 -Wall -Wextra references.cpp -o references
//   .\references
// ============================================================================

#include <iostream>

// Pass-by-reference: modifies the CALLER's variable (no copy, no pointer syntax).
void addOne(int& n) { n++; }

// Pass-by-pointer for comparison: caller must pass an address, we dereference.
void addOnePtr(int* n) { if (n) (*n)++; }

int main() {
    // ----- a reference is an ALIAS: same box, two names -----
    int x = 42;
    int& r = x;
    std::cout << "x=" << x << ", r=" << r << "  (r is an alias of x)\n";
    r = 99;                       // writing r writes x
    std::cout << "after r = 99 -> x=" << x << "  (same object)\n";
    std::cout << "&x=" << &x << ", &r=" << &r << "  (identical addresses)\n\n";

    // ----- RULE 2: a reference never re-seats -----
    int a = 1, b = 2;
    int& ra = a;
    ra = b;                       // assigns b's VALUE into a; ra still aliases a
    std::cout << "RULE 2: after ra = b -> a=" << a << " (a took b's value), "
              << "ra still aliases a\n\n";

    // ----- RULE 3: a reference can't be null; a pointer can -----
    int* p = nullptr;             // pointers CAN represent "nothing"
    std::cout << "RULE 3: pointer can be null (p == nullptr): "
              << std::boolalpha << (p == nullptr) << '\n';
    // int& bad = *p;             // ❌ would be UB — there is no null reference.
    //                                A reference MUST bind to a real object:
    int real = 7;
    int& good = real;             // ✅ bound to a real object
    std::cout << "        a reference must alias a real object: good=" << good
              << "\n\n";

    // ----- pass-by-reference vs pass-by-pointer -----
    int v = 10;
    addOne(v);                    // clean: no & at call site
    std::cout << "addOne(v)     -> v=" << v << "  (reference modified caller)\n";
    addOnePtr(&v);                // must take address explicitly
    std::cout << "addOnePtr(&v) -> v=" << v << "  (pointer modified caller)\n";
}
