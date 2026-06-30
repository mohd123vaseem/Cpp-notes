// ============================================================================
// Construction / Destruction Order
// ----------------------------------------------------------------------------
// Watch the exact sequence: base -> members (declaration order) -> ctor body,
// and destruction as the precise reverse.
//
// Build & run:
//   g++ -std=c++23 -Wall -Wextra construction_order.cpp -o construction_order
//   .\construction_order
// ============================================================================

#include <iostream>

// Two member types — each announces its own birth and death.
class Engine {
public:
    Engine()  { std::cout << "  Engine built\n"; }
    ~Engine() { std::cout << "  Engine destroyed\n"; }
};

class Wheels {
public:
    Wheels()  { std::cout << "  Wheels built\n"; }
    ~Wheels() { std::cout << "  Wheels destroyed\n"; }
};

// Base class.
class Vehicle {
public:
    Vehicle()  { std::cout << "  Vehicle (base) built\n"; }
    ~Vehicle() { std::cout << "  Vehicle (base) destroyed\n"; }
};

// Derived class with two members, declared engine_ THEN wheels_.
class Car : public Vehicle {
    Engine engine_;     // member 1 (declared first  -> built first)
    Wheels wheels_;     // member 2 (declared second -> built second)
public:
    Car()  { std::cout << "  Car body runs\n"; }
    ~Car() { std::cout << "  ~Car body runs\n"; }
};

int main() {
    std::cout << "Creating a Car:\n";
    {
        Car c;          // construction happens here
        std::cout << "--- Car fully built, now leaving scope ---\n";
    }                   // destruction happens here, in reverse

    std::cout << "\nExpected construction order:\n"
                 "  Vehicle (base) -> Engine -> Wheels -> Car body\n"
                 "Expected destruction order (exact reverse):\n"
                 "  ~Car body -> Wheels -> Engine -> Vehicle (base)\n";
}
