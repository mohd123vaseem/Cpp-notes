# Topic 3 — Smart Pointers

Tier 1 #3 — one of the **most-asked** C++ interview topics. Smart pointers are just **RAII wrappers around raw pointers**: they own heap memory and free it automatically in their destructor. This is the payoff for Topics 1 & 2 (RAII + pointers + object model) — it makes the four memory bugs (leak, dangling, double-free, use-after-free) structurally impossible.

## Why this topic matters

Modern C++ almost never uses raw `new`/`delete`. Smart pointers express **ownership** in the type system: who owns a resource, whether ownership is exclusive or shared, and who's just observing. Interviewers use this to see if you've moved beyond manual memory management — and "write a `shared_ptr` from scratch" is a classic whiteboard test.

---

## Progress tracker (sub-topics)

| # | Sub-topic | The classic question | Status |
|---|-----------|----------------------|--------|
| 1 | **`unique_ptr`** — sole ownership, move-only, zero-overhead | "Why is `unique_ptr` zero-cost?" | ✅ Done |
| 2 | **`shared_ptr`** — reference counting, the control block, atomic refcount + cost | "How does `shared_ptr`'s refcount work?" | ✅ Done |
| 3 | **`weak_ptr`** — breaking reference cycles | "What's a `shared_ptr` cycle and how does `weak_ptr` fix it?" | ✅ Done |
| 4 | **`make_unique` / `make_shared`** — why preferred | "Why prefer `make_shared` over `new`?" | ⬜ Pending |
| 5 | **When to use which** — the ownership decision | "unique vs shared vs weak — when each?" | ⬜ Pending |
| 6 | ⭐ **Write a basic `shared_ptr` from scratch** | "Sketch a minimal `shared_ptr`." | ⬜ Pending |

> Sub-topic 6 is the deep one — the highest-yield piece where interviewers separate people who *use* smart pointers from people who *understand* them.

> **Browser edge (bonus):** Chromium's `scoped_refptr`/`base::RefCounted` (intrusive refcount) and `base::WeakPtr` (sequence-checked) are great compare-and-contrast points vs `shared_ptr`/`weak_ptr` — we'll note these as we go.

---

## Sub-topic 1 — `unique_ptr`

### Core idea
`std::unique_ptr` owns a heap object **exclusively** — exactly **one** owner at a time. When it goes out of scope, it auto-`delete`s what it owns. RAII applied to a single pointer.
```cpp
{
    std::unique_ptr<int> p = std::make_unique<int>(42);  // owns a heap int
    std::cout << *p;   // use like a normal pointer → 42
}   // p out of scope → destructor deletes the int automatically
```

### Sole ownership → can't copy, can move
Copying would create two owners → both delete → double-free. So copy is **forbidden**; ownership is **moved** instead (source becomes `nullptr`):
```cpp
std::unique_ptr<int> a = std::make_unique<int>(42);
std::unique_ptr<int> b = a;             // ❌ compile error — can't copy
std::unique_ptr<int> c = std::move(a);  // ✅ ownership moved a → c; a is now nullptr
```
This is why `unique_ptr` is **move-only**.

### Zero-overhead (the interview headline)
A `unique_ptr<T>` is as cheap as a raw `T*`:
- **Same size** — one pointer inside (8 bytes on 64-bit), no bookkeeping.
- **Same speed** — `*p` / `p->` compile to identical machine code as a raw pointer.
- Only "work" is the auto-`delete` in the destructor — which you'd write by hand anyway.

→ Automatic, leak-proof cleanup for **free**. Default to `unique_ptr` for owning a heap object.

### Common uses
- RAII member owning a heap resource (no manual destructor).
- **Factory functions** returning `unique_ptr<T>` ("caller now owns this").
- Pimpl idiom; polymorphic ownership (base pointer to derived object).

### Reference / interview follow-ups
- **`.get()`** — get the raw pointer without giving up ownership (for passing to APIs that take `T*`). Don't `delete` it.
- **`.release()`** — give up ownership, return the raw pointer (now *you* must delete it).
- **`.reset(p)`** — delete the current object, optionally take ownership of a new one.
- **Custom deleter** — `unique_ptr<FILE, decltype(&fclose)>` etc., for resources not freed by `delete` (files, C handles). Note: a custom deleter can make it bigger than a raw pointer.
- **Arrays** — `std::unique_ptr<int[]>` uses `delete[]` correctly (but prefer `std::vector`).

### Summary
- Sole/exclusive ownership; auto-`delete`d in destructor.
- Can't copy (two owners → double-free); can move (transfers, source → `nullptr`).
- Zero-overhead: same size & speed as a raw pointer.
- Default choice for owning a single heap object.

---

## Sub-topic 2 — `shared_ptr`

### Core idea
`std::shared_ptr` allows **shared ownership** — multiple `shared_ptr`s own the **same** object. It stays alive while *at least one* owner exists, and is deleted when the **last** one goes away.
```cpp
std::shared_ptr<int> a = std::make_shared<int>(42);  // count = 1
{
    std::shared_ptr<int> b = a;   // COPY allowed! both own it → count = 2
}   // b dies → count = 1 (still alive)
// a dies → count = 0 → object deleted
```
Key difference from `unique_ptr`: **you *can* copy a `shared_ptr`** — each copy is another co-owner.

### How it works: reference counting + the control block
A `shared_ptr` holds two pointers — one to your object, one to a separate heap **control block**:
```
   shared_ptr a          shared_ptr b
   [obj ptr][ctrl ptr]   [obj ptr][ctrl ptr]
             │                     │
             └──────► Control Block ◄────┘        ┌─────────┐
                      strong = 2                   │ the int │
                      weak   = 0                   │   42    │
                                                   └─────────┘
```
The control block holds:
- **strong count** — how many `shared_ptr`s own the object. Hits **0** → object destroyed.
- **weak count** — how many `weak_ptr`s observe it. Both counts 0 → control block itself freed.

Mechanics: copy → strong++; destroy/reassign → strong--; reaches 0 → delete object.

### The atomic refcount — and its cost
The refcount is **atomic** (thread-safe), because two threads might copy/destroy `shared_ptr`s to the same object at once — a torn count would leak or double-free. But atomic ops are **more expensive** than plain int ops:
> Copying a `shared_ptr` isn't free — every copy/destroy is an **atomic** inc/dec. In hot paths, passing `shared_ptr` by value everywhere can hurt performance. Prefer `const&`, or use `unique_ptr`/raw refs when you don't need shared ownership.

#### How atomic prevents double-free (the mechanism)
Each thread owns its **own copy** of the `shared_ptr`; they all share **one** atomic count. Destroying a copy does an atomic decrement **and reads the result**, as one indivisible step:
```cpp
if (--strong_count == 0) delete object;   // only the thread that hits 0 deletes
```
With count = 2 and two threads finishing at once: atomicity forces an order → one reads `1` (doesn't delete), the other reads `0` (deletes). **Exactly one thread can ever see 0**, so:
- no **double-free** (only one "reached zero" event),
- no **leak** (no lost/torn decrements).

Key insight: **the count decides who deletes — not a `nullptr` check.** A thread doesn't inspect the pointer; it acts on the *result of its own atomic decrement*.

⚠️ This protects the **count/lifetime only**. Two threads mutating the **same** `shared_ptr` *instance* (e.g. one reassigns `a` while another reads it) is still a **data race**. Safe rule: **give each thread its own copy**, never share one instance across threads without a lock.

### ⚠️ The critical thread-safety boundary (common trap)
> The **control block (refcount) is thread-safe. The managed object is NOT.**
- ✅ Multiple threads can **copy/destroy** `shared_ptr`s to the same object — atomic count handles it.
- ❌ Multiple threads **reading/writing the pointed-to object** concurrently = data race; `shared_ptr` does nothing to protect that. You still need your own mutex/atomic for the object's *contents*.

So `shared_ptr` gives thread-safe *lifetime management*, not thread-safe *data access*.

### Cost vs `unique_ptr`
| | `unique_ptr` | `shared_ptr` |
|---|---|---|
| Ownership | exclusive (one) | shared (many) |
| Copyable? | ❌ move-only | ✅ copyable |
| Size | 1 pointer (8 bytes) | **2 pointers** (object + control block) |
| Overhead | zero | control-block alloc + **atomic** refcount ops |

Use `shared_ptr` **only when ownership is truly shared** — defaulting to it when `unique_ptr` would do is a common junior mistake.

### Summary
- Shared ownership; object lives until the **last** owner is gone.
- Copyable — copy bumps the count, destroy drops it, 0 → deleted.
- Points to a **control block** holding **strong** (owners) + **weak** (observers) counts.
- Refcount is **atomic** (thread-safe lifetime) but costs performance; the **managed object is not protected** (sync its data yourself).
- Heavier than `unique_ptr` — use only for genuinely shared ownership.

---

## Sub-topic 3 — `weak_ptr`

> **Quick note on `make_shared` (covered fully in sub-topic 4):** `std::make_shared<T>(args...)` creates a `T` on the heap and returns a `shared_ptr<T>` owning it — the preferred shorthand for `std::shared_ptr<T>(new T(args...))`. And `auto x = ...` just lets the compiler deduce the type. So `auto a = std::make_shared<Node>();` = "make a `Node` on the heap and store a `shared_ptr` to it in `a`."

### The problem it solves: reference cycles
If two objects hold `shared_ptr`s to **each other**, their counts never reach 0 → they **leak forever**:
```cpp
struct Node {
    std::shared_ptr<Node> partner;
    ~Node() { std::cout << "Node destroyed\n"; }
};
{
    auto a = std::make_shared<Node>();   // a's count = 1
    auto b = std::make_shared<Node>();   // b's count = 1
    a->partner = b;   // b's count = 2
    b->partner = a;   // a's count = 2
}   // local a dies → its Node count 2→1 (b->partner still owns it)
    // local b dies → its Node count 2→1 (a->partner still owns it)
    // both stuck at 1 → neither destructor runs → LEAK ("Node destroyed" never prints)
```
Nothing outside can reach them, yet they keep each other alive. Refcounting can't detect this; a GC could.

### The fix: `weak_ptr` — a non-owning observer
A `std::weak_ptr` points to a `shared_ptr`-managed object but **does not own it** — it doesn't bump the strong count, doesn't keep the object alive. Make **one** link weak to break the cycle:
```cpp
struct Node {
    std::weak_ptr<Node> partner;   // weak — does NOT keep the other alive
    ~Node() { std::cout << "Node destroyed\n"; }
};
{
    auto a = std::make_shared<Node>();
    auto b = std::make_shared<Node>();
    a->partner = b;   // weak — b's strong count STAYS 1
    b->partner = a;   // weak — a's strong count STAYS 1
}   // local a dies → count 1→0 → destroyed ✅
    // local b dies → count 1→0 → destroyed ✅  (cycle broken)
```

### Using a `weak_ptr`: you must `lock()` it
A `weak_ptr` doesn't own the object, so the object **might already be gone**. You can't dereference it directly — call `lock()` to safely get a `shared_ptr` (or `nullptr`):
```cpp
std::weak_ptr<Node> w = a;
if (auto sp = w.lock()) {   // alive → sp is a valid shared_ptr (count bumped while used)
    // use sp
} else {
    // object already destroyed → lock() returned nullptr (no dangling, no crash)
}
```
`w.expired()` also tells you if it's gone, but `lock()` is preferred — check-then-use would race; `lock()` is atomic.

### The weak count (ties back to the control block)
The control block holds two counts:
- **strong count** — owners (`shared_ptr`). Hits 0 → **object** destroyed.
- **weak count** — observers (`weak_ptr`). strong=0 **and** weak=0 → **control block** freed.

So after the object is gone (strong=0), the control block survives while `weak_ptr`s remain — that's how `lock()` can still safely answer "it's gone" (returns nullptr). Last `weak_ptr` dies → control block freed.

### Common uses (concrete)

**1. Breaking cycles — parent ↔ child.** E.g. a GUI tree: a window owns its buttons, each button refers back to its window.
```cpp
struct Window { std::vector<std::shared_ptr<Button>> buttons; };  // OWNS buttons
struct Button { std::weak_ptr<Window> parent; };                  // just refers back
```
Owner→owned = `shared_ptr`; back-reference = `weak_ptr`. If both were `shared_ptr` → cycle → leak.

**2. Caches — don't keep things alive just by caching.** E.g. an image cache that shouldn't hold an image if nothing else uses it:
```cpp
std::map<std::string, std::weak_ptr<Image>> cache;   // observes, doesn't own
std::shared_ptr<Image> getImage(const std::string& f) {
    if (auto img = cache[f].lock()) return img;       // still in use → reuse
    auto img = loadImage(f);
    cache[f] = img;                                   // cache observes (weak)
    return img;
}
```
With `shared_ptr` the cache would keep every image forever; with `weak_ptr` an image lives only while actually used.

**3. Callbacks that shouldn't extend lifetime.** The target might be destroyed before the callback fires:
```cpp
std::weak_ptr<Listener> w = listener;
onDataReady([w]{ if (auto l = w.lock()) l->onData(); });  // fire only if still alive
```
A captured `shared_ptr` would keep the listener alive (maybe forever); `weak_ptr` observes and safely skips if it died. *(Chromium's `base::WeakPtr` is exactly this — bonus-track edge.)*

> **Pattern behind all three:** use `weak_ptr` when you need to **refer to** an object but **not own it** (back-pointer, cache entry, callback target) — so you avoid cycles/leaks and can detect if it's gone via `lock()`.

### Summary
- **Reference cycle** = two `shared_ptr`s owning each other → counts never 0 → **leak**.
- `weak_ptr` = non-owning observer; doesn't bump strong count.
- Fix a cycle by making **one** link weak.
- Can't dereference — call **`lock()`** (valid `shared_ptr` if alive, `nullptr` if gone).
- The **weak count** keeps the *control block* alive (not the object) so liveness checks stay safe.

---

## Code examples in this folder

| File | Demonstrates |
|------|--------------|
| `threads_shared_ptr.cpp` | Safe pattern (each thread its own copy, atomic refcount) vs data race (threads sharing one instance) |
