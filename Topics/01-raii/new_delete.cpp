// ============================================================================
// new / delete — manual heap allocation
// ----------------------------------------------------------------------------
// This shows the raw machinery that RAII wrappers (vector, unique_ptr...) hide.
// Watch: single object, array, a class so you SEE the destructor fire on delete,
// and the dangling-pointer cleanup.
//
// Build & run:
//   g++ -std=c++23 -Wall -Wextra new_delete.cpp -o new_delete
//   .\new_delete
// ============================================================================

#include <iostream>

// A class so we can watch construction/destruction tied to new/delete.
class Widget {
public:
    explicit Widget(int id) : id_(id) {
        std::cout << "  Widget(" << id_ << ") constructed\n";
    }
    ~Widget() {
        std::cout << "  Widget(" << id_ << ") destroyed\n";
    }
private:
    int id_;
};

int main() {
    // ----- 1) single object: new T  <->  delete p -----
    std::cout << "1) single int on the heap:\n";
    int* p = new int(42);                 // allocate + construct
    std::cout << "  *p = " << *p << "  (address: " << p << ")\n";
    delete p;                             // free
    p = nullptr;                          // avoid dangling use
    std::cout << "  after delete, p = " << p << " (nullptr)\n";

    // ----- 2) array: new T[n]  <->  delete[] p -----
    std::cout << "\n2) array of 5 ints on the heap:\n";
    int* arr = new int[5];
    for (int i = 0; i < 5; ++i) arr[i] = (i + 1) * 10;
    std::cout << "  arr[0]=" << arr[0] << ", arr[4]=" << arr[4] << '\n';
    delete[] arr;                         // [] required for arrays
    arr = nullptr;

    // ----- 3) class object: delete runs the DESTRUCTOR, then frees -----
    std::cout << "\n3) single class object (watch the destructor):\n";
    Widget* w = new Widget(1);            // prints "constructed"
    delete w;                             // prints "destroyed", then frees
    w = nullptr;

    // ----- 4) array of class objects: delete[] destroys ALL of them -----
    std::cout << "\n4) array of 3 class objects:\n";
    Widget* ws = new Widget[3]{Widget(10), Widget(20), Widget(30)};
    delete[] ws;                          // destroys all 3 (reverse order)
    ws = nullptr;

    // ----- 5) delete on nullptr is safe (does nothing) -----
    std::cout << "\n5) delete on nullptr is a harmless no-op:\n";
    int* none = nullptr;
    delete none;                          // no crash, nothing happens
    std::cout << "  survived delete nullptr\n";

    std::cout << "\nEvery 'new' above had exactly one matching 'delete' "
                 "(same form). No leaks.\n";
}
