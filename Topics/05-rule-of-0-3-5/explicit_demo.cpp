// ============================================================================
// `explicit` — why a single-argument constructor is dangerous without it
// ----------------------------------------------------------------------------
// COMPLETE, RUNNABLE example. Everything used here is defined in this file.
//
// Build & run:
//   g++ -std=c++23 -Wall -Wextra explicit_demo.cpp -o explicit_demo
//   .\explicit_demo
// ============================================================================

#include <iostream>
#include <string>
using namespace std;

// ----------------------------------------------------------------------------
// 1) A plain data class. Nothing special — just something to own.
//    (This is what "Person" was in the earlier fragments.)
// ----------------------------------------------------------------------------
struct Person {
    string name;
    int age;
    Person(string n, int a) : name(n), age(a) {
        cout << "  [Person] " << name << " CREATED\n";
    }
    ~Person() {
        cout << "  [Person] " << name << " DESTROYED\n";
    }
};

// ----------------------------------------------------------------------------
// 2) A cut-down version of YOUR shared_ptr (same single-arg constructor).
//    Note: the constructor takes ONE parameter (T* p). ref_cnt is made inside.
// ----------------------------------------------------------------------------
template <typename T>
class shared_ptr {
    T*   ptr;
    int* ref_cnt;
public:
    // ⚠️ NOT explicit → this doubles as an IMPLICIT CONVERSION from T* to shared_ptr<T>
    shared_ptr(T* p) {
        ptr = p;
        ref_cnt = new int(1);
        cout << "  [shared_ptr] now OWNS " << ptr->name << " (count=1)\n";
    }

    shared_ptr(const shared_ptr& other) {
        ptr = other.ptr;
        ref_cnt = other.ref_cnt;
        ++(*ref_cnt);
        cout << "  [shared_ptr] copied (count=" << *ref_cnt << ")\n";
    }

    ~shared_ptr() {
        --(*ref_cnt);
        if (*ref_cnt == 0) {
            cout << "  [shared_ptr] count hit 0 -> DELETING the Person\n";
            delete ptr;
            delete ref_cnt;
        }
    }

    T* operator->() { return ptr; }
};

// ----------------------------------------------------------------------------
// 3) A function that TAKES OWNERSHIP.
//    (This is what "process" was.) It accepts a shared_ptr BY VALUE, meaning
//    "hand me ownership of this object" — when it returns, its shared_ptr dies.
// ----------------------------------------------------------------------------
void takeOwnership(shared_ptr<Person> sp) {
    cout << "  [takeOwnership] using " << sp->name << ", age " << sp->age << '\n';
}   // <-- sp is destroyed HERE. If count hits 0, the Person is DELETED.

int main() {
    cout << "=== THE PROBLEM: implicit conversion silently transfers ownership ===\n\n";

    // I create a Person and hold a RAW pointer to it. *I* own it. I plan to
    // delete it myself at the end.
    Person* raw = new Person("vaseem", 23);

    cout << "\nCalling takeOwnership(raw) -- I'm just passing a raw pointer...\n";

    // ⚠️ THIS IS THE BUG.
    // takeOwnership wants a shared_ptr<Person>, but I passed a Person*.
    // Because the constructor is NOT explicit, the compiler SILENTLY does:
    //        takeOwnership( shared_ptr<Person>(raw) )
    // I never asked for that! A shared_ptr now owns my object, and when it
    // dies at the end of takeOwnership, it DELETES my Person.
    takeOwnership(raw);

    cout << "\nBack in main. My 'raw' pointer is now DANGLING -- the object\n"
            "was already deleted inside takeOwnership, without me asking.\n";

    // Now MY delete runs on already-freed memory => DOUBLE FREE (undefined behavior).
    cout << "Now I call 'delete raw' as I always planned...\n";
    // delete raw;   // <-- UNCOMMENT to see the double-free crash (it's UB)
    cout << "  (delete commented out -- it would be a DOUBLE FREE / crash)\n";

    cout << "\n=== THE FIX ===\n"
            "Mark the constructor `explicit`:\n"
            "    explicit shared_ptr(T* p) { ... }\n"
            "Then `takeOwnership(raw);` NO LONGER COMPILES. You are forced to write\n"
            "    takeOwnership(shared_ptr<Person>(raw));\n"
            "...which makes the ownership transfer VISIBLE. You can't do it by accident.\n";
}
