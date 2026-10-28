// ============================================================================
// auto vs decltype — auto STRIPS const/ref (value copy); decltype KEEPS them
// ----------------------------------------------------------------------------
// We use typeid(...).name() just to print rough type info. Names are mangled
// & compiler-specific (e.g. 'i' = int), but the BEHAVIOR is the point:
//   - modifying an `auto` copy does NOT change the original (it stripped &)
//   - decltype preserves const/& exactly
//
// Build & run:
//   g++ -std=c++23 -Wall -Wextra auto_decltype.cpp -o auto_decltype
//   .\auto_decltype
// ============================================================================

#include <iostream>
#include <vector>
using namespace std;

int main() {
    // ---- auto strips const ----
    const int ci = 5;
    auto a = ci;          // a is int (const stripped) — a is a modifiable copy
    a = 99;               // ✅ allowed — a is NOT const
    cout << "auto from const int: a modified to " << a
         << " (ci still " << ci << ") — const was stripped\n";

    // ---- auto strips reference (gives a COPY) ----
    int x = 10;
    int& ref = x;
    auto b = ref;         // b is int (NOT int&) — a COPY of x
    b = 777;              // changes b only
    cout << "auto from int&: b = " << b << ", x = " << x
         << " (x unchanged) — reference was stripped → b is a copy\n";

    // ---- keep the reference explicitly with auto& ----
    auto& rb = x;         // rb is int& — actually aliases x
    rb = 555;             // changes x through rb
    cout << "auto& : rb = " << rb << ", x = " << x
         << " (x changed) — reference kept\n";

    // ---- const auto& in a range-for: no copy, read-only ----
    vector<string> words = {"alpha", "beta", "gamma"};
    cout << "range-for with const auto& (no copies): ";
    for (const auto& w : words) cout << w << " ";
    cout << "\n";

    // ---- decltype keeps const/& exactly ----
    const int cj = 42;
    decltype(cj) d = 100;     // d is const int — KEEPS const (auto would have dropped it)
    // d = 1;                 // ❌ would NOT compile — d is const
    cout << "decltype(const int): d = " << d << " (const preserved — can't reassign)\n";

    // ---- decltype for a computed return type ----
    auto addMixed = [](auto p, auto q) -> decltype(p + q) { return p + q; };
    cout << "decltype return: 3 + 2.5 = " << addMixed(3, 2.5)
         << " (type is double — decltype(int + double))\n";
}
