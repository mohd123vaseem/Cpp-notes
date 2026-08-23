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
| 4 | ⭐ **Iterator invalidation** (which ops invalidate iterators) | "when does a vector iterator invalidate?" | ⬜ Pending |
| 5 | **Iterator categories** (input/forward/bidirectional/random) | "what iterator does a list give?" | ⬜ Pending |
| 6 | **Common algorithms** — `sort`, `find`, `lower_bound`, `accumulate`, **erase-remove idiom** | "how do you remove elements from a vector?" | ⬜ Pending |

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

## Code examples in this folder

| File | Demonstrates |
|------|--------------|
| _(added as we go)_ | |
