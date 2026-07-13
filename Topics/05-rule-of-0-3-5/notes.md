# Topic 5 — Constructors/Destructors + Rule of 0/3/5

Tier 1 #5 (~2 days). You've already **lived** most of this — hand-writing `shared_ptr` in Topic 3 meant writing a destructor, copy constructor, and copy assignment operator. **That *is* the Rule of 3.** This topic names it, formalizes it, and adds the missing pieces.

## Why this topic matters

These are the **special member functions** — the ones the compiler will silently generate for you if you don't write them. Knowing *when the compiler's version is wrong* (and therefore when you MUST write your own) is the whole skill. It's the difference between a class that manages a resource correctly and one that double-frees or leaks.

---

## Progress tracker (sub-topics)

| # | Sub-topic | The classic question | Status |
|---|-----------|----------------------|--------|
| 1 | **The special member functions** — default, parameterized, copy, move ctors; destructor | "What are the special member functions?" | ✅ Done |
| 2 | **Copy ctor vs copy assignment** (and move ctor vs move assignment) | "When is the copy ctor called vs assignment?" | ⬜ Pending |
| 3 | **Deep vs shallow copy** | "What's the danger of the compiler's default copy?" | ⬜ Pending |
| 4 | ⭐ **Rule of 0 / 3 / 5** — when you must define them | "Explain the Rule of Three/Five." | ✅ Done |
| 5 | **Member initializer lists** + construction/destruction order | "Why prefer the init list over assigning in the body?" | ⬜ Pending |
| 6 | **`explicit`** keyword | "What does `explicit` prevent?" | ⬜ Pending |
| 7 | ⭐ **Write a class that manages a raw pointer correctly** | (the practical test — you've basically done this) | ⬜ Pending |

> Note: **move** ctor/assignment get introduced here but go deep in **Topic 7 (Move Semantics)** — that's also when we finish the `shared_ptr` Rule of 5.

---

## Sub-topic 1 & 4 — The special member functions + what "Rule of 0/3/5" means

### The special member functions
The functions the **compiler writes for you automatically** if you don't:
```cpp
class MyClass {
    MyClass();                              // 1. default constructor
    ~MyClass();                             // 2. destructor
    MyClass(const MyClass& other);          // 3. copy constructor
    MyClass& operator=(const MyClass& o);   // 4. copy assignment
    MyClass(MyClass&& other);               // 5. move constructor      (C++11)
    MyClass& operator=(MyClass&& o);        // 6. move assignment       (C++11)
};
```
*(You already hand-wrote #2, #3, #4 in your `shared_ptr`.)* The rule is about **when you must write them yourself vs. let the compiler do it.**

> **The name is literally the COUNT of special member functions you have to write: 0, 3, or 5.**

### The Rule of 3
> **If you need to write any ONE of — destructor, copy constructor, copy assignment — you almost certainly need all THREE.**

**Why:** needing any one means your class **manages a resource** (raw pointer, file handle, socket). And then all three jobs matter:
- **Destructor** → release the resource.
- **Copy ctor** → what happens when someone copies it? (share? duplicate?)
- **Copy assignment** → same question, but the target already owns something.

Your `shared_ptr` is the textbook case: you needed a destructor (to `delete`), so you *also* needed a copy ctor (increment count) and copy assignment (release old + share new). Had you written only the destructor and let the compiler generate copy → **double-free**.

### The Rule of 5
C++11 added **move** operations, so the rule extends: **3 + move ctor + move assign = 5.**

**Why:** moves are an optimization — when the source is a temporary about to die, **steal** its resource instead of copying. But if you define the Rule-of-3 functions and *not* the moves, **the compiler won't generate moves for you** → your class silently **falls back to copying** everywhere a move could have happened. Still correct, just slower.

*(That's exactly where the hand-written `shared_ptr` sits now: correct but missing moves → does an atomic refcount bump where it could just steal the pointers. Fixed in Topic 7.)*

### The Rule of 0 — the modern, best answer
> **Design your class so you need to write NONE of them.** Let every member manage itself.

Don't hold raw resources. Use members that are **already RAII types** — `std::vector`, `std::string`, `std::unique_ptr`, `std::shared_ptr`. They know how to copy, move, and destroy themselves, so the compiler-generated functions just call the members' correct versions and everything works.

```cpp
// Rule of 5 — raw pointer → you owe 5 functions
class Bad  { int* data; };

// Rule of 0 — the member manages itself → you write NOTHING ✅
class Good { std::vector<int> data; };
```

**Rule of 0 is the goal.** Rules of 3/5 are the fallback for when you genuinely must manage a raw resource yourself — e.g. when *implementing* a smart pointer.

### Summary
| Rule | Means | When |
|------|-------|------|
| **Rule of 0** | write **none** | members are RAII types that manage themselves — **aim for this** |
| **Rule of 3** | write all **3** (dtor, copy ctor, copy assign) | you manage a raw resource; need one → need all three |
| **Rule of 5** | write all **5** (+ move ctor, move assign) | same, plus moves so you don't silently fall back to copying |

---

## Code examples in this folder

| File | Demonstrates |
|------|--------------|
| _(added as we go)_ | |
