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
| 4 | ⭐ **const correctness** — `const int*` vs `int* const` vs `const int* const` | "Difference between the three?" | ⬜ Pending |
| 5 | **const member functions**, const params/returns, `mutable` | "What does a const member function guarantee?" | ⬜ Pending |

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

f(NULL);     // ⚠️ calls f(int)!  NULL is just 0, an int
f(0);        // ⚠️ calls f(int)!  same problem
f(nullptr);  // ✅ calls f(int*)  — nullptr has pointer type
```
You meant the pointer version, but `NULL`/`0` are integers so the compiler picks `f(int)`. `nullptr` resolves correctly.

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

## Code examples in this folder

| File | Demonstrates |
|------|--------------|
| `references.cpp` | Alias behavior, the 3 reference rules (esp. rule 3: can't be null), pass-by-reference vs pass-by-pointer |
| `nullptr_vs_null.cpp` | Overload resolution: `NULL`/`0` pick `f(int)`, `nullptr` picks `f(int*)`; the `int n = NULL` trap |
