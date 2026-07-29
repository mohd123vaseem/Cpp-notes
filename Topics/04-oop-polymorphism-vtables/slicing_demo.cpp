// ============================================================================
// Object Slicing — using a polymorphic type BY VALUE loses the derived part
// ----------------------------------------------------------------------------
// Shows:
//   1. Animal a = d;        → sliced → base version runs
//   2. pass-by-value        → sliced at the function boundary
//   3. reference / pointer   → NO slicing, correct derived version runs
//
// Build & run:
//   g++ -std=c++23 -Wall -Wextra slicing_demo.cpp -o slicing_demo
//   .\slicing_demo
// ============================================================================

#include <iostream>
using namespace std;

class Animal {
public:
    virtual void speak() { cout << "  Animal: ...\n"; }
    virtual ~Animal() = default;
};

class Dog : public Animal {
    int tailLength = 42;                 // derived-specific data (gets sliced off)
public:
    void speak() override { cout << "  Dog: Woof (tail=" << tailLength << ")\n"; }
};

// Takes the base BY VALUE → anything passed here gets sliced.
void byValue(Animal a)        { cout << "byValue     -> "; a.speak(); }
// Takes the base BY REFERENCE → no copy, no slicing.
void byReference(const Animal& a) { cout << "byReference -> "; a.speak(); }

int main() {
    Dog d;

    cout << "1) Direct copy into a base VALUE (sliced):\n";
    Animal a = d;          // slicing: only the Animal part copied
    cout << "   Animal a = d; a.speak() -> "; a.speak();   // "Animal: ..."

    cout << "\n2) Pass BY VALUE (sliced at the boundary):\n";
    byValue(d);            // "Animal: ..." — Dog sliced into the parameter

    cout << "\n3) Pass BY REFERENCE / pointer (NO slicing):\n";
    byReference(d);        // "Dog: Woof" ✅
    Animal* p = &d;
    cout << "pointer     -> "; p->speak();                 // "Dog: Woof" ✅

    cout << "\nsizeof(Animal) = " << sizeof(Animal)
         << ", sizeof(Dog) = " << sizeof(Dog)
         << "  (Dog is bigger -> the extra part is what gets sliced off)\n";
}
