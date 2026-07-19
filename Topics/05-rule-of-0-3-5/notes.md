# Topic 5 — Constructors/Destructors + Rule of 0/3/5

Tier 1 #5 (~2 days). You've already **lived** most of this — hand-writing `shared_ptr` in Topic 3 meant writing a destructor, copy constructor, and copy assignment operator. **That *is* the Rule of 3.** This topic names it, formalizes it, and adds the missing pieces.

## Why this topic matters

These are the **special member functions** — the ones the compiler will silently generate for you if you don't write them. Knowing *when the compiler's version is wrong* (and therefore when you MUST write your own) is the whole skill. It's the difference between a class that manages a resource correctly and one that double-frees or leaks.

---

## Progress tracker (sub-topics)

| # | Sub-topic | The classic question | Status |
|---|-----------|----------------------|--------|
| 1 | **The special member functions** — default, parameterized, copy, move ctors; destructor | "What are the special member functions?" | ✅ Done |
| 2 | **Copy ctor vs copy assignment** (and move ctor vs move assignment) | "When is the copy ctor called vs assignment?" | ✅ Done |
| 3 | **Deep vs shallow copy** | "What's the danger of the compiler's default copy?" | ✅ Done |
| 4 | ⭐ **Rule of 0 / 3 / 5** — when you must define them | "Explain the Rule of Three/Five." | ✅ Done |
| 5 | **Member initializer lists** + construction/destruction order | "Why prefer the init list over assigning in the body?" | ✅ Done |
| 6 | **`explicit`** keyword | "What does `explicit` prevent?" | ✅ Done |
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

## Sub-topic 2 — Copy Constructor vs Copy Assignment

| | Copy **constructor** | Copy **assignment** |
|---|---|---|
| Signature | `MyClass(const MyClass& other)` | `MyClass& operator=(const MyClass& other)` |
| The target object is… | **brand new** — doesn't exist yet | **already exists** and already owns something |
| So it must… | just **build** itself from `other` | **release its old resource first**, *then* take `other`'s |
| Needs self-check? | ❌ no (you don't exist yet) | ✅ **yes** (`a = a` is possible) |
| Returns | nothing (it's a constructor) | `MyClass&` (for chaining `a = b = c`) |

> **Construction = start from nothing; assignment = replace what's already there.** Exactly why `operator=` needed the release step + self-guard in the hand-written `shared_ptr`, and the copy ctor didn't.

### ⚠️ The `=` trap (which one actually fires)
```cpp
MyClass b = a;   // ⚠️ COPY CONSTRUCTOR — type on the left = declaration → object being created
MyClass c(a);    // copy constructor (clearer syntax)
MyClass d(5);
d = a;           // ✅ COPY ASSIGNMENT — d already exists
```
**Rule:** type on the left → *declaration* → **copy ctor**. Object already exists → **copy assignment**.
*(Hit exactly this while building `shared_ptr`: `shared_ptr p4 = p1;` called the copy ctor, not `operator=`.)*

### Where each fires
- **Copy ctor:** `MyClass b = a;` / `b(a)`; **pass-by-value**; return-by-value (often elided — Topic 7); container insertion by value.
- **Copy assignment:** assigning to an already-existing object.

> Pass-by-value silently invokes the copy ctor → the hidden cost behind preferring `const T&` params.

**Moves mirror this exactly:** move ctor = new object steals from a dying source; move assignment = existing object releases its own, then steals. (Topic 7.)

---

## Sub-topic 3 — Deep vs Shallow Copy

The **actual mechanism** behind why the Rule of 3 exists — the bug you get when you let the compiler copy for you.

### What the compiler's default copy does
If you don't write a copy constructor, the compiler generates one that does a **member-wise copy** — copying each member's *value*, one by one. For plain values that's perfect:
```cpp
class Point { int x, y; };
Point b = a;   // copies x and y — fine ✅
```
But when a member is a **pointer**:
```cpp
class Buffer {
    int* data;                      // owns heap memory
public:
    Buffer()  { data = new int[100]; }
    ~Buffer() { delete[] data; }
};
Buffer a;
Buffer b = a;   // compiler copies the POINTER VALUE (the address)
```

### Shallow copy — the problem
`data` is just **an address**, so `a.data` and `b.data` now hold the **same address**, pointing at the **same** heap array:
```
   a                      b
 ┌──────────┐           ┌──────────┐
 │ data ────┼──────┬────┼──── data │   ← both point to the SAME array
 └──────────┘      │    └──────────┘
                   ▼
            ┌─────────────┐
            │ heap array  │
            └─────────────┘
```
That's a **shallow copy** — it duplicated the *pointer*, not the *thing pointed to*. Two fatal problems:

1. **Double-free 💥** — when both objects die, **both destructors run `delete[] data`** on the same address. The second is a double-free → crash/corruption.
2. **Unintended aliasing** — they aren't independent: `b.data[0] = 99;` also changes what `a` sees. That's not what "copy" should mean.

### Deep copy — the fix
A **deep copy** duplicates the *pointed-to resource*, not just the pointer, so each object owns its own:
```
   a                      b
 ┌──────────┐           ┌──────────┐
 │ data ────┼──►[array] │ data ────┼──►[array]   ← separate arrays
 └──────────┘           └──────────┘
```
The copy constructor must **allocate new memory and copy the contents** in, rather than just assigning the pointer. Then each destructor frees its *own* array — no double-free, no aliasing.

### 🔑 This is exactly why the Rule of 3 exists
- You wrote a **destructor** (because you own a resource).
- The compiler's default **copy** is *shallow* → two objects share one resource.
- Both destructors fire → **double-free.**

> **The moment you write a destructor that frees something, the compiler-generated copy operations become dangerous.** That's the reasoning behind "if you need one, you need all three."

### ⚠️ Important nuance: shallow isn't always wrong
**Shallow copy is exactly what `shared_ptr` wants!** Its copy constructor deliberately copies the *pointer* (shallow) so both wrappers point to the same object — that's the point of *shared* ownership. What makes it **safe** is the **reference count**: only the *last* owner deletes, so no double-free.

So the real rule isn't "always deep copy":
> **Decide what copying should MEAN for your class — duplicate the resource (deep) or share it (shallow) — then make the destructor consistent with that choice.** Deep copy → each frees its own. Shallow sharing → you need refcounting (or non-owning semantics) so only one frees.

The compiler's default is *always* shallow and knows **nothing** about ownership. That's why you must step in.

### Summary
- **Shallow copy** (compiler default) — copies members' values; for a pointer that means copying the **address** → both objects share one resource.
- Danger: **double-free** + **unintended aliasing**.
- **Deep copy** — allocate new memory, copy the **contents** → independent objects, each frees its own.
- This is the mechanism behind the **Rule of 3**.
- **Shallow isn't always wrong** — `shared_ptr` copies shallowly *on purpose*, made safe by the refcount. Choose deliberately; keep the destructor consistent.

---

## Sub-topic 5 — Member Initializer Lists

### What it is
Two ways to set up members in a constructor:
```cpp
class Person {
    std::string name;
    int age;
public:
    // Way 1 — assignment in the BODY
    Person(std::string n, int a) { name = n; age = a; }

    // Way 2 — member INITIALIZER LIST (the colon before the body)
    Person(std::string n, int a) : name(n), age(a) { }
    //                           ^^^^^^^^^^^^^^^^^ the init list
};
```
Both "work", but they do different things under the hood — Way 2 is the correct default.

### Why the init list is better: initialize vs assign
Members are **constructed before the constructor body runs** (Topic 1). So with body assignment:
```cpp
Person(std::string n, int a) { name = n; }   // name was ALREADY default-constructed
                                             // (empty string) → now thrown away & overwritten
```
That's **two steps**: default-construct, then assign. With the init list it's **one step** — `name` is constructed directly from `n`, no throwaway default construction.

Irrelevant for an `int`; but for `std::string`, `std::vector`, or any heavy class you're doing double work on every construction.

### 🔑 The key insight: declaring a member does NOT create it

```cpp
class Person {
    std::string name;   // ← this is just a DECLARATION (a blueprint entry)
    int age;
public:
    Person(std::string n, int a) : name(n), age(a) { }
};
```

That line does **not** create a string. It only tells the compiler: *"every `Person` object will contain a member called `name` of type `string`."* It's the **blueprint, not the building.**

The actual `string` object is **created (constructed) when a `Person` is created** — and *that's* the moment the init list controls.

### Does declaring a member allocate memory / fill garbage? — Two stages

**Stage 1: the class definition — NO memory at all.**
```cpp
class Person { std::string name; int age; };
```
Zero bytes exist. Nothing allocated, no garbage. It's purely a **description of a type**. (The compiler only works out the *size* a `Person` will need.)

**Stage 2: creating an object — memory IS allocated.**
```cpp
Person p;   // NOW memory is reserved for name + age, all at once
```
But **allocated ≠ initialized**. What's *in* that memory depends on the member's type:

| Member type | At object creation |
|---|---|
| **Class type with a constructor** (`std::string`, `std::vector`) | its **default constructor runs** → properly initialized (empty string/vector). **Not garbage** ✅ |
| **Built-in type** (`int`, `double`, `int*`, `bool`) | memory reserved but **nothing runs** → holds leftover bits → **GARBAGE** ⚠️ |

```cpp
Person p;
// p.name → a valid EMPTY string (string's default ctor ran)
// p.age  → GARBAGE (int has no constructor; nobody initialized it)
```

**🎯 This explains the `ref_cnt` bug from the hand-written `shared_ptr`:**
```cpp
class shared_ptr {
    int* ref_cnt;                    // built-in type → NO constructor runs → garbage
    shared_ptr(T* p) {
        *ref_cnt = *new int(1);      // ❌ wrote through a GARBAGE pointer → UB
    }
};
```
A raw pointer is a built-in type, so it was allocated but never initialized. Had it been a `std::string`, it would have been safely empty automatically.

> **Practical rule: always initialize built-in members explicitly** (init list or a default member initializer) — nothing will do it for you. Class-type members take care of themselves.
> ```cpp
> class Person {
>     std::string name;   // safe — string's ctor gives an empty string
>     int age = 0;        // ✅ initialize built-ins yourself
> };
> ```

### Allocation ≠ Construction (what "built" actually means)

| | What it does | Result |
|---|---|---|
| **Allocation** | reserves raw **bytes** | a plot of land — random junk in it |
| **Construction** | runs the **constructor** to turn those bytes into a *valid* object | the house built on that land |

Memory being reserved doesn't make it a working object — **construction is what makes those bytes *mean* something.**

**Concrete: a `std::string` is roughly**
```cpp
class string { char* buffer; size_t size; size_t capacity; };
```
**After allocation only** (~24 bytes reserved for `p.name`):
```
buffer   = 0x8F3A21B7   ← random garbage
size     = 918273645    ← random garbage
capacity = -4029384     ← random garbage
```
Not a usable string — `.size()` would return nonsense, touching `buffer` would crash. The memory exists but is meaningless.

**After construction** (`string()` runs):
```
buffer   = (points to an empty internal buffer)
size     = 0
capacity = 15
```
Now it's a **valid** empty string — printing, appending, `.size()` all work.

> **"Built" = running the constructor that establishes the object's invariants** (its internal consistency). Memory alone doesn't do that.

**Why `int` seems to skip this:** an `int` has no internal structure or invariants — *any* 4-byte pattern is a valid `int`. So "construction" is a no-op → left as **garbage**. A `string` must run its constructor to become valid.

**References are the special case:** a reference has no storage of its own (Topic 2 — it's an alias). So **binding IS the construction** — there's no "allocate now, bind later," which is exactly why it must happen in the init list.

### 🔑 Worked example — the two paths, byte by byte
Constructing `Person p("vaseem");` — watch `p.name`'s internals.

**Path A — Init list:** `Person(string n) : name(n) { }`
```
1. ALLOCATE — 24 bytes reserved for p.name:
       buffer   = 0x8F3A21B7   ← garbage
       size     = 918273645    ← garbage
       capacity = -4029384     ← garbage

2. CONSTRUCT — string's COPY ctor runs, using n ("vaseem"):
       buffer   → "vaseem"     ← allocates & copies the chars
       size     = 6
       capacity = 15
```
**Done — ONE construction.** The garbage is overwritten once, directly with the final value. ✅

**Path B — Body assignment:** `Person(string n) { name = n; }`
```
1. ALLOCATE — same 24 bytes of garbage.

2. CONSTRUCT — string's DEFAULT ctor runs (nobody said what to build with):
       buffer   → (empty internal buffer)
       size     = 0
       capacity = 15
   ← a valid, usable, but EMPTY string

3. ASSIGN — now the body's `name = n;` runs (operator=):
       buffer   → "vaseem"
       size     = 6
```
**TWO operations.**

| | Init list | Body assignment |
|---|---|---|
| Step 1 | allocate | allocate |
| Step 2 | **construct with `"vaseem"`** ← done | construct as **empty** |
| Step 3 | — | **assign `"vaseem"`** |
| Total | **1** operation | **2** operations |

> ⚠️ **Precision note:** step 2 in Path B isn't merely "zeroing" the bytes — it's the **default constructor doing real setup work** (a real internal buffer, `size = 0`, `capacity = 15`) to produce a *valid* empty string. That's why it's wasteful: for a long value, step 2 may even allocate a small buffer that step 3 immediately discards for a bigger one. **Real work done, then thrown away.**

### Why you can't avoid the default construction in the body

The crucial timing rule (Topic 1): **all members are fully constructed BEFORE the constructor body's first line runs.**

```cpp
Person(std::string n) {
    // ⬆ by the time we reach HERE, `name` already exists as an empty string
    name = n;   // so this can only be an ASSIGNMENT, never an initialization
}
```

You physically **cannot** initialize a member in the body — the member is already built by then. The body can only **assign** (overwrite). The **init list is the only place** where you get to say *how the member is constructed in the first place*.

> **One line:** declaring the member is just the blueprint. The init list decides **how it's built**; the body can only **change it after it's already built**.

> ### ⚠️ AND THAT'S WHY `const` AND REFERENCE MEMBERS **REQUIRE** THE INIT LIST
> **You can't "change after building" something that can never be changed. There's no second chance.**

### Three cases where the init list is REQUIRED, not optional
**1. `const` members** — you can't assign to a const:
```cpp
class A { const int id;
public: A(int i) { id = i; }   // ❌ can't assign to a const member
        A(int i) : id(i) { }   // ✅
};
```
**2. Reference members** — a reference must be **bound** at creation and can never be re-seated (Topic 2):
```cpp
class B { int& ref;
public: B(int& r) { ref = r; }   // ❌ assigns a VALUE, doesn't bind the reference
        B(int& r) : ref(r) { }   // ✅
};
```
> **"Binding"** = attaching the reference to the object it aliases. Happens **once, at creation**, permanently (`int& r = x;` binds `r` to `x`).
>
> Why the body fails: by the time `{` is reached, `ref` must **already be bound to something** — but you never said what, and there's no such thing as an "unbound" reference → *"uninitialized reference member."* And even then, `ref = r;` would **assign `r`'s value into whatever `ref` aliases**, never re-point it (Topic 2 rule 2). So a reference **cannot** be bound in the body at all.

**3. Members whose TYPE has no default constructor** — the compiler can't default-construct the member before the body:
```cpp
class C { Widget w;            // Widget has no default ctor
public: C() { w = Widget(5); } // ❌
        C() : w(5) { }         // ✅
};
```
> ⚠️ **This is about the MEMBER'S OWN TYPE, not about your constructor.** A **default constructor** takes **zero arguments** — `Widget()`. 
> ```cpp
> class Widget { public: Widget(int x) { } };   // only ctor takes an int
> Widget w;      // ❌ no Widget() exists
> Widget w(5);   // ✅
> ```
> **Key rule:** the compiler gives you a free default constructor **only if you write no constructors at all**. Define *any* constructor and the free `Widget()` **disappears** — that's how a class ends up with "no default constructor."
>
> Why the body fails: before the body's first line, `w` must already be constructed. The compiler asks *"how do I build `w` with no arguments?"* → needs `Widget()` → doesn't exist → **error before your `w = Widget(5);` ever runs.** The init list fixes it by saying *how* to build `w` (using `Widget(int)`).

> ### 🔗 The unifying idea across all three cases
> The body runs **after** members are built. If a member **can't be built without your input** (needs an argument) or **can't be changed after being built** (`const`, reference), then the body is **too late**. The init list is the only place that participates in the actual construction.
Same for **base classes** — initialize a base in the init list, since it must be built before your body runs.

### ⚠️ The gotcha: declaration order wins
> **Members are initialized in the order they are DECLARED in the class — not the order you list them in the init list.**
```cpp
class X {
    int a_;
    int b_;
public:
    X() : b_(1), a_(b_) { }   // ⚠️ a_ is declared first → initialized FIRST,
};                            //    using b_ which is still garbage
```
GCC warns via `-Wreorder`. **Rule: always write the init list in declaration order.**

### Applied to the hand-written `shared_ptr`
```cpp
shared_ptr(T *p) { ptr = p; ref_cnt = new int(1); }              // what we wrote (harmless for pointers)
shared_ptr(T *p) : ptr(p), ref_cnt(new int(1)) { }               // idiomatic version
```
Raw pointers are trivial so there's no wasted work — but interviewers notice the idiom.

### Summary
- **Init list** = `: member(value)` between the signature and the body.
- It **initializes** directly; body assignment **default-constructs then overwrites** → wasteful for non-trivial types.
- **Required** for: **`const` members**, **reference members**, **members/bases without a default constructor**.
- ⚠️ Initialization follows **declaration order**, not init-list order.
- Prefer the init list by default.

---

## Sub-topic 6 — The `explicit` Keyword

### The problem it solves: implicit conversions
By default, a constructor that takes **one argument** doubles as an **implicit conversion** — the compiler will silently use it to convert that argument type into your class, without you asking.

```cpp
class Distance {
    int meters;
public:
    Distance(int m) : meters(m) { }        // single-arg constructor
};

void travel(Distance d) { /* ... */ }      // takes a Distance

int main() {
    travel(50);     // ⚠️ COMPILES! The compiler silently did: travel(Distance(50))
}
```
You passed an `int` where a `Distance` was expected — and it just… worked. The compiler quietly converted `50` into a `Distance`. Sometimes convenient; often **a bug hiding in plain sight**.

### Why silent conversions are dangerous
```cpp
Distance d = 50;   // ⚠️ compiles — is 50 meters? feet? did the author mean this?
```
Worse, it lets **nonsense** compile:
```cpp
void setDistance(Distance d) { /* ... */ }

int main() {
    setDistance(true);   // ⚠️ bool → int → Distance. Compiles! Meaningless.
    setDistance('x');    // ⚠️ char → int → Distance. Compiles! Also meaningless.
}
```
The compiler happily **chains** conversions and builds a `Distance` from a `char`. You wanted a type error; you got silent garbage.

### The fix: `explicit`
Mark the constructor `explicit` and the compiler will **only** use it when you ask for it directly:
```cpp
class Distance {
    int meters;
public:
    explicit Distance(int m) : meters(m) { }   // ← no implicit conversions
};

void travel(Distance d) { /* ... */ }

int main() {
    travel(50);             // ❌ ERROR now — good! an int is not a Distance
    travel(Distance(50));   // ✅ explicit and clear

    Distance a(50);         // ✅ direct initialization
    Distance b = 50;        // ❌ ERROR — copy-initialization blocked
}
```
You must **spell out** that you want a `Distance`. The intent becomes visible in the code.

### 🎯 The real danger: silent OWNERSHIP TRANSFER
*(Full runnable walkthrough: **`explicit_demo.cpp`** in this folder.)*

The hand-written `shared_ptr` has a **single-argument** constructor — `shared_ptr(T* p)` (the `ref_cnt` is created inside, not passed) — so it acts as an implicit conversion from `T*` to `shared_ptr<T>`. Now consider a function that takes ownership:

```cpp
void takeOwnership(shared_ptr<Person> sp);   // by value = "give me ownership"

Person* raw = new Person("vaseem", 23);      // I own this; I'll delete it later
takeOwnership(raw);                           // ⚠️ I only passed a RAW POINTER
delete raw;                                   // 💥 DOUBLE FREE
```

**What actually happens:**
1. `takeOwnership` needs a `shared_ptr<Person>` but got a `Person*`. Since the ctor isn't `explicit`, the compiler **silently rewrites the call** as `takeOwnership(shared_ptr<Person>(raw))` — a `shared_ptr` is created at the call site, **count = 1**.
2. That `shared_ptr` is bound to the parameter `sp`, so when `takeOwnership` **returns**, `sp` is destroyed → count drops to **0** → it **deletes the Person**.
3. Back in `main`, `raw` is now **dangling** — the object died without the caller ever asking.
4. The caller's own `delete raw;` → **double free** (UB / crash).

**One innocent-looking line silently transferred ownership and caused a double-free.**

**The fix** — mark it `explicit`:
```cpp
explicit shared_ptr(T* p) { ... }

takeOwnership(raw);                        // ❌ no longer compiles
takeOwnership(shared_ptr<Person>(raw));    // ✅ forced to make the transfer VISIBLE
```
You can still do it — but only **on purpose**, and it's obvious in the code.

> **Ownership transfer should never be silent.** That's exactly why the real `std::shared_ptr` marks its raw-pointer constructor `explicit`.

### Rule of thumb
> **Mark single-argument constructors `explicit` by default.** Leave it off only when the conversion is genuinely natural and safe — e.g. `std::string s = "hello";` (from `const char*`) reads naturally and surprises nobody.

Anything surprising, lossy, or ownership-transferring → **`explicit`**.

---

## Code examples in this folder

| File | Demonstrates |
|------|--------------|
| `explicit_demo.cpp` | Full runnable walkthrough: a non-`explicit` single-arg ctor silently transferring ownership → dangling pointer → double free, and the `explicit` fix |
