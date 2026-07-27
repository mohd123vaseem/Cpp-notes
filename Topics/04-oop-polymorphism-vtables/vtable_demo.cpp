// ============================================================================
// Virtual functions, the vtable & the vptr — made visible
// ----------------------------------------------------------------------------
// Shows:
//   1. sizeof proof that a class with virtuals carries a hidden vptr (+8 bytes)
//   2. dynamic dispatch: the DERIVED override runs through a BASE pointer
//   3. the "surprise": a NON-virtual function calls the BASE version instead
//
// Build & run:
//   g++ -std=c++23 -Wall -Wextra vtable_demo.cpp -o vtable_demo
//   .\vtable_demo
// ============================================================================

#include <iostream>
using namespace std;

// ---------------------------------------------------------------------------
// 1) Two nearly-identical classes: one WITHOUT virtuals, one WITH — so we can
//    compare their sizeof and see the vptr the virtual one carries.
// ---------------------------------------------------------------------------
struct PlainNoVirtual {
    int x;                       // just an int
    void hello() {}              // NON-virtual → no vtable, no vptr
};

struct HasVirtual {
    int x;                       // same single int...
    virtual void hello() {}      // ...but virtual → object gains a hidden vptr
    virtual ~HasVirtual() {}
};

// ---------------------------------------------------------------------------
// 2) A base/derived pair to demonstrate dispatch.
// ---------------------------------------------------------------------------
struct Animal {
    virtual void speak() { cout << "  Animal: ...\n"; }   // virtual  → overridable
    void          move()  { cout << "  Animal: moves\n"; } // NON-virtual → NOT dispatched
    virtual ~Animal() {}
};

struct Dog : public Animal {
    void speak() override { cout << "  Dog: Woof\n"; }     // overrides speak()
    void move()           { cout << "  Dog: runs\n"; }     // HIDES move() (non-virtual)
};

int main() {
    // --- 1) sizeof: the hidden vptr costs one pointer ---------------------
    cout << "1) The hidden vptr (sizeof proof):\n";
    cout << "   sizeof(PlainNoVirtual) = " << sizeof(PlainNoVirtual)
         << "  (just an int)\n";
    cout << "   sizeof(HasVirtual)     = " << sizeof(HasVirtual)
         << "  (int + hidden vptr => bigger by one pointer)\n";
    cout << "   sizeof(void*)          = " << sizeof(void*)
         << "  (a pointer's size on this platform)\n\n";

    // --- 2) Dynamic dispatch through a base pointer -----------------------
    cout << "2) Virtual call through a base pointer -> DERIVED runs:\n";
    Animal* a = new Dog();     // base pointer, derived object
    a->speak();                // virtual  -> follows vptr -> Dog::speak  ("Woof") ✅

    // --- 3) The "surprise": non-virtual uses the POINTER's type ----------
    cout << "\n3) Non-virtual call through the same base pointer -> BASE runs:\n";
    a->move();                 // NON-virtual -> compiler uses a's type (Animal*) -> Animal::move
    cout << "   (a really points to a Dog, but move() isn't virtual, so the\n"
            "    POINTER's type decided -> Animal::move, not Dog::move)\n";

    // Proof it's about virtual, not the object: call move() on a real Dog.
    Dog d;
    cout << "\n   Same object as a real Dog (d.move()): ";
    d.move();                  // Dog::move -> "runs" (now the static type IS Dog)

    delete a;
}
