// ============================================================================
// Iterator invalidation — the #1 STL gotcha
// ----------------------------------------------------------------------------
// Part 1: vector reallocation invalidates old pointers/iterators.
// Part 2: the erase-in-loop bug (commented — it's UB) and the correct fix.
//
// Build & run:
//   g++ -std=c++23 -Wall -Wextra iterator_invalidation.cpp -o iterator_invalidation
//   .\iterator_invalidation
// ============================================================================

#include <iostream>
#include <vector>
using namespace std;

int main() {
    // ---- Part 1: reallocation invalidates old pointers ----
    cout << "Part 1: vector reallocation\n";
    vector<int> v = {1, 2, 3};
    v.reserve(3);                       // capacity is exactly 3 → next push_back reallocs
    int* p = &v[0];
    cout << "  before push_back: *p = " << *p
         << ", capacity = " << v.capacity() << "\n";

    v.push_back(4);                     // grows → reallocates → old buffer freed
    cout << "  after push_back:  capacity = " << v.capacity()
         << "  (buffer moved → p is now DANGLING; reading *p would be UB)\n";
    // cout << *p;                       // ❌ don't — dangling pointer to freed memory

    // ---- Part 2: erasing while iterating ----
    cout << "\nPart 2: erase even numbers while iterating\n";
    vector<int> nums = {1, 2, 3, 4, 5, 6};

    // ❌ BROKEN version (leave commented — UB):
    // for (auto it = nums.begin(); it != nums.end(); ++it)
    //     if (*it % 2 == 0) nums.erase(it);   // erase invalidates it → ++it is UB

    // ✅ CORRECT: erase() returns the next valid iterator
    for (auto it = nums.begin(); it != nums.end(); ) {
        if (*it % 2 == 0)
            it = nums.erase(it);        // use the returned iterator; don't ++
        else
            ++it;
    }

    cout << "  after removing evens: ";
    for (int n : nums) cout << n << " ";
    cout << " (expected 1 3 5)\n";
}
