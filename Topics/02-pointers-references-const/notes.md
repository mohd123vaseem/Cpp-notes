# Topic 2 — Pointers, References & const Correctness

Tier 1 / #2 — an "asked everywhere" core topic. It's about **how C++ lets you refer to and access data indirectly**, and how `const` lets you lock things down so the compiler catches mistakes.

## Why this topic matters

Everything in C++ that isn't a plain value involves pointers or references under the hood — arrays, strings, function arguments, polymorphism (Topic 5), smart pointers (Topic 3). And `const` correctness is what separates people who "write C++" from people who write *safe, professional* C++. Interviewers use these to gauge exactly that.

---

## Progress tracker (sub-topics)

| # | Sub-topic | The classic question | Status |
|---|-----------|----------------------|--------|
| 1 | **Pointers** — what they are, `*` and `&`, `nullptr` | "What is a pointer?" | ✅ Done |
| 2 | **References** — aliases, how they differ from pointers | "Pointer vs reference — when each?" | ✅ Done |
| 3 | **`nullptr` vs `NULL` vs `0`** | "Why `nullptr` over `NULL`?" | ✅ Done |
| 4 | ⭐ **const correctness** — `const int*` vs `int* const` vs `const int* const` | "Difference between the three?" | ✅ Done |
| 5 | **const member functions**, const params/returns, `mutable` | "What does a const member function guarantee?" | ✅ Done |

> The ⭐ one (#4 — reading `const` right-to-left) is one of the most common C++ interview questions.

---

## Sub-topic 1 — Pointers

### What a pointer actually is
Every variable lives at some **address** in memory. A **pointer is a variable that stores an address** — instead of holding a value directly, it holds "where to find" a value.

```cpp
int x = 42;      // x holds 42, lives at some address, say 0x7ffe1234
int* p = &x;     // p holds that ADDRESS → p "points to" x
```
```
   p                         x
 ┌──────────────┐          ┌──────┐
 │ 0x7ffe1234  ─┼────────▶ │  42  │
 └──────────────┘          └──────┘
 (holds an address)     (holds the value)
```

### The two key operators: `&` and `*` (inverses)
- **`&x` = "address of x"** — makes a pointer.
- **`*p` = "value at the address in p"** — follows the pointer (dereferencing).

```cpp
int x = 42;
int* p = &x;      // & : address of x
std::cout << p;   // the ADDRESS (e.g. 0x7ffe1234)
std::cout << *p;  // 42 — dereference: go to that address, read the value
*p = 99;          // dereference on the LEFT: writes THROUGH the pointer
std::cout << x;   // 99 — we changed x without naming it
```
The superpower: through a pointer you can **read and modify** the original variable indirectly.

### Reading the declaration
`int* p;` → "`p` is a pointer to an `int`." The `*` in a *declaration* means "pointer to"; the `*` in an *expression* means "dereference." Same symbol, two roles — context decides.

### `nullptr` — a pointer to nothing
```cpp
int* p = nullptr;   // points to nothing (well-defined "empty")
if (p != nullptr) { std::cout << *p; }   // always check before dereferencing
```
⚠️ Dereferencing a **null** pointer is a crash (UB, usually segfault). An **uninitialized** pointer is worse — random garbage address:
```cpp
int* bad;             // ⚠️ garbage address — UB to dereference
int* good = nullptr;  // ✅ safe, well-defined "empty"
```
Rule: **always initialize pointers** — to a real address or to `nullptr`.

### Why pointers matter
- **Indirect access** — modify variables owned elsewhere (function args, references).
- **The heap** — `new` returns a pointer; that's how you reach heap memory.
- **Cheap passing** — a pointer is tiny (8 bytes on 64-bit); avoids copying huge objects.
- **Polymorphism** — virtual dispatch works through base-class pointers (Topic 5).

### Summary
- A pointer **stores an address** ("where"), not the value.
- `&x` = address of x. `*p` = value p points to (dereference).
- `*p = ...` writes *through* the pointer to the original variable.
- `nullptr` = "points to nothing"; **never dereference null or uninitialized** pointers.

---

## Sub-topic 2 — References

### What a reference is
A **reference is an alias** — another name for an existing variable. Not a copy, not an address you manage — just a second label on the same box.

```cpp
int x = 42;
int& r = x;      // r is a reference to x — an alias
r = 99;          // changes x, because r IS x
std::cout << x;  // 99
```
There's no separate object for `r`. `x` and `r` are two names for the exact same memory.
```
   x / r  (same box, two names)
 ┌──────┐
 │  99  │
 └──────┘
```

### The key difference from pointers
A pointer is a *separate object* holding an address; a reference is *just another name*. That single difference drives everything else:
```cpp
int a = 1, b = 2;

int* p = &a;   // pointer: needs &, is its own object
*p = 10;       // must dereference to touch the value
p = &b;        // can be re-pointed to something else

int& r = a;    // reference: binds directly, no & needed at use
r = 10;        // no dereference — use it like the variable itself
// r = b;      // does NOT rebind! this assigns b's VALUE into a
```

### 💡 What "object" means in C++ (and who has an address)

> **In C++, an "object" is NOT "an instance of a class."** An object is simply **a region of storage that holds a value.** An `int`, a `double`, a pointer, *and* a class instance are all objects. It just means "a thing that occupies memory and has a value."

So:
- **A pointer *is* its own object** — a real variable with its own storage that holds a value (an address). Not a class — just storage.
- **A reference is NOT its own object** — it's only an **alias** (a second name), with no storage of its own.

This shows up when you take addresses:
```cpp
int  x = 42;
int* p = &x;      // p is a real object with its own memory
int& r = x;       // r is just another name for x

&x;   // 0x1000  — address of x
&p;   // 0x2000  — p's OWN address (p lives somewhere too, distinct!)
 p;   // 0x1000  — the value in p (x's address)
&r;   // 0x1000  — NOT r's own address; it forwards to x's address
```
```
 Address   Name   Contents
 ────────────────────────────────
 0x1000    x      42          ← r is ALSO just "0x1000" (a name for x, no row of its own)
 0x2000    p      0x1000      ← p has its OWN row; its value is x's address
```

| | Has its own address? | What it is |
|---|---|---|
| `int x` | Yes | object holding a value |
| `int* p` | **Yes** (`&p` is real & distinct) | object holding *an address* |
| `int& r` | **No** (`&r` gives the referent's address) | just a **name/alias**, no storage |

*(Caveat: compilers often implement a reference with a hidden pointer, so it may use storage in practice — but at the language level a reference has no address you can observe.)*

### The three rules that define references

**1. A reference must be initialized when declared.** No "empty reference."
```cpp
int& r;          // ❌ compile error — must bind to something now
int& r = x;      // ✅
```

**2. A reference can never be re-seated.** Once `r` refers to `x`, it refers to `x` forever. Assigning to `r` changes `x`'s *value*, it doesn't make `r` point elsewhere.
```cpp
int& r = a;
r = b;           // assigns b's value into a; r still refers to a
```

**3. A reference can't be null.** It must alias a real object.
```cpp
int* p = nullptr;   // ✅ pointers CAN be null
int& r = *p;        // ⚠️ no such thing as a "null reference" — this is UB
                    //    (you can only bind a reference to a real object)
int& r2;            // ❌ can't even declare one without a target
```
(You *can* create a dangling reference by outliving the object, but you can't make one that points to "nothing.")

### Side-by-side
| | **Pointer** | **Reference** |
|---|---|---|
| Is it its own object? | Yes (holds an address) | No (just an alias) |
| Can be null? | Yes (`nullptr`) | No — must bind to a real object |
| Must init at declaration? | No | **Yes** |
| Can be reassigned to another target? | Yes | **No** — bound for life |
| Syntax to use the value | `*p` (dereference) | `r` (use directly) |
| Pointer arithmetic? | Yes (`p++`) | No |

### "Pointer vs reference — when each?" (interview answer)
**Use a reference when:** the thing always exists and you'll never rebind or need "nothing" — especially **function parameters** (pass `const T&` to avoid copies, or `T&` to modify the caller's variable). Cleaner syntax.

**Use a pointer when:** the thing might be **absent** (`nullptr`), you need to **rebind**, you need **pointer arithmetic**, or you're handling **arrays/heap ownership**.

> **One-liner:** *Reference = a guaranteed, non-null, fixed alias with clean syntax. Pointer = a rebindable, nullable handle with more power and more responsibility.*

### Follow-up: "Is a reference just a const pointer under the hood?"
Often yes — compilers usually implement references as auto-dereferenced pointers that can't be reassigned. But at the *language* level they differ: a reference has no address of its own, can't be null, can't be re-seated. Treat it as an alias, not "a pointer with sugar."

### Summary
- A reference is an **alias** — a second name for an existing object.
- Must be **initialized**, can **never rebind**, can **never be null**.
- Use it like the variable itself (no dereferencing).
- Default choice for **function parameters** (esp. `const T&`).
- Pick a **pointer** when you need null, rebinding, arithmetic, or ownership.

---

## Sub-topic 3 — `nullptr` vs `NULL` vs `0`

All three represent "a pointer to nothing," but only `nullptr` is type-safe. *"Why `nullptr` over `NULL`?"* is a common question.

### What each one is
- **`0`** — literally the integer zero. C++ lets you assign `0` to a pointer to mean null, but `0` is fundamentally an **`int`**.
- **`NULL`** — an old C macro, `#define NULL 0`. So it's just the integer `0` in disguise; it carries no pointer type.
- **`nullptr`** (C++11) — a real **keyword** of type `std::nullptr_t`. Means "null pointer" and only converts to pointer types, never to an integer. The modern, correct choice.

```cpp
int* p = 0;         // works — 0 is a special case that becomes a null pointer
int* p = NULL;      // works, but NULL is really just 0
int* p = nullptr;   // ✅ genuinely a null pointer, type-safe
```

### Why `0`/`NULL` are problematic: overload ambiguity
```cpp
void f(int n);      // overload A — integer
void f(int* p);     // overload B — pointer

f(0);        // ⚠️ calls f(int)!  0 is an int
f(NULL);     // ⚠️ NON-PORTABLE — see below
f(nullptr);  // ✅ calls f(int*)  — nullptr has pointer type (everywhere)
```
You meant the pointer version, but `0` is an integer so the compiler picks `f(int)`. `nullptr` resolves correctly.

> 📖 **"Portable"** = code that behaves the **same no matter where you compile or run it** — different compilers (GCC, Clang, MSVC), OSes (Windows, Linux, Mac), or machines. *Non-portable* = behavior changes depending on the compiler/platform (bad).
>
> **`f(NULL)` isn't even portable** — because `NULL` is defined differently per compiler:
> - **MSVC** (`#define NULL 0`): `NULL` is `int` `0` → silently calls `f(int)` (wrong overload).
> - **GCC/Clang** (`#define NULL __null`): `__null` converts to *both* `int` and pointer → **ambiguous → compile error**.
>
> So `NULL` either picks the wrong overload *or* fails to compile, depending on the compiler. `nullptr` behaves correctly and identically everywhere. (GCC also warns `-Wconversion-null` on `int n = NULL;` for the same reason.)

### Other reasons `nullptr` wins
- **Type safety** — won't silently convert to `int`:
  ```cpp
  int n = nullptr;   // ❌ compile error (good!)
  int n = NULL;      // compiles (NULL is 0) — bug slips through
  ```
- **Readability** — `nullptr` clearly says "null *pointer*."
- **Templates** — deduction gets `NULL` wrong (deduces `int`); `nullptr` deduces `std::nullptr_t`.

### Summary
| | What it is | Type | Problem |
|---|---|---|---|
| `0` | integer literal | `int` | ambiguous — number or pointer? |
| `NULL` | macro `#define NULL 0` | `int` | same as `0`, disguised |
| `nullptr` | keyword (C++11) | `std::nullptr_t` | **none — use this** |

> **Rule: always use `nullptr` in modern C++.** `0`/`NULL` are integers → overload ambiguity + silent int conversions.

---

## Sub-topic 4 — const Correctness ⭐

> ## `const` APPLIES TO WHATEVER IS ON ITS IMMEDIATE **LEFT**. IF THERE'S NOTHING ON ITS LEFT, IT APPLIES TO WHAT'S ON ITS **RIGHT**.

*(This is THE rule for the whole sub-topic — everything below follows from it. A friendly shortcut is to "read the declaration right-to-left".)*

One of the most-asked C++ interview questions. Read the declaration **right-to-left**, translating `*` → "pointer to" and `const` → "constant / read-only".

Two independent things could be const:
1. **The thing pointed to** (the value) — can I change it?
2. **The pointer itself** — can I repoint it?

### Case 1: `const int* p` — pointer to a const int
Right-to-left: "`p` is a **pointer to** an **int** that is **const**." → **value read-only, pointer can move.**
```cpp
int a = 1, b = 2;
const int* p = &a;
*p = 5;      // ❌ ERROR — can't change the value through p
p = &b;      // ✅ OK — can repoint
```

### Case 2: `int* const p` — const pointer to an int
Right-to-left: "`p` is a **const** **pointer to** an **int**." → **pointer locked, value editable.**
```cpp
int a = 1, b = 2;
int* const p = &a;   // must initialize now (can't reassign later)
*p = 5;      // ✅ OK — value changes (a becomes 5)
p = &b;      // ❌ ERROR — can't repoint
```

### Case 3: `const int* const p` — const pointer to a const int
Right-to-left: "`p` is a **const** **pointer to** an **int** that is **const**." → **both locked.**
```cpp
const int* const p = &a;
*p = 5;      // ❌ ERROR — value is const
p = &b;      // ❌ ERROR — pointer is const
```

> ### ⚠️ IMPORTANT — const-ness belongs to the pointer's *type*, not its target
>
> With `const int* p`, the read-only-ness is part of **`p`'s type**, not tied to whatever it currently points at. `p` is a "pointer to const int" **forever** — repointing it doesn't change that.
>
> ```cpp
> int a = 10, b = 5;
> int const* p = &a;
>
> *p = 99;   // ❌ can't write through p
> p = &b;    // ✅ repointed to b
> *p = 42;   // ❌ STILL can't write through p — even though it now points to b!
>
> b = 42;    // ✅ but b itself isn't const, so change it directly
> ```
>
> No matter what `p` points to, writing through it (`*p = ...`) is **always** forbidden — `p` only ever gives a **read-only view** of whatever it's aimed at. Think of `const int*` as **read-only glasses**: you can look at any int (repoint freely), but you can never *edit* what you're looking at; the object itself stays editable by its own name.

### The trick to never mix them up
- **`const` LEFT of the `*`** → the **data** is const (can't change `*p`).
- **`const` RIGHT of the `*`** → the **pointer** is const (can't change `p`).
```
const int*  →  const left of *   →  data locked
int* const  →  const right of *  →  pointer locked
```

### Why const correctness matters
- **A promise the compiler enforces.** `const int* p` = "I'll only read, never write" — violating it is a compile error, not a runtime bug.
- **The everyday use: `const T&` parameters.** `void print(const std::string& s);` passes read-only data cheaply *and* guarantees no mutation.
- **It's contagious (good):** a `const` object can only call `const` members / pass to `const` params. Get it right early; bolting it on later is painful.

### Summary
| Declaration | Read as | Value editable? | Pointer movable? |
|---|---|---|---|
| `int* p` | pointer to int | ✅ yes | ✅ yes |
| `const int* p` | pointer to const int | ❌ no | ✅ yes |
| `int* const p` | const pointer to int | ✅ yes | ❌ no |
| `const int* const p` | const pointer to const int | ❌ no | ❌ no |

- Read **right-to-left**. `const` **left of `*`** → data locked; `const` **right of `*`** → pointer locked.
- `const T&` params = pass cheaply + promise not to modify (the everyday use).

> **Note:** `const int* p` and `int const* p` mean the **exact same thing** (const before or after the *type* is identical). Only the `const`'s position *relative to the `*`* matters.

### The precise rule (better than "right-to-left")

> ## `const` APPLIES TO WHATEVER IS ON ITS IMMEDIATE **LEFT**. IF THERE'S NOTHING ON ITS LEFT, IT APPLIES TO WHAT'S ON ITS **RIGHT**.

```cpp
const int* p;
//  ^ const has NOTHING on its left → applies to the RIGHT: 'int'
//  → const binds to int → "pointer to const int"

int const* p;
//      ^ const has 'int' on its left → applies to 'int'
//  → const binds to int → "pointer to const int"
```
In both, `const` attaches to `int`; the `*` isn't involved → they're the **same**. ✅

**The distinction that actually feels "wrong":** don't confuse `int const*` with `int* const` — the `*` position separates them:
```cpp
int const* p;    // const BEFORE * → pointer to const int  (DATA locked)
int* const p;    // const AFTER  * → const pointer to int  (POINTER locked)
```

| Declaration | `const` vs `*` | Meaning |
|---|---|---|
| `const int* p` | before `*` | pointer to const int (data locked) |
| `int const* p` | before `*` | **same** — pointer to const int (data locked) |
| `int* const p` | **after** `*` | const pointer to int (pointer locked) |

So `const int*` and `int const*` are **twins** (const on the same side of `*` — the left); `int* const` is the **odd one out** (const jumped to the right of `*`).

- Moving `const` **around the type** (`const int` ↔ `int const`) → **no change**.
- Moving `const` **across the `*`** (`int const*` ↔ `int* const`) → **completely different meaning**.

---

## Sub-topic 5 — const Member Functions, const Params/Returns & `mutable`

Where `const` moves from pointers to **classes** — the everyday, professional use.

### Part 1 — const member functions
A **const member function** promises *"calling me won't modify the object."* Mark it with `const` **after** the parameter list:
```cpp
class Rectangle {
    int width_, height_;
public:
    int area() const { return width_ * height_; }  // const: only reads
    void setWidth(int w) { width_ = w; }            // NOT const: modifies
};
```
The compiler **enforces** it — inside a const member function, **every member variable is treated as read-only** (except `mutable` ones):
```cpp
class Rectangle {
    int width_, height_;
public:
    void shrink() const {   // const method
        width_ = 0;         // ❌ error — can't modify width_
        height_ = 0;        // ❌ error — can't modify height_ either
    }
};
```
So *"const makes the object const"* means: for the duration of that call, the **whole object is frozen** — `width_`, `height_`, all of them become read-only. Not one member — every member at once. (Only `mutable` members stay changeable — the escape hatch.) Think of it as: `const` after the function puts **read-only glasses on the entire object** while that function runs.

**Why it matters — the calling rule (asymmetric):**

| Object type | Can it call a **const** method? | Can it call a **non-const** method? |
|---|---|---|
| **non-const** object (`Rectangle r;`) | ✅ YES | ✅ YES |
| **const** object (`const Rectangle r;`) | ✅ YES | ❌ NO |

Read the table, not a sentence. Two plain facts:
1. A **const method** can be called by **any** object (const or non-const). It never requires the object to be const.
2. A **const object** is the restricted one: it can call const methods, but NOT non-const methods.

```cpp
Rectangle r;              // non-const object
r.area();                 // ✅ const method on non-const object — fine
r.setWidth(5);            // ✅ non-const method on non-const object — fine

const Rectangle cr;       // const object
cr.area();                // ✅ const method on const object — fine
cr.setWidth(5);           // ❌ ERROR — non-const method on a const object
```

The only illegal combo is the ❌ one: **non-const method called on a const object.** Everything else is allowed.

Since `const T&` params are everywhere (they make the object const inside the function), any read-only method **must** be marked `const` — otherwise it can't be called when someone holds a const object. *That* is const correctness in practice.

**const overloading** (how `vector::operator[]` works):
```cpp
char& at(int i)       { return data_[i]; }  // non-const: writable
char  at(int i) const { return data_[i]; }  // const: read-only
```

### Part 2 — const params and const returns
- **const ref parameter — the everyday one:** `void print(const std::string& s);` → pass by reference (no copy) **and** promise not to modify. Use for any non-trivial read-only object.
- **const by value** (`const int n`) is legal but rarely useful (only stops reassigning the local copy).
- **const return (reference/pointer):** matters because returning a **reference** hands the caller an *alias to your private member*, not a copy — so they could write straight into your internals. `const` makes that alias read-only:
  ```cpp
  std::string&       name()       { return name_; }  // ⚠️ caller gets an alias to name_
  p.name() = "hacked";                               //    → this MODIFIES name_! encapsulation broken

  const std::string& name() const { return name_; }  // ✅ read-only alias (no copy)
  p.name() = "hacked";                               //    ❌ compile error — name_ protected
  ```
  Returning `const` **by value** (`std::string name()`) is pointless — the caller gets their own copy to do as they like, disconnected from `name_`.

  | Return type | Caller gets | Can modify your member? |
  |---|---|---|
  | `std::string& name()` | alias to `name_` | ❌ **Yes — dangerous** |
  | `const std::string& name() const` | read-only alias (no copy) | No — safe |
  | `std::string name() const` | a copy | Irrelevant → `const` here is pointless |

### Part 3 — `mutable` (the escape hatch)
Lets a specific member be modified **even inside a const method** — for internal bookkeeping (cache, counter, mutex) that isn't part of the object's logical state:
```cpp
class Database {
    mutable int queryCount_ = 0;   // can change even in const methods
public:
    std::string lookup(const std::string& key) const {  // logically const
        ++queryCount_;             // ✅ allowed — queryCount_ is mutable
        return doLookup(key);
    }
};
```

**Logical vs bitwise const** (the concept behind `mutable`):
- **Bitwise const** — not a single bit changes.
- **Logical const** — the object's *observable meaning* doesn't change, even if a hidden internal does.

A const member function guarantees **logical** const, not bitwise. `mutable` marks a member as "implementation detail, not logical state." Common uses: caching, thread-safety `mutex`es, stats counters. ⚠️ Don't use it for anything that changes the object's real meaning.

### ⚠️ const is SHALLOW
In a const method, a pointer member `int* p_` becomes `int* const` (the **pointer** is const) — **not** `const int*`. So you can still do `*p_ = 5`. const protects the pointer, not what it points to.

### Interview-depth follow-ups
- *"Bitwise or logical const?"* → **Logical.** `mutable` members can change; pointer members' pointees stay writable (shallow).
- *"Why `mutable` for a `std::mutex` in a const method?"* → Locking modifies the mutex, but a read op should stay `const`; `mutable` allows it.

### Summary
| Feature | Syntax | Meaning |
|---|---|---|
| const member function | `int area() const` | "won't modify the object" (enforced) |
| const ref parameter | `void f(const T& x)` | pass without copying + won't modify — the everyday one |
| const return (ref) | `const T& get() const` | caller can read, not modify internals |
| `mutable` member | `mutable int n_;` | can change even in const methods (caches, counters, mutexes) |

- Mark **every** non-modifying method `const` — or it can't be called on const objects.
- const method = **logical** const, not bitwise; `mutable` is the escape hatch.
- const is **shallow**: a pointer member's pointee is still writable in a const method.

---

## Code examples in this folder

| File | Demonstrates |
|------|--------------|
| `references.cpp` | Alias behavior, the 3 reference rules (esp. rule 3: can't be null), pass-by-reference vs pass-by-pointer |
| `nullptr_vs_null.cpp` | Overload resolution: `NULL`/`0` pick `f(int)`, `nullptr` picks `f(int*)`; the `int n = NULL` trap |
| `const_pointers.cpp` | The 3 const-pointer combos + the key insight (const is part of the pointer's type, not its target) |
