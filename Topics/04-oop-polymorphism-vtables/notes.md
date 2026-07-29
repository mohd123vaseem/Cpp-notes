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
| 4 | ⭐ **Overloading vs Overriding vs Hiding** | "Difference between overloading and overriding?" | ✅ Done |
| 5 | ⭐ **Virtual functions + vtable/vptr** (dynamic dispatch mechanics) | "How do virtual functions work under the hood?" | ✅ Done |
| 6 | ⭐ **Virtual destructors** | "Why does a polymorphic base need a virtual destructor?" | ✅ Done |
| 7 | **Abstract classes & pure virtual** (interfaces) | "What is an abstract class / pure virtual?" | ✅ Done |
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

## Sub-topic 4 — Overloading vs Overriding vs Hiding ⭐

Three similar words, very different meanings. Interviewers probe the overriding-vs-hiding difference because it's subtle and causes real bugs.

### 1. Overloading — same name, DIFFERENT params, same scope
Multiple functions, same name, different parameter lists, in the same class. Compiler picks by arguments.
```cpp
class Printer {
public:
    void print(int i);
    void print(double d);
    void print(string s);
};
```
- Resolved at **compile time** (static / early binding) → **compile-time polymorphism**.
- ⚠️ **Return type alone does NOT count** — can't overload on return type only.
- **No inheritance needed** — within one class.

### 2. Overriding — derived redefines a `virtual` base function, SAME signature
```cpp
class Animal { public: virtual void speak() { cout << "..."; } };
class Dog : public Animal { public: void speak() override { cout << "Woof"; } };

Animal* a = new Dog();
a->speak();   // "Woof" — DERIVED version runs, chosen at RUNTIME
```
- Requires **inheritance** + base function **`virtual`** + **identical signature**.
- Resolved at **runtime** (dynamic / late binding) via the vtable (sub-topic 5) → **runtime polymorphism**.
- Use `override` to have the compiler verify you actually overrode something.

### 3. Hiding (name hiding / shadowing) — the sneaky one ⚠️
If a derived class declares a function with the **same name** as a base function, it **hides ALL base functions of that name** (regardless of signature) — unless it's a proper override.
```cpp
class Base {
public:
    void foo(int x);
    void foo(double d);
};
class Derived : public Base {
public:
    void foo(string s);   // hides BOTH base foos!
};

Derived d;
d.foo("hi");   // ✅ Derived::foo(string)
d.foo(42);     // ❌ COMPILE ERROR — Base::foo(int) is HIDDEN, not overloaded
```
The moment `Derived` declares *any* `foo`, all inherited `foo`s become invisible — the compiler stops looking in `Base` once it finds the name in `Derived`. This has **nothing to do with `virtual`** — it's pure name lookup, and almost always a mistake.

**Fix** — un-hide with `using`:
```cpp
class Derived : public Base {
public:
    using Base::foo;      // bring Base's foos back into scope
    void foo(string s);
};
d.foo(42);   // ✅ now finds Base::foo(int)
```

### ⭐ The critical distinction: Overriding vs Hiding
| | Overriding | Hiding |
|---|---|---|
| Base function `virtual`? | **Yes** | No (or signature differs) |
| Signature | **identical** to base | **different** from base |
| What happens | derived replaces base **in the vtable** → runtime dispatch | derived name **shadows** the base name → base version invisible |
| Through a **base pointer** | derived version runs ✅ | **base version runs** (no dispatch) ⚠️ |
```cpp
Base* b = new Derived();
b->someFunc();
// OVERRIDDEN (virtual, same sig) → Derived's version runs
// HIDDEN (non-virtual or diff sig) → BASE's version runs — surprise!
```

#### Why the "surprise" — the pointer's type vs the actual object
Calling through a **base pointer that points to a derived object** (`Base* b = new Derived();`):

**Case A — `virtual` → Derived runs ✅**
```cpp
class Base    { public: virtual void someFunc() { cout << "Base"; } };
class Derived : public Base { public: void someFunc() override { cout << "Derived"; } };
Base* b = new Derived();
b->someFunc();   // "Derived" — virtual → C++ checks the ACTUAL OBJECT at runtime
```
**Case B — NOT `virtual` → Base runs ⚠️ (the surprise)**
```cpp
class Base    { public: void someFunc() { cout << "Base"; } };      // no virtual
class Derived : public Base { public: void someFunc() { cout << "Derived"; } };
Base* b = new Derived();
b->someFunc();   // "Base"!  even though b really points to a Derived
```
Without `virtual`, C++ decides **at compile time based on the POINTER'S TYPE** (`Base*`), not the actual object → calls `Base::someFunc`. The derived version is hidden, never consulted.
> - **virtual** → decision based on the **actual object** (runtime) → Derived runs.
> - **not virtual** → decision based on the **pointer/reference type** (compile time) → Base runs.
>
> That one word `virtual` is the entire difference (mechanically explained in sub-topic 5).

#### What if you change the RETURN TYPE? (differs for all three)
| Mechanism | Change only the return type → |
|-----------|-------------------------------|
| **Overloading** | ❌ **illegal** — can't overload on return type alone; params must differ. Compiler picks by call-site *arguments*, so it can't tell which return you meant. |
| **Overriding** | ❌ **error** — signature (incl. return) must match… **except covariant returns** ✅ |
| **Hiding** | ✅ **still hides** — hiding is by *name only*, return type is irrelevant |

```cpp
// Overloading — differ only by return type:
int foo(int x);  double foo(int x);        // ❌ error

// Overriding — return type must match:
class Base    { public: virtual int  speak(); };
class Derived : public Base { public: double speak(); };   // ❌ not an override

// EXCEPTION — covariant return (pointer/ref to a more-derived type in the SAME hierarchy):
class Base    { public: virtual Base*    clone(); };
class Derived : public Base { public: Derived* clone() override; };  // ✅ legal
//   Derived* IS-A Base* → safe & allowed (common in clone() patterns)

// Hiding — return type doesn't matter, name still shadows:
class Base    { public: void foo(int); };
class Derived : public Base { public: double foo(int); };  // still HIDES Base::foo
```

### Summary
| Term | Same name? | Params | `virtual`? | Inheritance? | Resolved |
|------|-----------|--------|-----------|--------------|----------|
| **Overloading** | yes | **different** | no | no (same class) | compile time |
| **Overriding** | yes | **same** | **yes** | yes | **run time** |
| **Hiding** | yes | any | no | yes | compile time (name lookup) |

- **Overloading** — same name, *different params*, same class → compile-time choice.
- **Overriding** — same name, *same params*, `virtual`, derived class → runtime dispatch.
- **Hiding** — derived declares the same *name*, silently shadowing the base's versions → usually a bug.

---

## Sub-topic 5 — Virtual Functions + vtable/vptr ⭐

*"How do virtual functions work under the hood?"* — explains mechanically why `virtual` makes the derived version run through a base pointer.

### The problem
```cpp
Shape* s = getSomeShape();   // could be Circle, Square, Triangle...
s->area();                   // must call the CORRECT area() for whatever it really is
```
At compile time the compiler only knows `s` is a `Shape*` — the actual type is decided at runtime. The vtable mechanism bridges that gap.

### The two pieces

**vtable (virtual table):** for **each class with virtual functions**, the compiler builds **one** hidden static table — an array of **function pointers**, one slot per virtual function, pointing to that class's version.
```cpp
class Animal { public: virtual void speak(){cout<<"...";} virtual void move(){cout<<"moves";} };
class Dog : public Animal { public: void speak() override {cout<<"Woof";} };  // move() not overridden
```
```
Animal's vtable:               Dog's vtable:
 speak → Animal::speak          speak → Dog::speak    ← overridden (slot swapped)
 move  → Animal::move           move  → Animal::move  ← inherited (not overridden)
```
**Overriding = swapping a function pointer in the vtable.**

**vptr (virtual pointer):** every **object** of such a class gets one hidden member — a pointer to its class's vtable, set at **construction** based on the object's **actual type**.
```
  Animal object a          Dog object d
  [ vptr ]──►Animal        [ vptr ]──►Dog's
  [ data ]   vtable        [ data ]   vtable
```

### How a virtual call works
```cpp
Animal* a = new Dog();
a->speak();
```
The compiler generates (roughly):
```
1. dereference a          → reach the OBJECT it points to  (a Dog object)
2. read THAT OBJECT's vptr → the vtable                     (Dog's vtable)
3. look up speak's slot    → function ptr                   (→ Dog::speak)
4. call through it                                          → runs Dog::speak ✅
```
This is **dynamic dispatch / late binding** — decided at **runtime**.

> ⚠️ **Precision:** a **pointer has no vptr** — the **object** does. "Follow a's vptr" is loose shorthand for *"follow `a` to the object, then read **that object's** vptr."* The pointer only gets you to the object; the object's vptr (set at construction to its real type) is what decides. It works because the object is a `Dog`, so its vptr → Dog's vtable → the lookup finds `Dog::speak`. The pointer's static type (`Animal*`) is irrelevant.

### Why non-virtual differs (the sub-topic-4 "surprise")
A non-virtual call has **no vtable lookup** — the compiler hard-codes a direct call by the **pointer's static type** at compile time:
```cpp
a->someNonVirtual();   // "a is Animal* → call Animal::someNonVirtual" — object type ignored
```
> **`virtual` = "go through the vtable" (runtime, actual object). non-virtual = "call directly by pointer type" (compile time).**

### Cost of virtual functions
- **Memory:** +1 pointer per object (the vptr, 8 bytes on 64-bit) + one vtable per class (small, shared).
- **Speed:** each call is **indirect** (follow vptr → look up → call) — an extra memory access, and **can't be inlined** (compiler doesn't know at compile time which function runs).
- Negligible usually; matters in hot loops. **You pay only when you use `virtual`** (unlike Java where all methods are virtual by default).

### Summary
- **vtable** — one per class; array of pointers to its virtual-function versions. Overriding swaps a slot.
- **vptr** — one hidden pointer per object → its class's vtable; set at construction by **actual** type.
- **Virtual call** = follow vptr → look up slot → call → **runtime** dispatch.
- Works because the vptr reflects the **object's real type**, not the pointer's static type.
- **Cost:** +1 pointer/object, +indirect non-inlinable call.

---

## Sub-topic 6 — Virtual Destructors

*"Why does a polymorphic base class need a virtual destructor?"* — builds directly on the vtable dispatch rule.

### The problem: deleting through a base pointer
```cpp
class Base {
public: ~Base() { cout << "~Base\n"; }              // NON-virtual destructor
};
class Derived : public Base {
    int* data;
public:
    Derived()  { data = new int[100]; }
    ~Derived() { delete[] data; cout << "~Derived\n"; }
};

Base* b = new Derived();   // base pointer, derived object
delete b;                  // prints only "~Base"
```
**`~Derived` never runs!** `delete[] data` is skipped → the 100 ints **leak**.

### Why — the same dispatch rule as sub-topic 5
`delete b` must call *a* destructor. With a **non-virtual** destructor the compiler decides by the **pointer's static type** (`Base*`) → calls `~Base` only. The object is really a `Derived`, but the pointer type wins — the exact "surprise." The derived part is never destroyed → leak. (Technically this is **undefined behavior**.)

### The fix: `virtual ~Base()`
```cpp
class Base { public: virtual ~Base() { cout << "~Base\n"; } };
```
Now `delete b` goes **through the vtable**: follows the object's vptr → finds `~Derived` → runs it **first**, then chains up to `~Base`:
```
~Derived
~Base
```
Both run, in reverse-of-construction order (derived → base, Topic 1). No leak. ✅

> 📖 **What is a "polymorphic class"?** A class with **at least one virtual function** (its own or inherited) — which is exactly what gives it a **vtable + vptr** and enables **runtime dispatch**. No virtual function → not polymorphic → no vtable, no dynamic dispatch, no `dynamic_cast`.
> ```cpp
> class A { void foo(); };            // ❌ not polymorphic (no virtual)
> class B { virtual void speak(); };  // ✅ polymorphic
> class C : public B { };             // ✅ polymorphic (inherits a virtual)
> ```
> Note: this means **runtime** polymorphism. Overloading/templates are *compile-time* polymorphism and don't make a class "polymorphic" in this technical sense.

### The rule
> **A class deleted through a base pointer (used polymorphically) MUST have a `virtual` destructor.**
>
> Heuristic: **if a class has ANY virtual function, give it a virtual destructor.**

Conversely, a class **not** meant as a polymorphic base (no virtuals) should **not** have a virtual destructor — you'd pay the vptr cost for nothing.

### Cost
Making the destructor virtual gives the class a **vtable + vptr** (if it didn't have one) → every object grows by one pointer. So it's "virtual **when polymorphic**," not "always." A plain value type (e.g. `Point`) shouldn't have one.

### Connect it back
The destructor is just another slot in the vtable. Non-virtual → pointer-type dispatch → wrong (base only). Virtual → object-vptr dispatch → correct (derived, then base). Virtual destructors = "apply dynamic dispatch to cleanup so the *whole* object is destroyed."

### Summary
- Non-virtual destructor + delete via base pointer → only `~Base` runs → derived part **leaks** (UB).
- Fix: `virtual ~Base()` → vtable dispatch → `~Derived` then `~Base`.
- Rule: polymorphic base **must** have a virtual destructor; any class with a virtual function should too.
- Cost: adds vptr/vtable → only for polymorphic classes.

---

## Sub-topic 7 — Abstract Classes & Pure Virtual Functions

How C++ expresses an **interface** — the concrete mechanism behind the Abstraction pillar.

### Pure virtual function — "you MUST override this"
A **pure virtual** has no implementation in the base, marked `= 0`, and **must** be overridden:
```cpp
class Shape {
public:
    virtual double area() = 0;   // pure virtual — no body, MUST be overridden
};
```
It says: *"every Shape has an `area()`, but Shape itself can't compute one — each concrete shape must."*

### Abstract class — "cannot be instantiated"
A class with **≥1 pure virtual function** is **abstract** → you cannot create an object of it:
```cpp
Shape s;    // ❌ error — abstract, can't instantiate (it's incomplete)
Shape* p;   // ✅ pointer/reference to Shape is fine
```

### Concrete class — override everything, then instantiable
A derived class becomes **concrete** only once it overrides **all** pure virtuals:
```cpp
class Circle : public Shape {
    double r;
public:
    Circle(double radius) : r(radius) {}
    double area() override { return 3.14159 * r * r; }
};
Circle c(5);   // ✅ concrete now
```
If it hadn't overridden `area()`, it would stay abstract.

### The interface pattern
```cpp
class Shape {
public:
    virtual double area() = 0;
    virtual void   draw() = 0;
    virtual ~Shape() = default;      // ✅ virtual destructor — it's polymorphic
};
class Circle : public Shape { /* overrides area(), draw() */ };
class Square : public Shape { /* overrides area(), draw() */ };

void render(Shape* s) { s->draw(); cout << s->area(); }   // works for ANY shape
```
`Shape` is the **interface/contract**; `render()` works with any concrete shape without knowing its type. Add a `Triangle` later → `render()` unchanged. **Program to the interface, not the implementation.**

### Notes
- **C++ has no `interface` keyword** — an abstract class with *only* pure virtuals (no data, no implemented methods) *is* an interface.
- Give abstract bases a **virtual destructor** (they're polymorphic, deleted through base pointers).
- **Nuance:** a pure virtual *can* still have a body (`void Base::foo() {…}`) — a callable default — but the class stays abstract and it must still be overridden. Rare; signals depth.

### Summary
- **Pure virtual** = `virtual T f() = 0;` → derived **must** override.
- **Abstract class** = has ≥1 pure virtual → **can't be instantiated**.
- Becomes **concrete** when all pure virtuals are overridden.
- C++ interfaces = abstract class of pure virtuals; give them a virtual destructor.

---

## Code examples in this folder

| File | Demonstrates |
|------|--------------|
| `vtable_demo.cpp` | `sizeof` proof of the hidden vptr; dynamic dispatch (derived runs via base ptr); the non-virtual "surprise" (base runs) |
