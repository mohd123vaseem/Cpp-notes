# Topic 4 — OOP + Polymorphism + vtables

Tier 1 #4 (~3 days). One of the **most-asked** interview areas. Strategy: **clear the fundamentals fast, then spend real time on the high-ROI "juicy" internals** (vtables, virtual dispatch, overloading vs overriding).

> **Scope note:** SOLID principles + design patterns are **NOT** in this topic — they're **LLD-round** material (Parallel Track B), a separate interview type. This topic is *language-level* OOP.

## Why this topic matters

Polymorphism internals (how virtual dispatch actually works at the binary level) is exactly what separates candidates. And the "basics" (pillars, overloading vs overriding) get asked verbatim as openers. Ties directly into Topic 5 (ctors/dtors) and the vtable connects to virtual destructors.

---

## Progress tracker (sub-topics)

### Part A — Fundamentals (clear the basics fast)
| # | Sub-topic | The classic question | Status |
|---|-----------|----------------------|--------|
| 1 | **The 4 pillars** — Encapsulation, Abstraction, Inheritance, Polymorphism | "What are the pillars of OOP?" | ✅ Done |
| 2 | **Classes, access specifiers, `static` members, `this`** | "What does a static member mean?" | ✅ Done |
| 3 | **Inheritance types** (public/protected/private) + **composition vs inheritance** | "Composition vs inheritance — when each?" | ✅ Done |

### Part B — The high-ROI meat
| # | Sub-topic | The classic question | Status |
|---|-----------|----------------------|--------|
| 4 | ⭐ **Overloading vs Overriding vs Hiding** | "Difference between overloading and overriding?" | ⬜ Pending |
| 5 | ⭐ **Virtual functions + vtable/vptr** (dynamic dispatch mechanics) | "How do virtual functions work under the hood?" | ⬜ Pending |
| 6 | ⭐ **Virtual destructors** | "Why does a polymorphic base need a virtual destructor?" | ⬜ Pending |
| 7 | **Abstract classes & pure virtual** (interfaces) | "What is an abstract class / pure virtual?" | ⬜ Pending |
| 8 | **Object slicing** | "What is object slicing?" | ⬜ Pending |
| 9 | **Static vs dynamic binding** (early vs late) | "Early vs late binding?" | ⬜ Pending |
| 10 | ⚠️ **Virtual calls in ctor/dtor** (the gotcha) | "What happens if you call a virtual in a constructor?" | ⬜ Pending |
| 11 | **RTTI & `dynamic_cast`** | "How does the runtime know the real type?" | ⬜ Pending |
| 12 | **Diamond problem & virtual inheritance** (lighter) | "What's the diamond problem?" | ⬜ Pending |
| 13 | **`override` / `final`** | "What does `override` protect against?" | ⬜ Pending |
| 14 | **pimpl idiom** | "What is pimpl and why use it?" | ⬜ Pending |

---

## Sub-topic 1 — The 4 Pillars of OOP

**Memory hook: "A PIE"** — Abstraction, Polymorphism, Inheritance, Encapsulation.

### 1. Encapsulation — *bundle data + behavior, hide the internals*
Group **data** + the **functions that operate on it** into one class, and **hide internal state** behind a controlled interface (private members + public methods).
```cpp
class BankAccount {
    double balance;                    // private — no direct access
public:
    void deposit(double amt) { if (amt > 0) balance += amt; }  // enforces rules
    double getBalance() const { return balance; }
};
```
Outsiders can't do `account.balance = -5000;` — they must go through `deposit()`, which enforces the rules.
> *"Bundle data with the methods that use it, expose only a controlled interface."*

### 2. Abstraction — *expose what, hide how*
Show **what** an object does, hide **how**. Interact with a simple interface without knowing the messy implementation.
```cpp
class Car { public: void drive(); };   // you know WHAT, not HOW (fuel injection, transmission…)
```
Often achieved via abstract classes / interfaces (pure virtual — sub-topic 7).
> *"Expose a simple interface, hide the complex implementation."*

**Encapsulation vs Abstraction** (common comparison):
- **Encapsulation** = the *mechanism* — bundling + access control. *How* you hide.
- **Abstraction** = the *design idea* — showing only essentials. *What* you hide.
- Encapsulation is how you *achieve* abstraction.

### 3. Inheritance — *reuse & extend a base class ("is-a")*
A **derived** class inherits a **base** class's members, reusing and specializing. Models an **is-a** relationship.
```cpp
class Animal { public: void breathe(); };
class Dog : public Animal { public: void bark(); };   // Dog IS-A Animal → gets breathe()
```
> *"A derived class reuses and extends a base class (an 'is-a' relationship)."*

### 4. Polymorphism — *one interface, many behaviors*
"Poly" = many, "morph" = forms. The **same call** behaves differently depending on the actual object type.
```cpp
class Shape  { public: virtual double area() = 0; };
class Circle : public Shape { double area() override { /* πr² */ } };
class Square : public Shape { double area() override { /* side² */ } };

Shape* s = getSomeShape();
s->area();   // calls Circle::area OR Square::area — decided at RUNTIME
```
Powered by virtual functions + the vtable (sub-topic 5).
> *"The same interface produces different behavior depending on the actual object type."*

**Two kinds:**
- **Compile-time (static)** — function overloading, templates (sub-topic 4).
- **Runtime (dynamic)** — virtual functions (sub-topics 5–6).

### Summary
| Pillar | One-liner | Mechanism in C++ |
|--------|-----------|------------------|
| **Encapsulation** | bundle data + methods, hide internals | private/public, classes |
| **Abstraction** | expose *what*, hide *how* | abstract classes / interfaces |
| **Inheritance** | reuse & extend a base ("is-a") | `class D : public B` |
| **Polymorphism** | one interface, many behaviors | virtual (runtime), overloading/templates (compile-time) |

---

## Sub-topic 2 — Classes, Access Specifiers, `static` Members, `this`

### Access specifiers
| Specifier | Accessible from |
|-----------|-----------------|
| `public` | anywhere |
| `protected` | the class itself **+ derived classes** |
| `private` | the class itself **only** |

- **`class` defaults to `private`; `struct` defaults to `public`** — that's the *only* real difference between them.
- `protected` exists for inheritance — hidden from outsiders, usable by derived classes.

### `static` members — belong to the CLASS, not to any object
A normal member exists **once per object**; a `static` member exists **once for the whole class** — all objects share it.
```cpp
class Counter {
public:
    int id;                 // each object has its OWN id
    static int count;       // ONE count shared by ALL objects
    Counter() { count++; }
};
int Counter::count = 0;     // must DEFINE it once outside the class
```
```
 obj1     obj2     obj3
 id=1     id=2     id=3      ← each has its own id
      \     |     /
       ┌──────────┐
       │ count=3  │          ← ONE shared count for the whole class
       └──────────┘
```
*(This is exactly why `static int* ref_cnt` was wrong in the hand-written `shared_ptr` — a static member is shared by **every** object, so all shared_ptrs would share one count regardless of which object they point to.)*

### `static` methods — no object, therefore no `this`
```cpp
class Math { public: static int square(int x) { return x * x; } };
Math::square(5);   // called on the CLASS, no object
```

**Why no `this`:** `this` is a pointer to *the specific object a method was called on*. A static method is called on the **class** (`Math::square(5)`) — there's **no object**, so there's **nothing for `this` to point at**. It's not null or hidden; `this` genuinely doesn't exist in a static method.

**Why "no `this`" ⟹ "can only touch static members":** accessing a non-static member is *secretly* done through `this`:
```cpp
class Counter {
    int id;              // non-static — one per object
    static int count;    // static — one for the class
public:
    void normalMethod()      { id = 5;    }  // compiler REALLY means: this->id = 5
    static void staticMethod(){
        id = 5;      // ❌ ERROR — no `this`, so WHICH object's id?
        count = 5;   // ✅ OK — count belongs to the class, needs no object
    }
};
```
Non-static members only exist *inside an object*, reached *through* `this`. No `this` → no way to say "which object's `id`" → static methods can only touch things that don't belong to an object: other **static** members.

Mirror image:
- **Non-static method** → has `this` → can touch **both** non-static (via `this`) and static.
- **Static method** → no `this` → can touch **only static**.

### The `this` pointer
Inside a member function, `this` = a **pointer to the object the method was called on**.
```cpp
class Point {
    int x;
public:
    void setX(int x)   { this->x = x; }      // disambiguate member vs parameter
    Point& moveBy(int dx){ x += dx; return *this; }  // return object → chaining
};
```
Two uses: **disambiguation** (`this->x = x`) and **`return *this`** (chaining + copy assignment — you did this in `shared_ptr`).
- `this` is a **pointer** (`Point*`) → hence `this->x` and `return *this` (dereference to get the object).
- In a **const method**, `this` is `const Point*` — the mechanism behind "const methods can't modify the object" (Topic 2).

### Summary
- **Access:** public (anyone) / protected (class + derived) / private (class only). `class`→private, `struct`→public default.
- **`static` member** = one shared copy for the whole class; define once outside.
- **`static` method** = no object → **no `this`** → can only touch static members.
- **`this`** = pointer to the current object; used for disambiguation and `return *this`; `const` inside const methods.

---

## Sub-topic 3 — Inheritance Types + Composition vs Inheritance

### Part 1 — The three inheritance types
The access specifier on inheritance controls **how the base's members appear in the derived class**:
```cpp
class Base { public: int pub; protected: int prot; private: int priv; };

class D_public    : public Base    { };
class D_protected : protected Base { };
class D_private   : private Base   { };
```
(`priv` is **never** accessible in any derived class — private members stay with the base.)

| Inheritance type | `public` member becomes | `protected` member becomes |
|------------------|-------------------------|----------------------------|
| **`public`** | stays `public` | stays `protected` |
| **`protected`** | becomes `protected` | stays `protected` |
| **`private`** | becomes `private` | becomes `private` |

**Rule in words:** the inheritance type is a **ceiling** on access — members can only get *more* restrictive, never looser.

**Which to use:** **~always `public`** — that's real "is-a" inheritance, and the outside world can treat the derived object *as* the base (essential for polymorphism).
```cpp
class Dog : public Animal { };   // Dog IS-A Animal ✅ — the normal case
```
`private`/`protected` inheritance mean "implemented-in-terms-of," hide the base's interface, and **lose polymorphism** (outside world can't treat derived as base). Rare — composition usually does the job better. Just recognize them.

### Part 2 — Composition vs Inheritance ⭐ (the design question)
- **Inheritance = "is-a"** — the derived *is a kind of* the base.
  ```cpp
  class Dog : public Animal { };        // Dog IS-A Animal
  ```
- **Composition = "has-a"** — the class *contains* another object and uses it.
  ```cpp
  class Car { Engine engine; Wheels wheels[4]; };   // Car HAS-A Engine
  ```

**The test:** which sentence is true?
- "A Dog **is a**n Animal" ✅ → inheritance
- "A Car **has an** Engine" ✅ → composition
- "A Car **is an** Engine" ❌ → don't inherit

**Why "prefer composition over inheritance":**
1. **Inheritance = tight coupling** — derived depends on base internals; a base change can break all derived classes (fragile base class problem).
2. **Composition = flexible** — swap the contained object, change at runtime, compose multiple pieces.
3. **Inheritance exposes the base's whole interface**; composition exposes only what you choose.
4. **Deep hierarchies get rigid**; composition stays flat and modular.

```cpp
// ❌ inheritance abused for reuse:
class Server : public Logger { };   // a Server IS-A Logger? No — it just uses one.
// ✅ composition — "Server HAS-A Logger":
class Server { Logger logger; public: void handle(){ logger.log("request"); } };
```

> **Guideline:** use **inheritance** only for a genuine *is-a* relationship needing polymorphism. For reuse / "has-a" / "uses-a", use **composition**. When unsure, **prefer composition** (looser coupling, more flexible).

### Summary
- Inheritance types can only make base members **more restrictive**. `public` = is-a (use ~always); `private`/`protected` = rare, lose polymorphism.
- Base's `private` members are never accessible in derived classes.
- **Inheritance = is-a; Composition = has-a.** Prefer composition; inherit only for true is-a + polymorphism.

---

## Code examples in this folder

| File | Demonstrates |
|------|--------------|
| _(added as we go)_ | |
