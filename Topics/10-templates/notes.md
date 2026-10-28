# Topic 10 — Templates & Generics

Tier 2 #10 (~3 days). You've **already used** templates (hand-writing `SharedPtr<T>`, `my_vector<T>`), so this formalizes the mechanism and fills in the interview-relevant features. **Goal: read/reason about templated code comfortably — not metaprogramming mastery.**

## Why this topic matters

Templates are how C++ does **generic programming** — write code once that works for *any* type, with **zero runtime cost** (resolved at compile time). The entire STL is templates. Interviewers probe whether you understand *how* they work (compile-time instantiation, why they live in headers) and can read templated code.

---

## Progress tracker (sub-topics)

### Part A — Fundamentals
| # | Sub-topic | The classic question | Status |
|---|-----------|----------------------|--------|
| 1 | **Function templates** — generic functions | "What problem do templates solve?" | ✅ Done |
| 2 | **Class templates** — generic classes | "How does a class template work?" | ✅ Done |
| 3 | **Instantiation + why templates live in headers** | "Why can't you split a template into .h/.cpp?" | ✅ Done |

### Part B — Key features
| # | Sub-topic | The classic question | Status |
|---|-----------|----------------------|--------|
| 4 | **Template specialization** (full + partial) | "What is template specialization?" | ⬜ Pending |
| 5 | **Non-type params + variadic templates** | "What's a variadic template?" | ⬜ Pending |
| 6 | **`auto` & `decltype`** (type deduction) | "auto vs decltype?" | ✅ Done |
| 7 | **Concepts (C++20)** — constraining templates | "How do you constrain a template?" | ⬜ Pending |

### Part C — Advanced (lighter — read/recognize, not master)
| # | Sub-topic | The classic question | Status |
|---|-----------|----------------------|--------|
| 8 | **Type traits + SFINAE + CRTP** (overview) | "What is SFINAE / CRTP?" | ⬜ Pending |

---

## Sub-topic 1 — Function Templates

### Simple: what it is
A **function template** = a *recipe* for generating functions. Write the logic **once** with the type as a placeholder (`T`); the compiler generates a concrete version per type you use.
```cpp
template <typename T>
T maximum(T a, T b) { return (a > b) ? a : b; }

maximum(3, 7);       // → generates maximum<int>
maximum(2.5, 1.5);   // → generates maximum<double>
maximum('a', 'z');   // → generates maximum<char>
```

### The WHY
Before templates, to get `max` for many types you had two bad options:
1. **Duplicate the function per type** (`maxInt`, `maxDouble`, …) — identical logic copy-pasted → tedious, bug-prone.
2. **`void*` / macros** (C hacks) — lose type safety, cryptic errors.

Templates: **write the logic once, compiler stamps out a type-specific version on demand** — full type safety, **zero runtime cost**. That's **generic programming**: abstract over the *type* the way a function abstracts over the *value*.
```
Without: maxInt, maxDouble, maxChar...   (N copies of identical logic)
With:    template<typename T> T maximum(T,T)   (ONE recipe → compiler generates only what you use)
```

### How it works: the compiler generates code
The key mechanical insight (comes up in interviews): a template is **not itself code — it's a recipe to generate code.** When you call `maximum(3, 7)`, the compiler:
1. Deduces `T = int` from the arguments.
2. Generates an actual `int maximum(int, int)` function (this is called **instantiation** — sub-topic 3).
3. Compiles and calls that.

So `maximum(3,7)` and `maximum(2.5,1.5)` produce **two separate real functions** in your binary — each as fast as if you'd hand-written it. **Zero runtime overhead** — all the "generic" work happens at **compile time** (unlike runtime/type-erasure generics in some languages).

### 🔑 The ultimate breakdown of `template <typename T>`
| Piece | What it is | Meaning |
|-------|-----------|---------|
| **`template`** | keyword | "Get ready — a blueprint is coming." |
| **`typename`** | keyword | "The placeholder coming next represents a **data type**." (`class` is interchangeable here) |
| **`T`** | template **type parameter** | the placeholder itself (the stand-in for the actual type) |
- Multiple params: `template <typename T, typename U>`.

### Template argument deduction
Usually you don't specify the type — the compiler **deduces** it from the arguments:
```cpp
maximum(3, 7);          // T deduced as int
maximum<double>(3, 7);  // or force T = double explicitly
```
(That's why a ctor with no args, like `my_vector<int> v;`, *needs* `<int>` — nothing to deduce from.)

**Gotcha:** one `T` for multiple params → all args must be the same type:
```cpp
maximum(3, 7.5);   // ❌ int vs double → T can't be both (cast one, or use <T,U>)
```

### Summary
- Function template = recipe (`template <typename T>`) → type-specific functions; logic written once.
- **Why:** no per-type duplication, full type safety, **zero runtime cost** (compile-time).
- **How:** compiler **instantiates** a real function per type used.
- Deduction infers `T` from args; one `T` across params → args must match.

---

## Sub-topic 2 — Class Templates

### Simple: a generic class
Same idea as a function template, but for a **whole class** — works for any `T`. You already built two: `SharedPtr<T>`, `my_vector<T>`.
```cpp
template <typename T>
class my_vector {
    T* data; int size;
public:
    void push_back(T val);
    T& operator[](int i);
};
```

### The WHY
Same as function templates, scaled up: a container has the **same logic regardless of element type**. Without class templates → `IntVector`/`StringVector`/`PersonVector` (identical but duplicated). The **entire STL** (`vector`, `map`, `pair`, `unique_ptr`…) is class templates.

### 🔑 Key difference from function templates: no deduction
- **Function template:** compiler **deduces** `T` from arguments → `maximum(3,7)` just works.
- **Class template:** you **must specify** the type → `my_vector<int> v;` (no arguments to deduce from). *(This is the error you hit when you wrote `my_vector v;`.)*
> C++17 **CTAD** deduces class template types in some cases (`std::vector v{1,2,3};` → `vector<int>`), but you must still write `<T>` for an empty object.

### Class-template details
**1. Member functions defined OUTSIDE the class** get verbose — repeat the header + qualify with `<T>`:
```cpp
template <typename T>
void my_vector<T>::push_back(T val) { ... }   // repeat template<...>, qualify my_vector<T>::
```
→ why template classes often define everything **inline inside** the class (what you did).

**2. Default template arguments:**
```cpp
template <typename T, typename Allocator = std::allocator<T>>
class vector { ... };
vector<int> v;   // Allocator defaults → why std::vector<int> needs no allocator arg
```

**3. Non-type parameters** (compile-time values, sub-topic 5):
```cpp
template <typename T, int N> class array { T data[N]; };
array<int, 5> a;   // exactly how std::array<int,5> works — 5 is a non-type param
```

### Each instantiation is a DISTINCT type
`my_vector<int>` and `my_vector<string>` are **separate, unrelated types** (compiler generates a full class for each) — can't assign one to the other:
```cpp
my_vector<int> a; my_vector<string> b;
a = b;   // ❌ different, unrelated types
```

### Summary
- Class template = generic class; one definition for any `T`. STL is all class templates.
- **Must specify `<T>`** at declaration (no deduction; C++17 CTAD relaxes some cases).
- Out-of-class members repeat `template<T>` + `X<T>::` (→ often inline); default template args; non-type params (`array<int,N>`).
- Each instantiation (`vector<int>` vs `vector<string>`) is a **distinct, unrelated type**.

---

## Sub-topic 3 — Instantiation + Why Templates Live in Headers

### Simple: "instantiation" = compiler generating the real code
A template is a **recipe**; **instantiation** = the compiler **using it to generate actual code** for a specific type.
```cpp
template <typename T> T maximum(T a, T b) { return a>b?a:b; }   // recipe — generates NOTHING alone
maximum(3, 7);     // HERE → instantiates maximum<int>
maximum(2.5,1.5);  // HERE → instantiates maximum<double>
```
**A template you never use generates no code** — instantiated **on demand**. (Implicit = automatic on use (normal); explicit = you force it, rare.)

### ⭐ Why you CAN'T split a template into .h/.cpp (classic gotcha)
Normal functions split fine: declaration in `.h`, definition in `.cpp`, the **linker** connects them (separate compilation). **Templates can't** — body in a `.cpp` → **linker error.** Why:

The compiler needs the **full body at the point of use** to instantiate (it generates code from the recipe). But each `.cpp` compiles **separately**. If you split:
```cpp
// maximum.h : template<typename T> T maximum(T,T);              // declaration only
// maximum.cpp: template<typename T> T maximum(T,T){return ...;} // body here
// main.cpp  : #include "maximum.h";  maximum(3,7);              // uses maximum<int>
```
1. **main.cpp:** sees only the declaration → wants `maximum<int>` but has no body → emits a *reference*, assumes someone provides it.
2. **maximum.cpp:** has the body, but **never uses** `maximum<int>` → instantiates **nothing**.
3. **Link:** `main.o` needs `maximum<int>`, generated **nowhere** → **undefined reference (linker error).**
> In one line: the file with the body never *uses* it (generates nothing); the file that uses it never *sees* the body (can't generate it). They never meet.

### The fix: header-only
A template's **full definition must be visible wherever it's used** → put the **entire template (declaration + definition) in the header**:
```cpp
// maximum.h — EVERYTHING here
template <typename T> T maximum(T a, T b) { return a>b?a:b; }
```
Every `.cpp` that `#include`s it gets the complete recipe → instantiates what it needs. That's why **the whole STL is header-only**, and why your single-file `SharedPtr`/`my_vector` worked.

**Tradeoff:** each including `.cpp` re-instantiates & compiles the template → **slower compiles** (the linker de-dups the identical copies, so no binary bloat — but the compile work repeats). A real reason template-heavy C++ compiles slowly.

### 💡 "What if I `#include "maximum.cpp"` in main?"
**It would work** — `#include` pastes the file's text into main, so the full body becomes visible → main *can* instantiate `maximum<int>` → no linker error. You've essentially made the `.cpp` act as a header. **But it's a hack:**
1. **The build system *also* compiles `maximum.cpp` separately** (it auto-compiles every `.cpp` as its own translation unit). So it's compiled twice. For a *pure template* that's harmless (templates generate nothing standalone) — but the moment the file has **any non-template code** (a normal function/global), that definition appears in **two object files** → **duplicate symbol → linker error (ODR violation)**.
2. **Violates convention** — `.cpp` means "compiled standalone"; `#include`-ing one surprises everyone and the build system.

**Clean version of the same idea** — split the impl into a header-like file (`.tpp`/`.inl`/`.ipp`, which build systems do NOT auto-compile) and `#include` it from the header:
```cpp
// maximum.h
template <typename T> T maximum(T a, T b);   // declaration
#include "maximum.tpp"                        // definitions pulled in at the bottom
// maximum.tpp
template <typename T> T maximum(T a, T b) { return a>b?a:b; }
```
This is just **header-only split into two files for readability** — legitimate. `#include`-ing a `.cpp` is the hacky version.
> One line: `#include "maximum.cpp"` compiles & links (body becomes visible) but is a hack — the `.cpp` is *also* compiled separately → wasteful + duplicate-symbol errors if it has non-template code. Use header-only (one file, or `.h` + `.tpp`).

#### Worked example: the ODR duplicate-symbol crash (why non-template code breaks it)
**🔑 Primary reason of the error: the GLOBAL-SCOPE definition.** (A template is special — it's allowed to appear in many TUs and gets de-duplicated. A normal global/function is NOT — its concrete definition must exist exactly once.)

**Phase 1 — add non-template code to `maximum.cpp`:**
```cpp
// Inside maximum.cpp
int global_status = 200;              // ⚠️ a normal global variable (global scope)
template <typename T> T my_max(T a, T b) { return (a > b) ? a : b; }
```

**Phase 2 — the accident: `#include "maximum.cpp"` in main.cpp.** During compilation:
1. **`maximum.o`:** compiler reads `maximum.cpp` directly → builds a binary copy of `global_status`.
2. **`main.o`:** compiler reads `main.cpp`; because it `#include`d `maximum.cpp`, it literally copy-pastes that code in → builds a **second** binary copy of `global_status` inside `main.o`.

**Phase 3 — the linker error (the crash):** the linker hooks both `.o` files into one executable. It finds `global_status` in `maximum.o` **and** another `global_status` in `main.o` → **"Duplicate Symbol / Multiple Definition Error!"**

This violates the **One Definition Rule (ODR):** you may **declare** a name as many times as you want, but a concrete **definition** can exist **exactly once** across the whole project. The template itself was fine (de-duplicated); the **global-scope `global_status` definition** is what got duplicated → the error.

### Interview answer
> Templates instantiate at **compile time** and need the **full definition at the point of use**. With separate compilation, the body-in-`.cpp` file uses it nowhere (instantiates nothing) and the user file sees only the declaration (can't instantiate) → the needed instantiation exists nowhere → **undefined-reference linker error**. Fix: **header-only** (whole template in the header). Tradeoff: slower compiles.

### Summary
- **Instantiation** = compiler generates real code from the recipe for a type; unused template → nothing (**on demand**).
- **Can't split .h/.cpp:** body needed at point of use; split → instantiation generated nowhere → **linker error**.
- **Fix: header-only** (entire template in header) → STL is header-only.
- **Tradeoff:** each TU re-instantiates → slower compiles.

---

## Sub-topic 6 — `auto` & `decltype` (type deduction)

### `auto` — "compiler, figure out the type"
Omit the type; the compiler **deduces** it from the initializer (still a fixed static type — just not spelled out):
```cpp
auto x = 5;          // int
auto y = 3.14;       // double
auto it = myMap.begin();              // the iterator type — no need to spell it out
auto f = [](int x){ return x*2; };    // a lambda — its type is UNNAMEABLE; auto is the only way
```

### The WHY
1. **Verbose types** — `std::vector<std::pair<int,std::string>>::iterator` → just `auto it = ...` (type is obvious from the RHS).
2. **Unnameable types** — lambdas have compiler-generated types you can't write → `auto` is required.
3. **Maintainability** — adapts if a return type changes; avoids silent wrong-type conversions.

### ⚠️ Key gotcha: `auto` STRIPS `const` and `&` (gives a value copy)
```cpp
const int ci = 5;
auto a = ci;      // int (NOT const int!) — const dropped; a is a copy

int x = 10; int& ref = x;
auto b = ref;     // int (NOT int&!) — reference dropped; b is a COPY
b = 99;           // changes b, NOT x
```
Keep them explicitly:
```cpp
const auto  a = ci;    // const int
auto&       b = ref;   // int&  (now b IS x)
const auto& c = ci;    // const int&  (no copy, read-only)
```
**Range-for:** prefer **`const auto&`** for non-trivial types (no copy, read-only); `auto&` to modify; plain `auto` copies each element (wasteful).
```cpp
for (const auto& x : container)   // usual safe default — no copy
```

### `decltype(expr)` — the EXACT declared type (keeps `const`/`&`)
```cpp
int x = 5;             decltype(x)   a;   // int
const int ci = 5;      decltype(ci)  b = 20;  // const int   (KEEPS const — unlike auto)
int& ref = x;          decltype(ref) c = x;   // int&        (KEEPS reference)
```

### auto vs decltype
| | `auto` | `decltype` |
|---|---|---|
| Deduces from | the **initializer value** | an **expression's declared type** |
| `const`/`&` | **stripped** (plain value copy) | **preserved** (exact) |
| Use for | locals, iterators, lambdas, range-for | generic code needing the *precise* type / computed return types |

Classic `decltype` use — return type depends on args:
```cpp
template <typename A, typename B>
auto add(A a, B b) -> decltype(a + b) { return a + b; }   // add(3, 2.5) → double
```
*(C++14+: just `auto add(A,B)` deduces the return type from `return` — often no explicit decltype needed.)*

### Guidance
- Use **`auto`** freely where the type is clear (iterators, lambdas); don't overuse where it hurts readability.
- Prefer **`const auto&`** in loops over non-trivial types.
- **`decltype`** is mostly a generic-programming tool — you'll *read* it more than write it.

### Summary
- **`auto`** = deduce a variable's type from its initializer; **strips `const`/`&`** → value copy by default (keep with `const auto&`, `auto&`).
- **`decltype(expr)`** = exact declared type, **preserves `const`/`&`**; for generic code / computed return types.
- **auto = convenient value deduction (strips qualifiers); decltype = exact type query (keeps them).**

---

## Code examples in this folder

| File | Demonstrates |
|------|--------------|
| `auto_decltype.cpp` | `auto` strips `const`/`&` (modifying the copy leaves the original unchanged); `auto&` keeps the ref; `decltype` preserves `const`/`&`; `decltype` computed return type |
