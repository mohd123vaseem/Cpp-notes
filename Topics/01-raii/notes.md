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
