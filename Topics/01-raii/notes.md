# Topic 1 — RAII & the Memory Model

To understand RAII, you first need two building blocks: **where your variables live** and **when they die**. Let's go step by step.

---

## Part A — Stack vs Heap (where things live)

When your program runs, it stores data in two main places:

### 🟢 The Stack
Think of it like a **stack of plates**. Every time you call a function, a new "plate" (called a *stack frame*) is placed on top. All the normal local variables you declare live on that plate.

```cpp
void foo() {
    int x = 5;        // x lives on the stack
    double y = 3.14;  // y lives on the stack
}                     // <-- plate removed, x and y vanish AUTOMATICALLY
```

Key traits:
- **Automatic cleanup** — when the function ends, the plate is removed and everything on it is destroyed. You do nothing.
- **Very fast** — allocating is just moving a pointer.
- **Limited size** — typically ~1–8 MB. Too much → *stack overflow*.
- **LIFO** — Last In, First Out (last plate added is first removed).

### 🔴 The Heap
Think of it like **renting a storage unit**. You explicitly ask for memory with `new`, and you get back the *address* of it. It stays rented until you explicitly `delete` it.

```cpp
void foo() {
    int* p = new int(5);  // the int(5) lives on the HEAP
                          // p itself (the pointer) lives on the stack
    delete p;             // YOU must return the memory, or it leaks
}
```

Key traits:
- **Manual cleanup** — *you* must `delete` it. Forget → **memory leak**.
- **Slower** — finding free space takes work.
- **Large** — limited by your RAM, not a few MB.
- **Flexible lifetime** — survives beyond the function that created it (until you delete it).

> **The one-line summary:** Stack = automatic & fast but short-lived and small. Heap = manual & slower but big and long-lived.

---

## Part B — Object Lifetime (when things die)

A stack object's life has a clear beginning and end:

```cpp
{                      // scope begins
    MyClass obj;       // CONSTRUCTOR runs here  → obj is "born"
    obj.doStuff();
}                      // DESTRUCTOR runs here   → obj "dies", automatically
```

Two special member functions bookend every object's life:
- **Constructor** — runs when the object is created (set things up / acquire resources).
- **Destructor** (`~MyClass`) — runs when the object is destroyed (clean up / release resources).

The magic: for stack objects, **the destructor is called automatically** the moment the object goes out of scope. You don't call it. The compiler guarantees it.

---

## Part C — RAII (the idea that ties it together)

The problem RAII solves: **manual cleanup is error-prone.** Look at this:

```cpp
void process() {
    int* data = new int[1000];   // acquire
    if (somethingBadHappens())
        return;                  // 💥 LEAK! we returned without delete
    delete[] data;               // only runs if we reach here
}
```

If we `return` early — or an **exception** is thrown — we skip the `delete`. Leak.

**RAII's solution:** wrap the resource inside an object. Acquire it in the **constructor**, release it in the **destructor**. Now cleanup is tied to the object's lifetime — and since stack objects *always* get their destructor called, cleanup *always* happens.

```cpp
void process() {
    std::vector<int> data(1000);  // constructor acquires memory
    if (somethingBadHappens())
        return;                   // ✅ vector's destructor frees it
}                                 // ✅ or here. Either way, no leak.
```

### Why is the vector freed even on an early return / exception?

The vector object `data` lives on the **stack** (it's just a few pointers); the 1000 ints live on the **heap**, and `data` holds their address. The rule: **a stack object's destructor runs automatically on every path that leaves the scope** — reaching `}`, a `return`, *or* an exception.

- **Normal end / early `return`:** before leaving, the compiler calls `~vector()`, which `delete[]`s the heap memory.
- **Exception:** throwing triggers **stack unwinding** — C++ walks out of the function toward a `catch`, destroying every live local on the way. That unwinding *itself* calls `~vector()`, so the heap is freed even though `return`/`}` was never reached.

A raw `int* data = new int[1000]` leaks here because a pointer has **no destructor** — when the stack frame vanishes the address is lost but the heap memory is never released. The vector survives because its destructor *is* the cleanup, and destructors can't be skipped.

`std::vector`, `std::string`, `std::unique_ptr`, `std::lock_guard` — they're *all* RAII wrappers. That's why modern C++ rarely needs raw `new`/`delete`.

**RAII in one sentence:**
> *Resource Acquisition Is Initialization* — bind a resource to an object so that acquiring it = creating the object, and releasing it = destroying the object. Because destructors run automatically (even during exceptions), your cleanup is guaranteed.

---

## Part D — Why two regions (stack vs heap)? Why not one?

Every value has two questions: **how big?** (known at compile time, or only at runtime?) and **how long must it live?** (dies with the function, or outlives it?). Stack and heap are two answers, each tuned for one case.

- **Stack** — allocation is just moving one pointer (≈1 CPU instruction), freed automatically in strict LIFO order. Price: size must be known at compile time, lifetime is tied to scope, and it's small (~MBs).
- **Heap** — a managed pool you allocate from at runtime in any order, kept alive as long as you want. Price: the allocator must search/track/handle fragmentation, *you* must free it, and it's much slower + cache-scattered.

**Why not just one?**
- *Heap-only:* every loop counter and temporary would pay allocator cost → orders of magnitude slower, manual cleanup everywhere.
- *Stack-only:* impossible to size data at runtime (user input, files), return data that outlives a function (lists, trees), or share/co-own objects.

Real programs need **both**: fast scope-bound locals (stack) **and** dynamic, long-lived, shared data (heap). RAII bridges them — a stack object owns heap memory and frees it in its destructor: heap's flexibility + stack's automatic cleanup.

| | **Stack** | **Heap** |
|---|---|---|
| Allocation cost | ~1 instruction (move pointer) | Allocator search + bookkeeping |
| Size known | At compile time | Can be decided at runtime |
| Lifetime | Tied to scope (auto-freed) | Whatever you want (manual / RAII) |
| Order of freeing | Strict LIFO | Any order |
| Cleanup | Automatic | You must free it |
| Typical size | Small (~MBs) | Large (most of RAM) |
| Speed | Very fast, cache-hot | Slower, scattered |
| Physically lives in | **RAM** (always) | **RAM** (always) |
| Cache behavior | Small & reused → copies usually stay cache-hot | Scattered → more cache misses |

> **Note:** Both stack and heap live in RAM, in the *same* address space — that's why they can collide. The CPU cache is **not** a separate place data lives; it's a fast scratchpad holding temporary *copies* of recently-used RAM. The stack just *tends* to stay cached because it's small and reused. Cache = speed, not location.

---

## Mental model to keep

- **Acquire in constructor → release in destructor → cleanup is automatic & exception-safe.**
- Stack = automatic/fast/small/short-lived. Heap = manual/slower/big/long-lived.
- The key trick that makes RAII work: **destructors of stack objects always run**, on every exit path including exceptions.

---

## Part E — `new` / `delete` (manual heap)

The raw, manual way to use the heap — what RAII wrappers do for you internally. Know it to understand the machinery and read old code; don't write it daily.

```cpp
int* p = new int(42);   // allocate 1 int on heap, return its address
delete p;               // free it

int* arr = new int[5];  // allocate array of 5 ints
delete[] arr;           // note the [] — frees the whole array
```

- `new` = **allocate** heap memory + **construct** the object, returns a pointer.
- `delete` = **run destructor** (for classes) + **free** the memory.
- **Forms must match:** `new` ↔ `delete`, `new[]` ↔ `delete[]`. Mismatch = undefined behavior.
- **Every `new` needs exactly one matching `delete`.** None → leak. Twice → double-free crash.
- For a class, forgetting `delete` means the **destructor never runs** → resource cleanup silently skipped.
- After `delete`, the pointer **dangles** (still holds the freed address). Set `p = nullptr;` to be safe. `delete nullptr;` is a no-op.

| Allocate | Free | For |
|----------|------|-----|
| `new T` | `delete p` | single object |
| `new T[n]` | `delete[] p` | array |

> Modern advice: **prefer RAII wrappers** (`std::vector`, `std::string`, `std::make_unique`) over raw `new`/`delete` — they guarantee the matching `delete` even on early return / exception.

---

## Part F — The Four Classic Memory Bugs

These happen when you manage heap memory by hand — exactly what RAII exists to prevent. *"What does RAII protect you from?"* → these four are the answer.

### 1. Memory Leak
**What:** You `new` something but never `delete` it — you lose the only pointer to it.
```cpp
void leak() {
    int* p = new int(42);
    // ... forgot delete p;
}   // p (the pointer) dies; the heap int lives on, unreachable → leaked
```
**Why bad:** Memory usage keeps growing. In a long-running app (server, browser tab) leaks pile up until you run out of memory. Silent and slow.

### 2. Dangling Pointer
**What:** A pointer that still holds an address, but the target is gone (freed or out of scope).
```cpp
int* p = new int(42);
delete p;   // memory freed, but p STILL holds the old address → dangling
```
**Why bad:** Looks valid, points at memory you no longer own. Using it causes bug #3 or #4. **Fix:** `p = nullptr;` right after `delete`.

### 3. Use-After-Free
**What:** You read or write through a dangling pointer.
```cpp
int* p = new int(42);
delete p;
std::cout << *p;   // ❌ read freed memory
*p = 99;           // ❌ write freed memory
```
**Why bad:** Undefined behavior. The freed memory may already belong to something else → read garbage or corrupt unrelated data. One of the **most exploited security vulnerabilities** (big in browser/Chromium work). Often crashes far from the real cause.

### 4. Double-Free
**What:** You `delete` the same pointer twice.
```cpp
int* p = new int(42);
delete p;   // fine
delete p;   // ❌ double-free
```
**Why bad:** Undefined behavior, usually a crash. Corrupts the allocator's bookkeeping; also a security hole. Common when two pieces of code both think they own the pointer.

### How they relate
```
        delete p;
           │
           ├─ then use p          → use-after-free (#3)
           ├─ then delete p again → double-free (#4)
           └─ leave p as-is       → dangling pointer (#2, the loaded gun)
```
A **leak** = forgetting to free; the other three = freeing then mishandling.

### How RAII kills all four
```cpp
auto p = std::make_unique<int>(42);
// no delete        → can't leak (#1)
// freed once, auto → no double-free (#4)
// clear ownership  → no dangling / use-after-free (#2, #3)
```
**Punchline:** don't manage memory by hand → these four become impossible.

| Bug | Cause | One-line fix |
|-----|-------|--------------|
| Memory leak | `new` without `delete` | RAII / always pair them |
| Dangling pointer | pointer outlives its target | `nullptr` after delete; don't return locals' addresses |
| Use-after-free | using a dangling pointer | don't touch freed memory; RAII |
| Double-free | `delete` twice | clear ownership; RAII |

---

## Part G — Construction / Destruction Order

A near-guaranteed interview question: *"For a class with a base class and members, in what order do constructors and destructors run?"* The rules are simple and 100% predictable.

### The three rules of construction
When you create an object, things are built in this order:

1. **Base class first** — before a derived object exists, its base part must be fully built.
2. **Then member variables** — built in the **order they're declared** in the class (NOT the order you list them in the constructor's initializer list).
3. **Then the constructor body** `{ ... }` runs.

```cpp
class Engine {
public:  Engine()  { std::cout << "Engine built\n"; }
};
class Wheels {
public:  Wheels() { std::cout << "Wheels built\n"; }
};

class Vehicle {                          // base class
public:  Vehicle() { std::cout << "Vehicle (base) built\n"; }
};

class Car : public Vehicle {             // derived from Vehicle
    Engine engine_;                      // member 1 (declared first)
    Wheels wheels_;                      // member 2 (declared second)
public:
    Car() { std::cout << "Car body runs\n"; }
};
```

Creating a `Car` prints:
```
Vehicle (base) built     ← 1. base first
Engine built             ← 2. members, in declaration order
Wheels built
Car body runs            ← 3. constructor body last
```

### Destruction is the exact reverse
When the object dies, everything unwinds in **mirror image**:

1. **Derived destructor body** runs first.
2. **Then members**, in **reverse declaration order**.
3. **Then the base class** destructor last.

```
~Car body runs           ← derived first
Wheels destroyed         ← members, reverse order
Engine destroyed
~Vehicle (base)          ← base last
```

> **The pattern to remember:** construction goes **outside-in / top-down** (base → members → body); destruction goes **inside-out / bottom-up** (body → members → base). Destruction is always the precise reverse of construction. It's like nested boxes: you build the outer box, put things in top-to-bottom; to unpack, you take them out bottom-to-top, then discard the box.

### Two gotchas interviewers probe

**Gotcha 1 — declaration order wins, not initializer-list order.** Members are *always* constructed in the order they're declared in the class, even if your constructor's initializer list lists them differently:

```cpp
class X {
    int a_;
    int b_;
public:
    X() : b_(1), a_(b_) {}   // ⚠️ looks like b_ first, but a_ is declared first!
                            // a_ is built FIRST using b_ which isn't set yet → bug
};
```
This is why compilers warn (`-Wreorder`) and why you should write the initializer list in declaration order.

**Gotcha 2 — why base destructors and `virtual` matter.** If you `delete` a derived object through a base pointer and the base destructor *isn't* `virtual`, only the base destructor runs — the derived part leaks. (That's a Topic-5/polymorphism detail, but it's rooted in this destruction-order rule. We'll hit it properly there.)

### Why this order exists (the logic)
It's not arbitrary — it's **dependency order**:
- The **derived** class may *use* its base and members in its constructor, so those must exist first → base and members built before derived body.
- During destruction, the derived part may still *use* its base/members while cleaning up, so the derived body runs first, and the base/members are torn down only after nobody needs them.

In plain words: **build the foundation (base) first, then build on top of it; demolish from the top (derived) down, foundation last.** Like a house — you can't put a roof on thin air, and you can't pull the foundation out from under a standing house. Build the thing others depend on first; destroy the thing that depends on others first.

### Quick summary
| Phase | Order |
|-------|-------|
| **Construction** | base class → members (declaration order) → constructor body |
| **Destruction** | destructor body → members (reverse order) → base class |

- Destruction is *always* the exact reverse of construction.
- Members follow **declaration order**, not initializer-list order.

---

## ⭐ Interview Q&A

**Q: What is RAII and why does C++ rely on it?**
> RAII ties a resource's lifetime to an object's lifetime — acquire in the constructor, release in the destructor. Since C++ has no garbage collector, it relies on deterministic destructors to free resources automatically. Because a stack object's destructor always runs when it leaves scope (even during exception unwinding), cleanup is guaranteed and leak-free.

**Q: How do you prevent leaks without a garbage collector?**
> Use RAII wrappers instead of raw `new`/`delete`: `std::vector`, `std::string`, `std::unique_ptr`, `std::lock_guard`, etc. They free their resource in their destructor, so you never have to remember to clean up, and early returns/exceptions can't leak.

**Q: Why is RAII exception-safe?**
> During stack unwinding (when an exception propagates), every fully-constructed local object has its destructor called. So resources wrapped in RAII objects are released even though the normal code path was interrupted.

---

## Code examples in this folder

| File | Demonstrates |
|------|--------------|
| `raii.cpp` | Constructor/destructor firing on scope exit + on the exception path |
| `new_delete.cpp` | Manual heap: `new`/`delete`, `new[]`/`delete[]`, destructor on delete, dangling cleanup |
| `memory_bugs.cpp` | The 4 bugs (leak, dangling, use-after-free, double-free) + their fixes; sanitizer notes |
| `construction_order.cpp` | Base → members (decl order) → body on construction; exact reverse on destruction |

---

## Progress tracker (sub-topics)

| # | Sub-topic | Status |
|---|-----------|--------|
| 1 | Stack vs heap (language + OS level) | ✅ Done |
| 2 | Object lifetime (ctor/dtor, scope) | ✅ Done |
| 3 | RAII idiom + why it's exception-safe | ✅ Done |
| 4 | `new`/`delete`, `new[]`/`delete[]` | ✅ Done |
| 5 | The 4 bugs RAII prevents: leak, dangling pointer, double-free, use-after-free | ✅ Done |
| 6 | Construction/destruction order (base→derived, members in decl order, reverse on destroy) | ✅ Done |
