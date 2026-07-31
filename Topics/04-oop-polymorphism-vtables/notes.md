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
| 8 | **Object slicing** | "What is object slicing?" | ✅ Done |
| 9 | **Static vs dynamic binding** (early vs late) | "Early vs late binding?" | ✅ Done |
| 10 | ⚠️ **Virtual calls in ctor/dtor** (the gotcha) | "What happens if you call a virtual in a constructor?" | ✅ Done |
| 11 | **RTTI & `dynamic_cast`** | "How does the runtime know the real type?" | ✅ Done |
| 12 | **Diamond problem & virtual inheritance** (lighter) | "What's the diamond problem?" | ✅ Done |
| 13 | **`override` / `final`** | "What does `override` protect against?" | ✅ Done |
| 14 | **pimpl idiom** | "What is pimpl and why use it?" | ✅ Done |

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

## Sub-topic 8 — Object Slicing

The bug when you use polymorphic types **by value** instead of by pointer/reference — it silently destroys polymorphism.

### What slicing is
Copying a **derived object into a base object by value** → only the **base part** is copied; the derived-specific parts are **sliced off** and lost.
```cpp
class Animal { public: virtual void speak(){cout<<"Animal\n";} };
class Dog : public Animal {
    int tailLength;
public: void speak() override { cout<<"Woof\n"; }
};

Dog d;
Animal a = d;    // ⚠️ SLICING — only the Animal part of d is copied
a.speak();       // "Animal" — NOT "Woof"!
```
`a` is an actual `Animal` object — only the base sub-part came across; `tailLength` and the Dog-ness are gone.

### Why it happens — fixed size + base's vptr
A `Animal` variable is exactly `sizeof(Animal)`; a `Dog` is bigger. Pour a `Dog` into a `Animal`-sized box → the extra part doesn't fit → discarded.
```
   Dog d                       Animal a  (fixed Animal-sized box)
 │ Animal part │──copied──────►│ Animal part │   ← only this fits
 │ tailLength  │──SLICED─X                       (discarded)
```
Also: the copy gets **`Animal`'s vptr**, not `Dog`'s → `a` is genuinely an `Animal` → `a.speak()` calls `Animal::speak`. **Value semantics defeat virtual dispatch** — polymorphism *requires* pointers/references.

### The sneaky causes
```cpp
void process(Animal a) { a.speak(); }   // ⚠️ base BY VALUE
process(Dog{});                          // sliced at the boundary → "Animal"

std::vector<Animal> zoo;
zoo.push_back(Dog{});                    // ⚠️ sliced — vector stores Animal objects
```
→ pass polymorphic types by `const Base&`/`Base*`, and store `vector<Base*>` or `vector<unique_ptr<Base>>`.

### The fix: pointers or references (no copy → no slicing)
```cpp
Dog d;
Animal& ref = d;   ref.speak();   // "Woof" ✅
Animal* ptr = &d;  ptr->speak();  // "Woof" ✅
void process(const Animal& a);    // by reference → full Dog, dispatch intact
```

> **Rule: use polymorphic types through pointers or references, never by value.** By value → slicing → derived part lost + dispatch broken.

Bonus: an **abstract** base can't be sliced — `Animal a = d;` won't compile (can't instantiate the base). Another reason to favor pure-virtual interfaces.

### Summary
- **Slicing** = derived → base **by value** → derived part cut off; copy gets base's vptr → dispatch breaks.
- **Sneaky:** pass-by-value, `vector<Base>`.
- **Fix:** `Base*` / `const Base&` always. Abstract bases can't be sliced.

---

## Sub-topic 9 — Static vs Dynamic Binding

Formalizes the recurring theme: **when does the compiler decide which function to call?** "Binding" = connecting a call `f()` to the actual function that runs.

- **Static binding** (early binding) — decided at **compile time**.
- **Dynamic binding** (late binding) — decided at **run time**.

### Static binding — compile time (the default)
Resolved by the **static type** (declared type / pointer type), hard-coded:
```cpp
greet(42);              // static — overload picked by argument type
a.someNonVirtual();     // static — resolved by a's type
Base* b = new Derived();
b->nonVirtualFunc();    // static — resolved by POINTER type (Base*) → the "surprise"
```
Applies to: normal functions, **overloading**, **hiding**, **non-virtual** calls. Fast, inlinable; ignores the actual object type.

### Dynamic binding — run time (virtual only)
A **virtual** function called through a pointer/reference → resolved at runtime via the object's **vptr → vtable**:
```cpp
Animal* a = new Dog();
a->speak();             // dynamic — follows the object's vptr → Dog::speak
```
Applies to: virtual functions via pointer/reference. Slower (indirect, non-inlinable); picks the correct override for the **actual object**.

### The three conditions for dynamic binding (all required)
1. Function is **`virtual`**, **and**
2. Called through a **pointer or reference** (not a by-value object), **and**
3. There's an override to dispatch to.

Miss any one → falls back to static. Same `Dog d` in all three — only the **access** differs:
```cpp
Dog d;

Animal  a = d;    // by VALUE → sliced, 'a' is a pure Animal (gets Animal's vptr)
a.speak();        // "Animal" — STATIC (condition #2 fails: not ptr/ref)

Animal& r = d;    // by REFERENCE → no copy, still a Dog underneath
r.speak();        // "Woof"   — DYNAMIC ✅ (virtual + reference + override)

Animal* p = &d;   // by POINTER → no copy, still a Dog underneath
p->speak();       // "Woof"   — DYNAMIC ✅
```
**Why by-value is always static:** calling a virtual directly on a value object (`a.speak()`), the compiler *already knows* the exact type — a value object's type is fixed and can never secretly be a derived type. Nothing to decide at runtime → static. Only a **pointer/reference** can point to any derived object, so only those defer the decision to runtime.

> This is *why* slicing kills polymorphism: slicing only happens **by value**, and a by-value object can never satisfy condition #2 → every virtual call on it is statically bound to the base. **Polymorphism requires pointers or references.**

### Side-by-side
| | Static (early) | Dynamic (late) |
|---|---|---|
| Decided | compile time | run time |
| Based on | **static/declared type** | **actual object type** (via vptr) |
| Applies to | normal, overloading, hiding, non-virtual | **virtual** via pointer/reference |
| Mechanism | direct call, hard-coded | vptr → vtable → indirect call |
| Speed | faster, inlinable | slower, not inlinable |

### Everything in Topic 4 fits this one axis
- **Overloading** → static (compile-time by args)
- **Hiding** → static (compile-time name lookup)
- **Non-virtual override** → static (pointer type wins — the "surprise")
- **Virtual override** → dynamic (object's vptr wins — real polymorphism)
- **Object slicing** → forces static (by-value breaks the pointer/reference condition)

> **One-liner:** static = compiler picks by *declared type* at compile time (default); dynamic = runtime picks by *actual object type* via the vtable — only for a virtual function called through a pointer/reference.

---

## Sub-topic 10 — Virtual Calls in Constructors/Destructors ⚠️

*"What happens if you call a virtual function from a constructor?"* — a classic gotcha.

### The surprise
Inside a constructor/destructor, a virtual call runs the **current class's** version — **not** the derived override.
```cpp
class Base {
public:
    Base() { init(); }                          // calls Base::init, NOT Derived::init!
    virtual void init() { cout << "Base::init\n"; }
};
class Derived : public Base {
public:
    void init() override { cout << "Derived::init\n"; }
};

Derived d;   // prints "Base::init"  ← NOT "Derived::init"
```

### Why — construction order + the vptr
1. **Construction order** (Topic 1): base is built **before** derived — `Base()` runs first.
2. The **vptr** is set up **stage by stage** as each class's constructor runs.

During `Base()`, the `Derived` part doesn't exist yet → the vptr still points to **Base's vtable** → `init()` resolves to `Base::init`.
```
Constructing a Derived:
  1. Base()    runs → vptr → Base's vtable    → init() = Base::init
  2. Derived() runs → vptr → Derived's vtable → NOW virtual calls hit Derived
```

### Why it's actually correct (not a bug)
If `Base()` *could* call `Derived::init`, that override might touch derived members not yet initialized:
```cpp
class Derived : public Base {
    int* data;
public:
    Derived() { data = new int[100]; }
    void init() override { data[0] = 5; }   // data isn't set until Derived() runs!
};
```
Calling `Derived::init` from `Base()` would use uninitialized `data` → garbage/crash. So C++ deliberately resolves the call to the current class.

**Destructors, in reverse:** during `~Base()`, the `Derived` part is **already destroyed** (destruction order: derived → base), so a virtual call there also resolves to `Base`'s version — avoiding access to destroyed members.

### Rule
> **Don't call virtual functions from ctors/dtors expecting derived dispatch** — you get the current class's version. If you need derived behavior during setup, use **two-phase init** (construct, then call `init()` separately once fully built).

### Summary
- Virtual call in ctor/dtor → **current class's** version, not the derived override.
- **Why:** vptr is built stage by stage; during `Base()` only the base exists → vptr → base's vtable.
- **Intentional & safe:** avoids calling a derived override that touches not-yet-constructed / already-destroyed members.
- **Rule:** no polymorphic dispatch in ctors/dtors; use two-phase init if needed.

---

## Sub-topic 11 — RTTI & `dynamic_cast`

**RTTI = Run-Time Type Information** — ask, at runtime, "what is this object's **actual** type?" Powered by the vtable.

### The problem
```cpp
class Animal { public: virtual ~Animal() = default; };
class Dog : public Animal { public: void fetch(); };

void handle(Animal* a) {
    a->fetch();   // ❌ Animal has no fetch(). But IS it a Dog?
}
```
You need "if this is really a Dog, give me a `Dog*`." → `dynamic_cast`.

### `dynamic_cast` — the safe down-cast (base → derived)
```cpp
void handle(Animal* a) {
    Dog* d = dynamic_cast<Dog*>(a);   // "is a really a Dog?"
    if (d) d->fetch();                // succeeded → a IS a Dog
    else   cout << "not a dog\n";     // failed
}
```
| Cast target | On success | On **failure** |
|---|---|---|
| **Pointer** `dynamic_cast<Dog*>(a)` | valid `Dog*` | **`nullptr`** |
| **Reference** `dynamic_cast<Dog&>(a)` | valid `Dog&` | **throws `std::bad_cast`** |

Why differ? A pointer can be null (failure → nullptr); a reference **can't** be null (Topic 2) → must throw. Check pointer form with `if`, wrap reference form in `try/catch`.

### Under the hood (ties to the vtable)
Each class's **vtable** holds a pointer to a **`type_info`** describing its real type. `dynamic_cast` follows the object's vptr → vtable → type_info → checks if the actual type matches/derives from the target.

**Consequence:** `dynamic_cast` works **only on polymorphic types** (≥1 virtual function → a vtable). No virtual → no vtable → won't compile. (Another reason interface bases have a virtual destructor.)

### `typeid` — the other RTTI tool
```cpp
#include <typeinfo>
Animal* a = new Dog();
typeid(*a).name();              // "Dog" (mangled, compiler-specific)
if (typeid(*a) == typeid(Dog)) { /* exact-type match */ }
```
⚠️ `typeid(*a)` (dereferenced) → **actual runtime type** (Dog); `typeid(a)` (the pointer) → **static type** (`Animal*`). And `typeid` checks **exact** type:
- **`dynamic_cast`** — "is it this type **or derived from it**?" (is-a) → safe casting.
- **`typeid`** — "is it **exactly** this type?" → exact comparison.

### ⚠️ Design smell
Heavy `dynamic_cast`/type-checking usually means you're re-implementing what **virtual functions do for free**:
```cpp
// ❌ RTTI-heavy:
if (auto d = dynamic_cast<Dog*>(a)) d->makeSound();
else if (auto c = dynamic_cast<Cat*>(a)) c->makeSound();
// ✅ virtual instead:
a->makeSound();   // each type overrides makeSound() — no casting
```
Use `dynamic_cast` only when you genuinely need the concrete type; reach for virtual functions first.

### Cost
`dynamic_cast` does a runtime type-check (walks the hierarchy) — slower than `static_cast`. RTTI can be disabled (`-fno-rtti`) to save space — **Chromium does this** (uses its own type systems). *(bonus-track talking point.)*

### Summary
- **RTTI** = query actual type at runtime; stored in the vtable (`type_info`).
- **`dynamic_cast<Derived*>(base)`** = safe down-cast → derived ptr, else `nullptr` (ptr) / throws `bad_cast` (ref).
- Works **only on polymorphic types**.
- **`typeid`** = exact-type info; `typeid(*ptr)` = actual runtime type.
- Heavy `dynamic_cast` = design smell → prefer virtual functions.
- One of the **four casts** (full treatment in Topic 6).

---

## Sub-topic 12 — The Diamond Problem & Virtual Inheritance (lighter)

C++'s multiple-inheritance gotcha.

### Multiple inheritance + the diamond
C++ allows inheriting from more than one base. When two bases share a **common** base, you get a diamond:
```cpp
class A { public: int value; };
class B : public A { };
class C : public A { };
class D : public B, public C { };
```
```
        A
       / \
      B   C
       \ /
        D
```

### The problem: two copies of A
`D` inherits `A` through **both** `B` and `C` → **two separate copies** of `A`'s members:
```cpp
D d;
d.value = 5;      // ❌ AMBIGUOUS — B's A::value or C's A::value?
d.B::value = 5;   // the A-copy via B
d.C::value = 10;  // a DIFFERENT A-copy via C
```

### The fix: virtual inheritance
Mark `A` inheritance `virtual` in both `B` and `C` → share **one** `A`:
```cpp
class A { public: int value; };
class B : virtual public A { };
class C : virtual public A { };
class D : public B, public C { };

D d;
d.value = 5;   // ✅ only ONE shared A — no ambiguity
```
```
        A       ← ONE shared A
       / \
      B   C
       \ /
        D
```

### Mechanism (light)
With virtual inheritance the shared base isn't at a fixed per-path offset, so the compiler adds a hidden indirection (a vbase pointer) so `B` and `C` both find the *one* shared `A`. Small runtime cost → why it isn't the default.

### Practical reality
> Multiple inheritance and the diamond are **rare** in real code — mostly relevant when combining **interfaces** (abstract classes, no data), where there's no data to duplicate so the diamond is harmless (often no `virtual` needed).
```cpp
class Drawable     { public: virtual void draw() = 0; };
class Serializable { public: virtual void save() = 0; };
class Widget : public Drawable, public Serializable { /* implements both */ };  // MI done well
```

### 💡 Doubts clarified (with memory layouts)

#### 1. What is a "sub-object"?
A derived object **physically contains** its base's data as a region *inside* it — that region is the **base sub-object**. Inheritance is **one object** with the base's part embedded inside it, **not** two separate linked objects.
```cpp
class Animal { int age; };
class Dog : public Animal { int tailLength; };
```
```
 One Dog object in memory:
 ┌───────────────────────┐
 │ age        (4 bytes)  │  ← the "Animal sub-object" (base region) — sits INSIDE the Dog
 ├───────────────────────┤
 │ tailLength (4 bytes)  │  ← Dog's own region
 └───────────────────────┘
```
"Sub-object" = a *region within* the bigger object (like a kitchen is a region of a house, not a separate house). This is *why* a `Dog*` works as a `Animal*`: the Animal region is literally right there at the start.

#### 2. What does "B and C don't carry their own A" mean?
Each of B and C normally **embeds its own A sub-object**. Build a `D` (inherits both) **without** virtual inheritance → D contains a whole B *and* a whole C, each bringing its own A → **two A regions**:
```
 One D object (NON-virtual):          d.value → ❌ AMBIGUOUS (which A?)
 ┌────────────────────┐
 │ B part:            │
 │   [ A sub-object ] │  ← A region #1 (B's own copy)
 ├────────────────────┤
 │ C part:            │
 │   [ A sub-object ] │  ← A region #2 (C's own copy)
 ├────────────────────┤
 │ D's own members    │
 └────────────────────┘
```
**Virtual** inheritance tells B and C: *don't embed your own A — there'll be ONE shared A in the D, and you each just point to it*:
```
 One D object (VIRTUAL):              d.value → ✅ ONE A, unambiguous
 ┌────────────────────┐
 │ B part: vbase ─────┼──┐
 ├────────────────────┤  │
 │ C part: vbase ─────┼──┤   ← both point to the SAME A
 ├────────────────────┤  │
 │ D's own members    │  │
 ├────────────────────┤  │
 │ [ A sub-object ]   │◄─┘   ← ONE shared A, placed once
 └────────────────────┘
```

#### 3. Standalone `B` or `C` — own A, or shared?
Their **own**. A lone `B` has exactly one A sub-object; there's nothing to share with:
```
 One standalone B object:
 ┌────────────────────┐
 │ vbase ptr ─────────┼──┐
 │ B's own members    │  │
 ├────────────────────┤  │
 │ [ A sub-object ]   │◄─┘   ← its own single A
 └────────────────────┘
```
Sharing only matters **within one most-derived object** reached via multiple paths (the diamond). **Rule:** *within any single object, virtual inheritance guarantees exactly ONE A, no matter how many paths reach it* — trivially one in a standalone B, collapsed-to-one in a D.

#### 4. vbase pointer & "indirection" (neither is a keyword)
- **vbase pointer** = a hidden compiler-added pointer, stored in each virtually-inheriting part, pointing to the shared base sub-object (`vbase` = *v*irtual *base*). You never write it.
- **indirection** = the general act of accessing something by *following a pointer first* (one extra hop), instead of grabbing it at a fixed offset.

**Why the pointer is needed:**
- *Non-virtual:* the base sits at a **fixed, known offset** inside the derived object → compiler bakes in the offset → **direct** access, no pointer.
- *Virtual:* the shared A's position **varies by whole-object layout** (in a standalone B the A is one place; inside a D it's elsewhere). B's code can't hard-code an offset → it stores a **vbase pointer** filled in at construction and **follows it at runtime** = indirection. That extra hop is the cost.
- Even a standalone virtual-`B` carries the vbase pointer, because when compiling `B` the compiler doesn't know whether it'll later be part of a diamond — so it always uses the indirection.

**vbase pointer vs vptr** (both hidden compiler pointers, different jobs):
| Pointer | Purpose | Points to |
|---|---|---|
| **vptr** | dynamic dispatch (pick the right override) | the class's **vtable** (function pointers) |
| **vbase pointer** | find the shared virtual base | the shared **base sub-object** |

### Summary
- **Diamond** = two bases share a common base → derived gets **two copies** → ambiguous access.
- **Fix:** `virtual` inheritance → all paths share **one** base instance.
- Small runtime indirection cost; rare in practice; mostly for combining interfaces.

---

## Sub-topic 13 — `override` and `final`

Two C++11 keywords that make virtual functions safer and clearer.

### `override` — "I intend to override; compiler, verify it"
```cpp
class Base    { public: virtual void speak(); };
class Derived : public Base { public: void speak() override { } };  // compiler confirms it overrides
```
If it doesn't actually override (typo, wrong signature, base not virtual), the compiler **errors**.

**Why it matters — catches the hiding bug (sub-topic 4):** a signature mismatch silently *hides* instead of overriding. `override` turns that into a compile error:
```cpp
class Base    { public: virtual void speak(int x); };
class Derived : public Base {
    void speak(double x) { }           // ⚠️ wrong sig → silently HIDES → polymorphism broken, compiles
    void speak(double x) override { }   // ✅ ERROR — "doesn't override anything" → bug caught
};
```
Also catches typo'd names and accidental `const`. **Best practice: always write `override` on intended overrides** — free insurance. (`override` implies `virtual`; no need to repeat it.)

### `final` — "no further overriding/inheritance"
**On a virtual function** — can't be overridden further:
```cpp
class Derived : public Base { public: void speak() final { } };
class Deeper  : public Derived { void speak() { } };   // ❌ ERROR — speak is final
```
**On a class** — can't be inherited from:
```cpp
class Widget final { };
class Sub : public Widget { };   // ❌ ERROR — Widget is final
```

**Why use `final`:**
- **Intent** — "not meant to be extended."
- **Prevents misuse** — locks behavior that must stay fixed.
- **Performance** — the compiler knows there are no further overrides → can **devirtualize** the call (skip the vtable lookup, even inline).

### Contrast
| Keyword | Says | Effect |
|---|---|---|
| **`override`** | "I *am* overriding — verify it" | compile error if it doesn't |
| **`final`** (method) | "this override is the last" | no further overriding |
| **`final`** (class) | "don't inherit from me" | no derivation |

Combinable: `void speak() override final { }` — overrides base *and* can't be overridden further.

### Summary
- **`override`** — compiler verifies you actually override; catches silent hiding bugs. **Always use.**
- **`final`** — method: no further override; class: no inheritance. Documents intent + enables devirtualization.

---

## Sub-topic 14 — The pimpl Idiom

**pimpl = "Pointer to IMPLementation"** — hide a class's implementation behind a pointer. Also called the "compilation firewall" / "opaque pointer."

### The problem
A class's **private** members still live in its **header**, visible to everyone who `#include`s it:
```cpp
// widget.h
class Widget {
public:  void doSomething();
private: int x; std::vector<int> data; ComplicatedType helper;   // all exposed in the header
};
```
1. **Leaky headers** — internals (types, their `#include`s) exposed to users.
2. **Recompilation cascade** (the big one) — change *any* private member → header changes → **every file that includes it recompiles.** In huge codebases (Chromium) one small change → thousands of files rebuilt.

### The fix — hide everything behind a pointer
```cpp
// widget.h  — PUBLIC header
class Widget {
public:
    Widget();
    ~Widget();                     // declared here, DEFINED in .cpp (see gotcha)
    void doSomething();
private:
    class Impl;                    // forward declaration ONLY
    std::unique_ptr<Impl> pImpl;   // the single pointer to implementation
};
```
```cpp
// widget.cpp — IMPLEMENTATION (hidden)
class Widget::Impl {
public:
    int x; std::vector<int> data; ComplicatedType helper;   // real details live HERE
    void doSomethingImpl() { /* ... */ }
};
Widget::Widget() : pImpl(std::make_unique<Impl>()) { }
Widget::~Widget() = default;                                  // in .cpp, where Impl is complete
void Widget::doSomething() { pImpl->doSomethingImpl(); }
```
The header now shows **no real members** — just a forward-declared `Impl` and a pointer.

### Why it fixes both
1. **Truly hidden** — header exposes only the public interface (encapsulation at the *file* level).
2. **No recompilation cascade** — change the internals → only `widget.cpp` recompiles; the header didn't change, so nothing that includes it rebuilds. **The "compilation firewall."**

### Why it works
A **pointer to an incomplete type is fine.** To *store* a `unique_ptr<Impl>`, the compiler only needs to know `Impl` **exists** (forward declaration) — not what's inside. The full definition is only needed in the `.cpp`.

### Gotchas
- **Destructor must be in the `.cpp`.** `unique_ptr<Impl>` needs the **complete** `Impl` to destroy it. An implicit/`=default` destructor *in the header* would try to generate it where `Impl` is still incomplete → error. So declare `~Widget();` in the header, `= default` it in the `.cpp` after `Impl` is defined.
- **Cost:** extra heap allocation for `Impl` + extra indirection per access + lost inlining. Use at **library boundaries / widely-included headers**, not every small class.

### Where used
Library/API design (stable ABI — change internals without breaking users' compiled code); large codebases to cut build times (Qt uses it heavily).

### Summary
- **pimpl** = move private members into an `Impl` class; header holds only a **forward declaration + `unique_ptr<Impl>`**.
- **Solves:** leaky headers + the **recompilation cascade** (internal changes rebuild only the `.cpp`).
- **Works because** a pointer to an incomplete type is legal.
- **Gotcha:** destructor in the `.cpp` (unique_ptr needs complete `Impl`).
- **Cost:** allocation + indirection → use where it pays off.

---

## Code examples in this folder

| File | Demonstrates |
|------|--------------|
| `vtable_demo.cpp` | `sizeof` proof of the hidden vptr; dynamic dispatch (derived runs via base ptr); the non-virtual "surprise" (base runs) |
| `slicing_demo.cpp` | Object slicing: by-value copy & pass-by-value lose the derived part (base runs); reference/pointer keep it (derived runs) |
| `pimpl_without.cpp` | The "normal" class — private members exposed in the header + the recompilation-cascade problem (commented) |
| `pimpl_with.cpp` | The pimpl fix — header shows only `class Impl;` + `unique_ptr`; all details hidden in the .cpp part |
