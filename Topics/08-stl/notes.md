# Topic 8 — STL: Containers, Complexity, Iterator Invalidation

Tier 2 #8 (~3 days). You **use** STL daily via DSA, so the goal here is the **interview framing** you may lack: *internals* (why each container has its complexity), *iterator invalidation* (the #1 gotcha), and **implementing containers from scratch** (a verified real ask).

## Why this topic matters

- *"map vs unordered_map — when and why?"* and *"when does a vector iterator get invalidated?"* are **near-guaranteed** questions.
- Interviewers notice **clean, idiomatic STL** in coding rounds (right container, `reserve`, erase-remove).
- **Web-verified:** you can be asked to **implement a container** — e.g. **Design HashMap** ([LeetCode #706](https://leetcode.com/problems/design-hashmap/)) with buckets + collision handling, or a dynamic-array `vector`. It's the same "write it from scratch" muscle as `SharedPtr`.

---

## Progress tracker (sub-topics)

### Part A — The containers (usage → internals → complexity)
| # | Sub-topic | The classic question | Status |
|---|-----------|----------------------|--------|
| 1 | **Sequence containers** — `vector`, `deque`, `list`, `array` | "vector vs list vs deque?" | ✅ Done |
| 2 | **Associative** — `map`/`set` (red-black tree) vs `unordered_map`/`set` (hash) | "map vs unordered_map — when/why?" | ✅ Done |
| 3 | **Container adaptors** — `stack`, `queue`, `priority_queue` | "how is priority_queue implemented?" | ✅ Done |

### Part B — The interview gotchas
| # | Sub-topic | The classic question | Status |
|---|-----------|----------------------|--------|
| 4 | ⭐ **Iterator invalidation** (which ops invalidate iterators) | "when does a vector iterator invalidate?" | ✅ Done |
| 5 | **Iterator categories** (input/forward/bidirectional/random) | "what iterator does a list give?" | ✅ Done |
| 6 | **Common algorithms** — `sort`, `find`, `lower_bound`, `accumulate`, **erase-remove idiom** | "how do you remove elements from a vector?" | ✅ Done |

### Part C — Implement from scratch (verified real ask)
| # | Sub-topic | The classic question | Status |
|---|-----------|----------------------|--------|
| 7 | ⭐ **Implement a dynamic-array `vector`** (grow/reallocate, amortized O(1) push_back) | "how does vector grow?" / "implement vector" | ⬜ Pending |
| 8 | ⭐ **Design HashMap** (buckets + separate chaining + resizing) | "implement a hashmap" | ⬜ Pending |

---

## Sub-topic 1 — Sequence Containers: `vector`, `deque`, `list`, `array`

Store elements in a linear sequence. Focus: internals + when-to-use (usage you already know).

### `std::vector` — dynamic array (the default)
Contiguous, growable array — elements **back-to-back in memory**.
- Index access **O(1)**; `push_back` **amortized O(1)** (occasionally reallocates); insert/erase **middle O(n)** (shift the rest).
- **Cache-friendly** (contiguous → spatial locality) → often fastest in practice.
> **Default to `vector`** (~90% of cases). Only weakness: middle insert/erase.

### `std::deque` — double-ended queue
Fast insert/remove at **both ends**.
- `push_front` AND `push_back` **O(1)**; index access O(1) (slightly slower than vector).
- Internally **chunks** (fixed-size blocks linked), not one contiguous block → less cache-friendly.
> Use when you need fast **front** insertion too. Backs `stack`/`queue`.

### `std::list` — doubly-linked list
Each element a node with prev/next pointers.
- insert/erase **anywhere O(1)** — *if you hold an iterator there* (relink pointers, no shift).
- Index access **O(n)** — no random access (`list[5]` doesn't exist).
- Scattered nodes → **poor cache locality** + 2 pointers/element overhead.
> Rare. `vector` often **beats** `list` even for middle insertion — the O(n) shift on contiguous memory is cache-friendly, while `list`'s pointer-chasing thrashes the cache. **"O(1) insert" on paper loses to cache reality often.**

### `std::array` — fixed-size array
Wrapper over a C array; **size fixed at compile time**, lives on the **stack**, zero overhead + STL niceties (`.size()`, `.at()`).
> Use for compile-time fixed sizes — safe replacement for `int arr[5]`.

### Decision guide
| Container | Memory | Index access | Insert/erase | Use when |
|---|---|---|---|---|
| **`vector`** | contiguous | O(1) | O(1) back, O(n) middle | **default** — access/iteration heavy |
| **`deque`** | chunked | O(1) | O(1) both ends, O(n) middle | fast **front AND back** insert |
| **`list`** | scattered nodes | **O(n)** | O(1) anywhere (w/ iterator) | *lots* of middle insert/erase + hold iterators (rare) |
| **`array`** | contiguous, stack | O(1) | none (fixed) | **compile-time fixed** size |

### 🔑 Key model: contiguous vs node-based
- **array-based** (`vector`/`deque`/`array`) → contiguous(ish) → **fast access, cache-friendly**, but shift to insert mid.
- **node-based** (`list`) → scattered pointers → **fast relink**, but no random access, cache-hostile.
> Interview one-liner: "Default `vector` (cache-friendly, O(1) access); `deque` for both-ends insert; `list` only for frequent middle insert where I hold iterators — though `vector` often wins on cache locality; `array` for fixed sizes." (Shows you get **cache reality**, not just Big-O.)

---

## Sub-topic 2 — `map`/`set` vs `unordered_map`/`set` ⭐ (the most-asked STL question)

`map` = key→value; `set` = keys only (same tree internals). `unordered_` versions use a hash table. Real comparison = **tree vs hash**.

### `map`/`set` — red-black tree (self-balancing BST)
Keeps itself balanced (height ~log n, never degrades to a chain).
- **All ops O(log n)** (insert/erase/find walk the height).
- **Keys kept SORTED** → in-order traversal is sorted for free.
- Supports **ordered/range queries**: min/max, iterate in order, `lower_bound`/`upper_bound`, next key ≥ X.
```cpp
std::map<int,string> m; m[3]="c"; m[1]="a"; m[2]="b";
for (auto& [k,v] : m) cout << k;   // 1 2 3 — SORTED automatically
```

### `unordered_map`/`unordered_set` — hash table
Hashes the key → index into an array of **buckets**.
- **O(1) average** (hash straight to bucket); **O(n) worst** (many collisions → long bucket chain).
- **NO ordering** — iteration order is arbitrary (unrelated to sorted/insertion order). No range queries.
```cpp
std::unordered_map<int,string> m; m[3]="c"; m[1]="a"; m[2]="b";
for (auto& [k,v] : m) cout << k;   // some order — NOT sorted, unpredictable
```

### The core tradeoff
| | `map`/`set` (RB-tree) | `unordered_map`/`set` (hash) |
|---|---|---|
| find/insert/erase | **O(log n)** | **O(1) avg**, O(n) worst |
| Keys ordered? | ✅ sorted | ❌ arbitrary |
| Range/ordered queries | ✅ (`lower_bound`, min/max, in-order) | ❌ |
| Memory | less (nodes) | more (buckets + slack) |
| Worst-case | O(log n) **guaranteed** | O(n) if bad hashing |

### 🔑 When to use which
> **Default `unordered_map`** for plain key-value lookup (O(1) beats O(log n)). **Use `map`** when you need keys **sorted** or **ordered/range operations** (sorted iteration, range queries, smallest/largest, next key ≥ X).
>
> **Rule:** need ordering/range → `map`; just fast lookup → `unordered_map`.

### Interview nuances
1. **"O(1) avg" caveats:** bad hash → O(n); **rehashing** when over the **load factor** re-hashes everything (O(n) spike, like vector realloc) → `reserve()` to avoid mid-use.
2. **`map` = worst-case guarantee** — predictable O(log n), no spikes/hash-collision attacks → good for latency-sensitive/adversarial input.
3. **Custom keys:** `map` needs `operator<`; `unordered_map` needs a **hash function + `operator==`**.
4. **Never rely on `unordered_map` iteration order** (can change after a rehash).

### Summary
- `map`/`set` = RB-tree → O(log n), sorted, range queries.
- `unordered_map`/`set` = hash → O(1) avg (O(n) worst), unordered.
- Need sorted/range → `map`; fast lookup → `unordered_map`. Watch rehashing, custom-key requirements, worst-case guarantees.

---

## Sub-topic 3 — Container Adaptors: `stack`, `queue`, `priority_queue`

Not new containers — **adaptors**: thin wrappers that take an existing container and **restrict its interface** to enforce an access pattern (no iteration/indexing).

### `std::stack` — LIFO (Last In, First Out)
Add/remove at the **same end (top)**. Ops: `push`, `pop`, `top`, `empty`, `size`. Backed by **`deque`**.
```cpp
std::stack<int> s; s.push(1); s.push(2); s.push(3);
s.top();  // 3   s.pop();  // removes 3
```
Use: undo, DFS, expression parsing, reversing.

### `std::queue` — FIFO (First In, First Out)
Add at **back**, remove at **front**. Ops: `push`(back), `pop`(front), `front`, `back`. Backed by **`deque`** (needs fast insert both ends → why not `vector`).
```cpp
std::queue<int> q; q.push(1); q.push(2); q.push(3);
q.front();  // 1   q.pop();  // removes 1
```
Use: BFS, scheduling, producer-consumer.

### `std::priority_queue` — highest priority first (a heap)
Largest on top (**max-heap** default). Ops: `push`, `pop` (removes largest), `top`. Backed by a **`vector`** as a **binary heap**. `push`/`pop` **O(log n)**, `top` **O(1)**.
```cpp
std::priority_queue<int> pq; pq.push(3); pq.push(1); pq.push(5);
pq.top();  // 5   pq.pop();  // removes 5
// min-heap (smallest on top):
std::priority_queue<int, std::vector<int>, std::greater<int>> minHeap;
```
Use: Dijkstra, top-K, merge-K-lists.

### ⭐ How is `priority_queue` implemented?
A **binary heap stored in a `vector`**: a complete binary tree (parent ≥ children for max-heap) laid out flat in an array — node `i`'s children at `2i+1`, `2i+2`. `push`/`pop` sift up/down in **O(log n)**; `top` is O(1).

### Summary
| Adaptor | Discipline | Backed by | Complexity |
|---|---|---|---|
| **`stack`** | LIFO (top only) | `deque` | O(1) |
| **`queue`** | FIFO (front/back) | `deque` | O(1) |
| **`priority_queue`** | priority (max on top) | `vector` (heap) | push/pop O(log n), top O(1) |
- Adaptors restrict a container to an access pattern. `priority_queue` = **binary heap in a vector**; `std::greater` → min-heap.

---

## Sub-topic 4 — Iterator Invalidation ⭐ (the #1 STL gotcha)

An **iterator** = a generalized pointer to an element inside a container. **Iterator invalidation** = modifying a container leaves existing iterators/pointers/references **dangling** → using them is **UB**.

### Why it happens
An iterator points into the container's storage. If an operation **moves that storage**, the iterator points at an old/invalid location. Classic case — **`vector` reallocation**:
```cpp
std::vector<int> v = {1,2,3};
int* p = &v[0];
auto it = v.begin();
v.push_back(4);      // ⚠️ may REALLOCATE (grow → new block, copy, free old)
*p;  *it;            // ❌ DANGLING — point to the freed old buffer
```

### 💡 What "reallocation on grow" means (the foundation)

**The setup: a vector has TWO sizes.** A `vector` tracks two different numbers:
- **`size()`** = how many elements are actually in it right now.
- **`capacity()`** = how many elements it has room for in its currently-allocated memory block.

`capacity` is usually bigger than `size` — the vector grabs extra room ahead of time so it doesn't have to reallocate on every single `push_back`.
```cpp
std::vector<int> v;
v.push_back(1);
std::cout << v.size();      // 1  — one element
std::cout << v.capacity();  // maybe 1, 2, or more — room reserved
```

**The core problem: a vector is a CONTIGUOUS array.** Elements are stored back-to-back in one block of heap memory (that's what makes indexing O(1) and cache-friendly).
```
capacity = 4, size = 3:
 ┌───┬───┬───┬───┐
 │ 1 │ 2 │ 3 │   │   ← one contiguous heap block; room for 4, holding 3
 └───┴───┴───┴───┘
```
But that block has a **fixed size once allocated** — you can't just "extend" a heap block (the memory right after it might already be used). So what happens when the vector is full (`size == capacity`) and you `push_back` one more?

**Reallocation: what "grow" actually does.** When there's no capacity left, the vector can't grow the existing block, so it does a **reallocation** — 4 steps:
```
size = 4, capacity = 4  → FULL. push_back(5) triggers:
1. ALLOCATE a new, BIGGER block (usually 2× the capacity → room for 8)
2. COPY (or move) all existing elements from the old block to the new one
3. ADD the new element (5) to the new block
4. FREE the old block
```
```
Old block (capacity 4, full):        New block (capacity 8):
 ┌───┬───┬───┬───┐                    ┌───┬───┬───┬───┬───┬───┬───┬───┐
 │ 1 │ 2 │ 3 │ 4 │  ──copy all──►     │ 1 │ 2 │ 3 │ 4 │ 5 │   │   │   │
 └───┴───┴───┴───┘                    └───┴───┴───┴───┴───┴───┴───┴───┘
      ↑ then FREED                         ↑ everything lives here now, at a NEW address
```
**The whole array physically moved to a new memory address.** That's "reallocation on grow."

**Why this connects to iterator invalidation.** Your old iterator/pointer pointed into the old block — which just got freed in step 4 → dangling:
```cpp
std::vector<int> v = {1, 2, 3, 4};   // full, capacity 4
int* p = &v[0];                      // p points into the OLD block
v.push_back(5);                      // reallocation → old block freed, data moved
// p still points to the old (freed) address → DANGLING → using *p is UB
```
The elements didn't just change — they **relocated to a completely different address.**

**Why "amortized O(1)" — the doubling trick.** You might think "if every push_back copies everything, isn't it O(n)?" No — the vector **doubles** (grabs 2× capacity), so reallocations happen *rarely* (only at 1, 2, 4, 8, 16… elements); most push_backs just drop the element into existing free space (O(1)). Averaged out, the occasional O(n) copy spreads thin over many cheap O(1) inserts → **amortized O(1)**. (Growing by +1 each time → reallocate every push → O(n²) total. Doubling is what makes it efficient.)

**How to avoid reallocation: `reserve()`.** If you know roughly how many elements you'll add, `reserve()` pre-allocates capacity up front, so no reallocation happens mid-use:
```cpp
std::vector<int> v;
v.reserve(1000);        // allocate room for 1000 NOW
for (int i = 0; i < 1000; i++)
    v.push_back(i);     // no reallocations — capacity is already 1000
```
Common performance tip (and interview point): **`reserve` when you know the size** to avoid repeated reallocations *and* keep iterators/pointers stable. *(You build this logic yourself in sub-topic 7 — Implement a vector.)*

### The rules per container
- **`vector`** — realloc on grow → **ALL** iterators/ptrs/refs invalid; middle insert/erase → those **at and after** the point invalid. (Most invalidation-prone.)
- **`deque`** — ends: iterators invalid but refs to others often survive; middle: all invalid.
- **`list` / `forward_list`** — node-based → **stable**: insert/erase invalidates **only the erased element's** iterator; others survive.
- **`map`/`set`** (RB-tree) — node-based → insert invalidates **nothing**; erase invalidates only the **erased** element's iterator.
- **`unordered_*`** (hash) — erase → only erased element's iterator; **insert that triggers a rehash → ALL iterators invalid** (refs to elements survive).

### 🔑 The pattern (don't memorize — understand)
- **Array-based** (`vector`, `deque`) → insert/erase/grow **moves elements** → **lots of invalidation**.
- **Node-based** (`list`, `map`, `set`) → fixed nodes → **stable**, only the erased node's iterator dies.
- **Hash** (`unordered_*`) → stable **except on rehash** (rebuilds buckets → all invalid).

### The classic bug: erasing in a loop
```cpp
// ❌ BROKEN — erase() invalidates `it`, then ++it is UB
for (auto it = v.begin(); it != v.end(); ++it)
    if (*it % 2 == 0) v.erase(it);

// ✅ CORRECT — erase() RETURNS the next valid iterator
for (auto it = v.begin(); it != v.end(); )
    if (*it % 2 == 0) it = v.erase(it);   // use the returned iterator
    else              ++it;
```
(For `vector`/`string` bulk removal, use the **erase-remove idiom** — sub-topic 6.)

### Summary
- Modifying a container can make iterators/pointers/refs **dangle** → UB.
- ⭐ **`vector`:** grow-realloc invalidates **everything**; middle insert/erase from that point on.
- **`list`/`map`/`set`:** node-based → **stable** (only erased element's iterator).
- **`unordered_*`:** stable **except rehash** → all invalid.
- **Erase-in-loop fix:** `it = container.erase(it)` (erase returns the next valid iterator).

---

## Sub-topic 5 — Iterator Categories

### Simple: not all iterators can do the same things
Different containers allow different movement. C++ groups iterators into **5 categories** by **what operations they support** — i.e. "how powerful is this iterator: can it jump around, or only step one at a time?"

### The 5 categories (weakest → strongest, each adds to the previous)
1. **Input** — read-only, single-pass, forward: `*it`, `++it`, `==`/`!=`. Once past an element, can't go back (e.g. reading a stream).
2. **Output** — write-only, single-pass, forward: `*it = x`, `++it`. Mirror of input.
3. **Forward** — read/write, **multi-pass**, forward: can re-iterate the same elements. (`forward_list`, `unordered_*`.)
4. **Bidirectional** — forward **+ backward** (`--it`), one step at a time, **no jumping**. (`list`, `map`, `set`.)
5. **Random-access** — bidirectional **+ O(1) jump**: `it + 5`, `it[3]`, `it2 - it1`, `<`/`>`. (`vector`, `deque`, `array`, raw pointers.)
```
Input ──► Forward ──► Bidirectional ──► Random-access
(read,     (+ multi-    (+ backward       (+ O(1) jump,
 forward)   pass)        --it)             it+n, it[n])
Output ──► (write-only branch)
```

### Which container gives which
| Container | Category | `it + 5` / `c[i]`? |
|---|---|---|
| `vector`, `deque`, `array` | **Random-access** | ✅ jump anywhere |
| `list`, `map`, `set` | **Bidirectional** | ❌ step one at a time (both dirs) |
| `forward_list`, `unordered_map/set` | **Forward** | ❌ forward-only stepping |

### 🔑 Why it matters — explains the puzzles
1. **Why `list[5]` doesn't exist:** `list` = bidirectional → no random access → jumping to index 5 isn't O(1) (would walk 5 nodes) → STL provides no `operator[]`.
2. **Why `std::sort` won't compile on a `list`:** `std::sort` **requires random-access iterators** (quicksort/introsort jump around). `list` is only bidirectional → use its own `list.sort()` (merge sort).
   ```cpp
   std::sort(v.begin(), v.end());   // ✅ vector = random-access
   std::sort(l.begin(), l.end());   // ❌ list isn't random-access — won't compile
   l.sort();                        // ✅ list's own sort
   ```
3. **Each algorithm states a minimum category** (a contract): `find` needs input (works on all); `reverse` needs bidirectional; `sort` needs random-access.

> **Key insight:** the category **flows from the container's internals**. Contiguous array (`vector`) → `base + i*size` → random access. Linked list (`list`) → follow next/prev → bidirectional, no jump. Singly-linked (`forward_list`) → only next → forward-only. The data structure determines possible movements; the category encodes it.

### Summary
- 5 categories: **input/output → forward (multi-pass) → bidirectional (`--it`) → random-access (`it+n`, `it[i]`)**.
- `vector`/`deque`/`array` = random-access; `list`/`map`/`set` = bidirectional; `forward_list`/`unordered_*` = forward.
- Category flows from internals → explains no `list[5]`, no `std::sort` on `list` (use `list.sort()`), and each algorithm's minimum-iterator requirement.

---

## Sub-topic 6 — Common Algorithms + the Erase-Remove Idiom

### `<algorithm>` works on ITERATOR RANGES, not containers
Algorithms take `begin`/`end`, so the same one works on any container giving the right iterator category. Write once, use everywhere.

### Algorithms to know cold
```cpp
std::sort(v.begin(), v.end());                        // O(n log n), random-access
std::sort(v.begin(), v.end(), std::greater<int>());   // descending / custom comparator

auto it = std::find(v.begin(), v.end(), 42);          // linear O(n), returns iter or end()

// lower_bound/upper_bound: BINARY SEARCH on SORTED data, O(log n)
auto lb = std::lower_bound(v.begin(), v.end(), 42);   // first element >= 42  (upper_bound: > 42)

int sum = std::accumulate(v.begin(), v.end(), 0);     // <numeric>; fold. ⚠️ init type matters:
                                                      //   on vector<double>, 0 → int → truncates! use 0.0
```
Others: `count`, `min_element`/`max_element`, `reverse`, `unique` (removes *consecutive* dups), `all_of`/`any_of`, `transform`, `copy`.

> **What "idiom" means:** just "a standard, well-known way of doing something in a language" — a recognized pattern experienced programmers use and recognize instantly. Not a keyword or special C++ thing — plain English (like an idiom in a spoken language: a common phrase everyone knows). (RAII, pimpl are also "idioms.")

### ⭐ The erase-remove idiom
**Goal:** remove all elements equal to a value (or matching a condition) from a `vector`.

> **Why erase-in-loop is O(n²) but the idiom is O(n)** — same `erase`, different usage:
> ```cpp
> // ❌ O(n²) — erase ONE element per iteration; each erase shifts the rest (O(n)), done k times
> for (auto it = v.begin(); it != v.end(); )
>     if (*it == 2) it = v.erase(it);   // repeated single-element erase → O(n) each → O(n²)
>     else ++it;
>
> // ✅ O(n) — erase-remove idiom: ONE remove pass + ONE range erase (tail needs no shifting)
> v.erase(std::remove(v.begin(), v.end(), 2), v.end());
>
> // ✅ O(n) — C++20 std::erase(v, 2): just a fancy one-call shortcut for the SAME idiom above
> std::erase(v, 2);
> ```
> A single-element `erase(it)` shifts everything after it (O(n)); calling it in a loop → **O(n²)**. The idiom calls `erase` **once on the whole garbage range** (no shifting) after one O(n) `remove` pass → **O(n)**. `std::erase(v, 2)` (C++20) is just a cleaner way to write that same erase-remove combination — same O(n).

**Naive erase-in-loop is O(n²)** (each `erase` shifts the rest) and invalidation-prone.

**🔑 Key insight: `std::remove` does NOT remove.** Algorithms see only *iterators*, not the container — they **can't change its size**. `std::remove` **shifts the kept elements to the front** (overwriting unwanted ones) and **returns an iterator to the new logical end**; the tail is garbage, size **unchanged**.
```cpp
std::vector<int> v = {1, 2, 3, 2, 4, 2};
auto newEnd = std::remove(v.begin(), v.end(), 2);
// v: {1, 3, 4, ?, ?, ?}   size STILL 6; newEnd → after '4'
```
```
Before:  1  2  3  2  4  2      size 6
remove:  1  3  4  ?  ?  ?      size STILL 6, newEnd → after '4'
              kept   garbage
```
**The idiom — pair `remove` with the container's `erase`** to chop the garbage tail:
```cpp
v.erase(std::remove(v.begin(), v.end(), 2), v.end());   // → {1,3,4}, size 3
//       └── returns newEnd ──┘   └ erase newEnd..end
```
Inside-out: `remove` reorders + returns new end → `erase(newEnd, end())` deletes the tail. **O(n)**, correct.

**Condition-based:** `v.erase(std::remove_if(v.begin(), v.end(), [](int x){return x%2==0;}), v.end());`

**C++20 shortcut:** `std::erase(v, 2);` / `std::erase_if(v, pred);` (does the whole idiom in one call). Know both.

### 🔑 So how can `erase` resize but `std::remove` can't? (free algorithm vs member)
Different *kinds* of functions:
```
std::remove(v.begin(), v.end(), 2)
            └──────┬──────┘
            just two iterators — no `v`, no access to size/memory
            → can only rearrange ELEMENTS, can't resize

v.erase(newEnd, v.end())
└┬┘
 the container itself (this → v) — full access to size, capacity, buffer
 → CAN change the size, free memory
```
- **`std::remove`** = **free algorithm** (`<algorithm>`) — receives **only iterators**, no handle on the container → can rearrange elements but **can't change size**. (Stays generic: one algorithm for vector/deque/array/raw.)
- **`v.erase(...)`** = **member function** called **on the container** (`this` → `v`) → full access to its internals → **can resize / free memory.**
> Rule: `std::something(...)` = free algorithm, iterators only, **can't resize**; `container.something(...)` = member, has the container, **can resize**. Resizing is container-specific → only members do it. That's *why* removal needs both steps.

### ⭐ Interview gotcha: "what does `std::remove` do?"
> It **can't actually remove** — algorithms only see iterators, not the container, so they can't change its size. It **shifts kept elements to the front and returns the new logical end**; size unchanged, tail garbage. To truly remove, pair with the container's `erase` — the **erase-remove idiom**.

### Summary
- Algorithms operate on **iterator ranges** → container-agnostic.
- Know: `sort`, `find`, `lower_bound`/`upper_bound` (binary search on **sorted**), `accumulate` (watch init type).
- **`std::remove` doesn't remove** — shifts kept elements, returns new end, size unchanged.
- **Erase-remove idiom:** `v.erase(std::remove(...), v.end());` — O(n) correct bulk-remove. `remove_if` for conditions; C++20 `std::erase`/`erase_if`.

---

## Code examples in this folder

| File | Demonstrates |
|------|--------------|
| `iterator_invalidation.cpp` | `vector` reallocation dangling a pointer; the erase-in-loop bug + the `it = erase(it)` fix |
