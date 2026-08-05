# Topic 7 — Move Semantics & Rvalue References

Tier 2 #7 (~4 days). **The single most-probed "modern C++" topic** — the filter between "knows C++03" and "knows modern C++." Builds directly on the copy ctor / Rule of 0/3/5 you just learned: move ops are the natural next layer — *"instead of copying, steal."*

> This topic also lets us **finish the hand-written `shared_ptr`** (add the move ctor + move assignment → Rule of 5).

## Why this topic matters

Before C++11, returning or passing big objects by value meant **expensive deep copies** even when the source was a temporary about to be destroyed. Move semantics lets you **transfer** (steal) resources instead of copying them — huge performance win, and the reason modern C++ can pass heavy objects around cheaply.

---

## Progress tracker (sub-topics)

| # | Sub-topic | The classic question | Status |
|---|-----------|----------------------|--------|
| 1 | **lvalues vs rvalues** | "What's an lvalue vs an rvalue?" | ✅ Done |
| 2 | **rvalue references (`&&`)** | "What is `T&&`?" | ✅ Done |
| 3 | **`std::move`** — what it actually does | "Does `std::move` move anything?" | ✅ Done |
| 4 | **Move constructor & move assignment** | "How do you write move ops?" | ✅ Done |
| 5 | **When a move happens vs a silent copy** | "When does move fall back to copy?" | ✅ Done |
| 6 | **Copy elision / RVO / NRVO** | "What is RVO and how does it interact with move?" | ✅ Done |
| 7 | **`std::forward` & perfect forwarding** (lighter) | "What's the difference between move and forward?" | ✅ Done |
| 8 | ⭐ **Finish `shared_ptr`: move ctor + move assignment (Rule of 5)** | (completes Topic 3's capstone) | ✅ Done — see `03-smart-pointers/shared_ptr_impl.cpp` |

---

## Sub-topic 1 — lvalues vs rvalues

### Simple: two kinds of "things" in your code
- **lvalue** = something with a **name** and a **place in memory** — it *persists*. You can take its address (`&`).
- **rvalue** = a **temporary** value with no lasting home — about to disappear. You *can't* take its address.
```cpp
int x = 10;
x        // lvalue — has a name, lives somewhere, sticks around
10       // rvalue — temporary literal, gone after this line
x + 5    // rvalue — the result 15 is a temporary
```
Test: **"Can I take its address with `&`?"** `&x` ✅ (lvalue) · `&10` ❌, `&(x+5)` ❌ (rvalues).
Memory hook: **l**value = "**l**ocator" (has a location); **r**value = temporary, usually on the **r**ight of `=`.

### The WHY — why this distinction was invented
```cpp
std::string a = "hello";
std::string b = a;                     // (1) copy from a named variable
std::string c = std::string("world");  // (2) copy from a temporary
```
Both make a **deep copy** (duplicate the char buffer). But:
- **(1)** `a` is an **lvalue** — it lives on. If `b` stole `a`'s buffer, `a` would break. So we *must* copy. ✅
- **(2)** `std::string("world")` is an **rvalue** — a temporary that **dies at end of line anyway.** Copying its buffer then destroying the original is **wasteful.**

> **The key realization:** if the source is a **temporary (rvalue) about to be destroyed**, why copy its resources? Just **steal** them — take its buffer, leave it empty (it's dying anyway). Skips an expensive copy, nobody notices.

But to do that, the language needs a way to **tell the two apart** — "permanent lvalue I must copy, or doomed rvalue I can safely rob?" **That's why lvalue/rvalue matters: it's the signal that decides copy vs steal.**

```
b = a;                    a is an LVALUE (lives on)  → must COPY (safe)
c = std::string("world"); temp is an RVALUE (dying)  → could STEAL (cheap!) ← the opportunity
```
Everything else in this topic (`&&`, `std::move`, move ctors) is machinery to exploit one insight: **rvalues are safe to plunder.**

### Summary
- **lvalue** = named, persistent, has address (`&` works) → copy if you want a duplicate.
- **rvalue** = temporary, no name, no address, about to die.
- Test: can you take its address? Yes → lvalue; no → rvalue.
- **Why it matters:** an rvalue's resources are **safe to steal instead of copy** — the foundation move semantics is built on.

---

## Sub-topic 2 — Rvalue References (`&&`)

### Simple: a reference that binds ONLY to rvalues
`&` = normal (lvalue) reference; `&&` = **rvalue reference**, binds only to temporaries:
```cpp
int x = 10;
int&  lref = x;    // lvalue ref → binds an lvalue
int&& rref = 10;   // rvalue ref → binds a temporary
int&  bad  = 10;   // ❌ normal ref can't bind a temporary
int&& bad2 = x;    // ❌ rvalue ref can't bind a named lvalue
```

### The WHY — it lets functions overload on lvalue vs rvalue
Rvalues are safe to steal (sub-topic 1). But how does a function *know* its argument is a stealable temporary vs a must-not-touch lvalue? `&&` lets you write **two versions** and have the compiler pick:
```cpp
void process(const std::string& s);   // (A) binds LVALUES  → copy/read, don't steal
void process(std::string&& s);        // (B) binds RVALUES  → temporary, safe to steal!

std::string a = "hello";
process(a);                    // → (A) — a is an lvalue
process(std::string("temp"));  // → (B) — temporary, steal-safe
process("literal");            // → (B) — also temporary
```
> `&&` is the **type-system tag for "this is a temporary you may plunder."** Move ctor / move assignment are just functions taking `T&&` — the "steal" versions the compiler calls for rvalues. Without `&&`, no function could activate only for temporaries.

```
Before &&: one version → had to COPY to be safe, always.
With &&:   process(std::string&&) ← fires ONLY for temporaries, where stealing is safe.
```

### ⚠️ The twist (classic trap, sets up std::move)
```cpp
void process(std::string&& s) {   // s's TYPE is rvalue reference...
    // ...but inside here, s itself is an LVALUE (it has a name → &s works)
}
```
The rvalue-ness described **how it was bound**, not what `s` is once you use it. A **named** thing is an lvalue. So to keep treating `s` as stealable when passing it onward, you must **re-cast it back to an rvalue** — that's exactly what `std::move` does.
> **`T&&` as a parameter type = "binds to rvalues." But a *named* variable of that type is an lvalue when used.** Type vs value-category — keep them separate. This is *the* reason `std::move` is needed.

### Summary
- **`&&`** = rvalue reference — binds only to rvalues (temporaries).
- **Why:** enables **overloading on lvalue vs rvalue** → compiler routes temporaries to a steal-safe version. The tag for "you may plunder this."
- Move ctor/assignment are functions taking `T&&`.
- ⚠️ A *named* rvalue reference is itself an **lvalue** when used → re-cast with `std::move`.

---

## Sub-topic 3 — `std::move`

### Simple: it does NOT move anything — it's just a cast
> **`std::move` is just a cast** from lvalue → rvalue reference. It *relabels* something as "treat me as a temporary you can steal from." No data moves, no memory touched.
```cpp
std::string a = "hello";
std::string b = std::move(a);   // std::move(a) just CASTS a to an rvalue
```
The chain:
```
std::move(a)  → casts a to rvalue (relabel only)
     ↓
picks the MOVE constructor (the T&& overload)
     ↓
move constructor STEALS a's buffer   ← the actual "move" happens HERE
```
`std::move` is the **trigger**, not the action.

### The WHY
A **named** variable is always an lvalue (sub-topic 2's twist) → defaults to **copy**:
```cpp
std::string a = "hello";
std::string b = a;             // ❌ a is an lvalue → COPY (wasteful if you're done with a)
std::string b = std::move(a);  // ✅ cast a → rvalue → MOVE ctor → steal a's buffer
```
You know `a` is disposable; the compiler doesn't. **`std::move` is how you tell it:** *"I promise I'm done with `a` — treat it as a temporary and steal from it."* Temporaries move automatically; `std::move` extends that to *named* variables you're finished with.

### 🔑 THE core benefit: move is O(1), copy is O(n) (speed, not space)
Common misconception: "move iterates and copies the chars like copy, just saving space." **No — move does NOT touch the characters at all.** A `std::string` is 3 small fields + a heap buffer:
```cpp
class string { char* buffer; size_t size; size_t capacity; };
```
**COPY (O(n))** — makes `b` independent → allocates a new buffer + copies all n chars:
```
a.buffer ──► [h,e,l,l,o]      (original)
b.buffer ──► [h,e,l,l,o]      (brand-new copy — n chars copied one by one)  ← O(n) loop
```
**MOVE (O(1))** — steals the pointer; the buffer never moves:
```cpp
b.buffer = a.buffer;   // copy the POINTER (8 bytes)
b.size = a.size; b.capacity = a.capacity;
a.buffer = nullptr;    // null the source so it won't free the buffer
```
```
Before:  a.buffer ──► [h,e,l,l,o]      b.buffer ──► (none)
After:   a.buffer ──► nullptr          b.buffer ──► [h,e,l,l,o]   ← SAME buffer, re-pointed! no copy
```
| | Copy | Move |
|---|---|---|
| Allocate new buffer? | ✅ | ❌ |
| Copy n chars? | ✅ (the O(n) loop) | ❌ |
| **Time** | **O(n)** | **O(1)** ⚡ |

A 1-million-char string: copy = allocate 1 MB + copy 1M chars (slow); move = re-point 3 fields (**instant**, same as a 5-char string).

> **THE why of move semantics = the time reduction.** For a temporary dying anyway, don't spend O(n) duplicating a buffer you're about to throw away — just re-point in O(1). Saved **time** is the primary benefit; skipping the extra allocation is secondary.

**⚠️ Nuance — move is O(1) only when there's a resource (pointer/handle) to steal.** Types storing data *inline* gain nothing:
```cpp
std::array<int,1000> a;
auto b = std::move(a);   // still O(n)! no pointer to steal → copies inline data element by element
```
`string`/`vector`/`unique_ptr` (heap pointer inside) → move is O(1). Plain inline data → move = copy.

### ⚠️ What happens to the source
After moving from `a`, it's left **valid but unspecified** (for `string`/`vector`, usually empty — buffer stolen):
```cpp
std::string b = std::move(a);   // b owns "hello"
std::cout << a;   // ⚠️ don't rely on a's value — unspecified
a = "new";        // ✅ safe — a is still a valid object, can reassign/destroy
```
Rule: after moving, **don't read** the source's value; you *may* **reassign/destroy** it.

### Common uses
```cpp
v.push_back(std::move(s));           // move s into a container instead of copying
std::unique_ptr<W> b = std::move(a); // transfer ownership of a move-only type
```

### Summary
- **`std::move` = a cast**, not a move; relabels lvalue → rvalue.
- It **triggers** the move ctor/assignment (the `T&&` overload) which does the real stealing.
- **Why:** named vars are lvalues → default to copy; `std::move` opts into stealing something you're done with.
- ⚠️ Source is valid-but-unspecified after; don't read it, may reassign.

---

## Sub-topic 4 — Move Constructor & Move Assignment

The Rule-of-5 partners to the copy operations — but they **steal** the resource instead of copying it.

### The signatures
```cpp
class MyString {
    char* data;
public:
    MyString(MyString&& other) noexcept;              // move constructor
    MyString& operator=(MyString&& other) noexcept;   // move assignment
};
```
They take **`MyString&&`** (rvalue reference), not `const MyString&` — that `&&` makes the compiler pick them for rvalues (temporaries / `std::move`'d values).

### The pattern: steal, then null the source
**Move constructor:**
```cpp
MyString(MyString&& other) noexcept {
    data = other.data;       // 1. STEAL — take other's pointer (O(1), no copy)
    other.data = nullptr;    // 2. NULL the source — so its dtor won't delete our buffer
}
```
vs the copy ctor (`new` + `strcpy` → O(n)). Move = two pointer assignments.

### Why nulling is critical (else double-free)
After the steal, both point to the same buffer:
```
this->data ──┐
             ├──► [h,e,l,l,o]   ← same buffer, two owners
other.data ──┘
```
If you don't null `other.data`, when `other` dies its dtor does `delete[] other.data` → frees **your** buffer; then your dtor frees it again → **double-free** (Topic 1). Nulling makes `other`'s dtor a safe `delete[] nullptr` no-op.

### Move assignment (trickier — like copy assignment)
Handles `a = std::move(b)` when `a` already owns something → release first + self-guard:
```cpp
MyString& operator=(MyString&& other) noexcept {
    if (this == &other) return *this;   // 1. self-assignment guard
    delete[] data;                      // 2. RELEASE our current resource
    data = other.data;                  // 3. STEAL other's pointer
    other.data = nullptr;               // 4. NULL the source
    return *this;                       // 5. return *this (chaining)
}
```
Same skeleton as copy assignment (*guard → release old → take new → return `*this`*) — only step 3 **steals** instead of deep-copying.

### ⚠️ Always mark move ops `noexcept`
Or containers fall back to **copying** on reallocation (detailed reasoning below). A proper move (pointer steal) never throws, so `noexcept` is both true and important.

### Compiler auto-generates moves (Rule of 0)
For classes whose members manage themselves (`string`, `vector`), the compiler-generated move just moves each member → O(1), no hand-writing:
```cpp
class Person { std::string name; std::vector<int> data; };  // moves auto-generated
```
Write moves **by hand** only when managing a **raw resource** (raw pointer) — the Rule-of-5 / `shared_ptr` case. Caveat: declaring a destructor or copy op **suppresses** auto-generated moves → you must write them.

### Summary
- **Move ctor** `T(T&&) noexcept` — steal pointer, null source. O(1).
- **Move assign** `T& operator=(T&&) noexcept` — guard → release own → steal → null source → return `*this`.
- **Null the source** or double-free.
- **Always `noexcept`** or containers copy instead of moving.
- Auto-generated for Rule-of-0; hand-write for raw-resource classes.

### 🔍 Why `noexcept` decides move-vs-copy in `std::vector` (the deep why)
When a `vector` grows past capacity and **reallocates**, it relocates all elements from the old buffer to a bigger new one — choosing **move** (fast) or **copy** (slow). The chain of logic:

1. **`vector` makes a promise** — the **strong exception guarantee**: if a reallocation throws partway, your vector is left **exactly as it was**. Nothing lost.
2. **Moving is destructive** — a move empties the source. So mid-relocation, after moving elements 0–4, those old slots are **gutted** (data stolen into the new buffer).
3. **The disaster:** if element 5's move then **throws**, `vector` can't undo it — elements 0–4 are already destructively moved out, the old buffer is wrecked, no way back to the original state. **Promise broken.**
4. **Copy doesn't have this problem** — copying leaves the source untouched. If a copy throws midway, `vector` discards the half-built new buffer and keeps the intact old one. **Promise kept.**
5. **So `vector` decides by `noexcept`:**
   - Move is **`noexcept`** → "can't throw" → no half-moved disaster possible → **uses move** ⚡
   - Move is **not** `noexcept` → "might throw" → too risky → **falls back to copy** 🐢

> **The why:** a move that might throw could leave `vector` in a broken half-moved state it can't roll back, violating the strong guarantee. If your move isn't `noexcept`, `vector` silently **copies** — losing all move performance. A (genuinely non-throwing) `noexcept` move unlocks fast reallocation.

> **What does "throws" even mean?** (full topic: #13) "Throwing" = raising an **exception** — a structured runtime failure that jumps out to a `catch` block (`throw std::runtime_error("...")`). It's an error, but a catchable one, not a crash.
> - A **proper move** (steal a pointer, null the source) does nothing that can fail → **never throws** → correctly `noexcept`.
> - A move that **allocates** (`data = new char[...]`) *can* throw `std::bad_alloc` if memory runs out → **might throw** → not `noexcept` → `vector` copies to be safe.
>
> So "if the move throws" = "if it fails partway (e.g. a `new` inside it runs out of memory)." Pointer-steal moves have no such risk → mark them `noexcept`.

---

## Sub-topic 5 — When a Move Actually Happens vs a Silent Copy

### The trap
Move is an **optimization the compiler applies when conditions are met.** Miss a condition and code that *looks* like a move silently does an O(n) **copy** — a performance bug you can't see (same syntax, no warning, correct but slow).

### When a move HAPPENS ✅
```cpp
std::string s = make();          // move — make() returns a temporary (rvalue)
v.push_back(std::string("hi"));  // move — argument is a temporary
std::string b = std::move(a);    // move — you cast lvalue a to an rvalue
```
Move happens when the source is an **rvalue** (naturally or via `std::move`) **and** a move op exists.

### When it SILENTLY COPIES ⚠️
**Trap 1 — named lvalue (forgot `std::move`):**
```cpp
std::string b = a;    // COPY — a is an lvalue
```
**Trap 2 — `const` source (the sneaky one):**
```cpp
const std::string a = "hello";
std::string b = std::move(a);   // ⚠️ COPY! move ctor takes T&& (non-const) → can't bind
                                //    a const rvalue; copy ctor (const T&) binds → copy wins
```
You **can't move out of a `const` object** — moving *modifies* the source (nulls it), which const forbids. `std::move` on a const quietly copies.

**Trap 3 — no move op exists** (never written, or suppressed by a user copy ctor/destructor) → `std::move` has nothing to call → copy.

**Trap 4 — move not `noexcept`, during vector reallocation** (sub-topic 4) → copies.

### ⚠️ Gotcha: never `std::move` a return value
```cpp
std::string makeString() {
    std::string s = "hello";
    return std::move(s);    // ❌ WRONG — disables copy elision, forces a move
    return s;               // ✅ RIGHT — compiler eliminates the move ENTIRELY (RVO/NRVO)
}
```
`return s;` lets the compiler construct `s` **directly in the caller** — zero moves, zero copies. `std::move(s)` **blocks** that, forcing an actual move where you could've had *nothing*. So it turns "no work" into "a move." (details: sub-topic 6)

#### "But move is O(1) — how is plain return *faster*?"
Because plain `return` can be **O(0) — zero operations.** Zero beats O(1).
- A move is *cheap* but still **real work**: copy 3 fields + null the source. It executes.
- **RVO** does **no work**: it builds `s` **directly in the caller's memory** from the start, so `s` and `result` are the *same object* — there's nothing to transfer.
```
return std::move(s):  [build s here] ──move──► [result there]   = 1 move  (O(1))
return s; (RVO):      s and result are the SAME memory          = 0 moves (O(0))
```
| | Work |
|---|---|
| `return std::move(s)` | **1 move** (real, executes) |
| `return s;` (RVO) | **0** — operation eliminated |

**Why `std::move` breaks it:** RVO needs you to return the **object itself** (`return s;`). `return std::move(s)` returns an *expression* (a cast), not the plain object → compiler can't build it directly in the destination → **RVO disabled** → a real move is now required. You asked for a move, so you got one, instead of the *zero* you'd have had.

> Move = "quickly carry the box to the next room." RVO = "the box was built in the next room already — no carrying." `std::move` insists on carrying a box that never needed to move. **You can't beat 'no work' → never `std::move` a return.**

### The checklist — "will this move?"
1. Source an **rvalue**? (named lvalue → copy)
2. Source **non-const**? (const → copy)
3. Type **has a move op**? (no → copy)
4. (vector realloc) Move **`noexcept`**? (no → copy)
All yes → move. Any no → silent copy.

### Summary
- Move vs copy is invisible at the call site → silent copies are unseen perf bugs.
- **Move** when source is an rvalue + a move op exists.
- **Silent copy** when: named lvalue (forgot `std::move`), `const` source, no move op, or non-`noexcept` move in vector realloc.
- ⚠️ **Never `std::move` a return** — it disables the even-better copy elision.

---

## Sub-topic 6 — Copy Elision / RVO / NRVO

### Simple: "elision" = omission
**Copy elision** = the compiler is allowed to **omit** a copy/move entirely — constructing the object **directly in its final destination** instead of build-then-transfer. Result: **zero operations**.

### The mechanism: the hidden return slot
The compiler compiles a function **once** (many callers), so it can't hard-code the destination. It uses a hidden pointer via the calling convention:
```cpp
// You write:
std::string makeString() { std::string s = "hello"; return s; }
std::string result = makeString();

// Compiler effectively does:
void makeString(std::string* slot) {     // caller passes WHERE to build the result
    new (slot) std::string("hello");      // construct directly into the destination
}
std::string result;        // reserve memory
makeString(&result);       // pass its address → s is built AS result
```
- Caller allocates `result`, passes its **address** (hidden arg).
- Function constructs the local **at that address** → `s` *is* `result`, same memory from birth → **zero moves/copies**.
- Works across any callers/files because it's a *convention* (pass the destination pointer), not line-by-line linking.

### RVO vs NRVO
```cpp
std::string make() { return std::string("hi"); }        // RVO  — unnamed temporary
std::string make() { std::string s = "hi"; return s; }  // NRVO — Named local
```
Same idea; difference is whether the returned object has a **name**. NRVO is harder for the compiler (named var may be used in complex ways).

### Why NRVO is harder (why it's a separate concept)
A **named** local is trickier to elide than an anonymous temporary, because it can have **multiple return paths** returning *different* named variables:
```cpp
std::string make(bool flag) {
    std::string s = "hello";
    std::string t = "world";
    if (flag) return s;   // sometimes s...
    else      return t;   // ...sometimes t
}
```
Which one should the compiler build directly in the destination slot — `s` or `t`? It can't build both there → NRVO may be impossible here. An **unnamed temporary** (RVO) never has this ambiguity: exactly one object, obviously the return value. That extra difficulty is why the standard treats them differently.

### 🔑 The C++17 rule: RVO is MANDATORY, NRVO is not
- **RVO (unnamed temporary)** → **mandatory since C++17.** Elision guaranteed; the temporary and destination *are the same object* — not even a conceptual move.
- **NRVO (named local)** → **optional**, even in C++17. Allowed but not required; all major compilers do it. If it can't, it falls back to a **move** (never a copy). So NRVO = "elision if possible, else move."

### Why `std::move` on return is harmful (full circle)
```cpp
return s;              // ✅ enables NRVO → zero ops (or a move if NRVO fails)
return std::move(s);   // ❌ returns an EXPRESSION, not the named object → NRVO can't apply → forced move
```
`std::move` caps your best case at "a move" and removes the "zero" option.
> **Rule:** return the named local plainly — RVO/NRVO gives zero-cost when possible, a move as fallback. You can't beat it by hand.

### Real-world impact
Returning big objects by value is **cheap and idiomatic** in modern C++:
```cpp
std::vector<int> buildData() {
    std::vector<int> v;   /* fill a million elements */   return v;   // NRVO → zero transfer
}
std::vector<int> data = buildData();   // cheap despite "return by value"
```
Copy elision + move semantics killed the old fear of returning big objects by value.

### Summary
- **Copy elision** = omit the copy/move, build the object directly in its destination (zero ops).
- **Mechanism:** caller passes a **hidden pointer** to the destination; function builds the local there.
- **RVO** (unnamed temporary) = **mandatory since C++17** (guaranteed zero-cost).
- **NRVO** (named local) = **optional** (compilers do it; falls back to a **move**, never copy).
- ⚠️ **Never `std::move` a return** — blocks NRVO.
- Return big objects by value freely.

---

## Sub-topic 7 — `std::forward` & Perfect Forwarding (lighter)

### The problem
A wrapper function that passes its argument to another function loses the argument's lvalue/rvalue-ness:
```cpp
void target(const std::string& s);   // lvalue version
void target(std::string&& s);        // rvalue version (can steal)

template <typename T>
void wrapper(T arg) { target(arg); }   // ⚠️ inside, arg has a NAME → it's an lvalue, always
```
```cpp
wrapper(std::string("temp"));   // caller passed an RVALUE...
                               // ...but arg is an lvalue inside → target(const&) → move opportunity LOST
```
Forwarding through a wrapper **silently degrades rvalues into lvalues** (sub-topic 2's twist).

### The WHY
Wrappers are everywhere (`make_unique`, `make_shared`, `emplace_back`, factories). If each degraded rvalues into lvalues, move semantics would die through every layer of indirection. You need to **forward an argument preserving exactly what it was** → perfect forwarding.

### The fix: universal reference `T&&` + `std::forward`
```cpp
template <typename T>
void wrapper(T&& arg) {                 // T&& in a TEMPLATE = "universal/forwarding reference"
    target(std::forward<T>(arg));       // forward, preserving lvalue/rvalue-ness
}
```
- **`T&&` in a template** (with deduction) is *not* a plain rvalue ref — it binds to **both** lvalues and rvalues and *remembers which*.
- **`std::forward<T>(arg)`** = "cast back to rvalue *only if* the caller originally passed an rvalue; else keep it an lvalue." Restores what was lost:
```cpp
wrapper(s);                     // lvalue in → forward keeps lvalue → target(const&) ✅
wrapper(std::string("temp"));   // rvalue in → forward restores rvalue → target(&&)  ✅
```

### `std::move` vs `std::forward` (classic interview Q)
| | `std::move` | `std::forward` |
|---|---|---|
| Casts to rvalue… | **unconditionally** | **conditionally** (rvalue only if original was rvalue) |
| Meaning | "I'm done — steal it" | "pass along as whatever it originally was" |
| Where | a value you own & want to move | inside a wrapper with universal ref `T&&` |
> `std::move` = *always* treat as rvalue. `std::forward` = treat as rvalue *only if it was one*.

### Summary
- **Problem:** forwarding through a wrapper loses lvalue/rvalue-ness (named param = lvalue) → moves vanish.
- **Perfect forwarding:** universal ref `T&&` (template) captures the category; `std::forward<T>(arg)` restores it.
- **forward** = conditional cast to rvalue; **move** = unconditional.
- Used by `make_unique`/`make_shared`/`emplace_back`/factories. (Deep mechanics — reference collapsing — are advanced; know the what + why.)

---

## Code examples in this folder

| File | Demonstrates |
|------|--------------|
| `../03-smart-pointers/shared_ptr_impl.cpp` | The hand-written `SharedPtr` completed to **Rule of 5** — move ctor (O(1) pointer-steal + null source) and move assignment (self-guard → release → steal → null → `return *this`), plus moved-from destructor null-guard |

**Key move-ctor/assignment lessons from finishing `SharedPtr`:**
- Move = **steal pointers + null the source** (no refcount change — moving keeps the same owner count, unlike copy which does an atomic ++).
- **`std::move` on a raw pointer is pointless** — raw pointers/`int`/`bool` have no move semantics; plain assignment already "steals" the address. Reserve `std::move` for resource-owning types.
- **Null the source** so its destructor is a harmless no-op (else double-free).
- **Guard the destructor** against a moved-from (null) state, or destroying a hollow object crashes.
- Precedence trap: `*p -= 1` / `(*p)--` decrements the value; `*p--` accidentally decrements the **pointer**.
