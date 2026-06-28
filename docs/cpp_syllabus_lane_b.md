# C++ Interview Syllabus — Systems & Database Roles (Lane B1/B2)

Prep syllabus for C++-specialist roles (systems/platform/GPU/embedded and database/infra companies). **Everything below is ordered by importance** — both the tiers and the topics within each tier. Start at the top and work down.

**Note on DSA:** This doc covers only the C++/systems portion. DSA remains the shared requirement for every lane and should run in parallel (roughly 70% DSA / 30% this early on, shifting toward 50/50 as interviews approach).

---

## Priority Legend

| Tier | Meaning | Action |
|------|---------|--------|
| **P0** | Asked in almost every interview. Your make-or-break core. | Master deeply. Be able to explain *and* implement. |
| **P1** | Frequently asked, strong differentiator. | Know well, practice explaining out loud. |
| **P2** | Comes up in deeper rounds or specific teams. | Solid working knowledge. |
| **P3** | Edge / nice-to-have. Separates strong from average. | Familiarity; mention naturally when relevant. |

---

## P0 — Master These First (highest ROI)

### 1. Memory & Object Model
The foundation everything else builds on.

- **RAII** — the central C++ idiom; understand why resource lifetime ties to object lifetime
- Object lifetime & storage duration (automatic, static, dynamic, thread-local)
- **Rule of 0 / 3 / 5**
- Stack vs heap (language level *and* OS level)
- Construction/destruction order

### 2. Move Semantics
The single most-probed "modern C++" topic.

- Rvalue references, `std::move`, `std::forward`
- Perfect forwarding
- When a move actually happens vs a silent copy
- Copy elision / RVO / NRVO

### 3. Smart Pointers (internals, not just usage)

- `unique_ptr` — why it's zero-overhead
- `shared_ptr` — control block, **atomic refcount** (and its cost), thread-safety boundaries
- `weak_ptr` — breaking reference cycles
- `make_shared` vs `make_unique` trade-offs
- **Practice writing a basic `shared_ptr` from scratch** — a classic ask

### 4. Concurrency & Memory Model
Your strongest leverage from browser work — go deepest here.

- `std::thread` lifecycle, joining/detaching
- Locking family: `lock_guard`, `unique_lock`, `scoped_lock`
- `condition_variable` + producer–consumer pattern
- `std::atomic` and the **memory model**: relaxed / acquire-release / seq_cst
- Data race vs race condition vs deadlock vs livelock
- `async` / `future` / `promise`
- False sharing & cache-line contention
- *(Lock-free structures are NOT required — that's HFT territory you're skipping)*

---

## P1 — Strong Differentiators

### 5. const Correctness
Near-guaranteed warm-up and a constant probe.

- `const` member functions; `const` objects
- Const refs / pointers — pointer-to-const vs const-pointer (read right-to-left)
- `mutable`
- How const-correctness propagates through an API

### 6. Polymorphism Internals

- **vtable / vptr** mechanics — explain virtual dispatch at the binary level
- Virtual destructors (why they matter)
- Object slicing
- Diamond problem & virtual inheritance
- `override` / `final`
- **pimpl idiom** (ubiquitous in large codebases like Chromium)

### 7. Exception Handling & Exception Safety
Routinely asked in systems rounds; ties directly into RAII.

- `try` / `catch` / `throw`, stack unwinding
- **Exception safety guarantees**: basic / strong / nothrow
- `noexcept` and why it matters (move ops, containers)
- How RAII guarantees cleanup during unwinding
- Why throwing from a destructor is dangerous

### 8. Casting & RTTI

- `static_cast` / `dynamic_cast` / `reinterpret_cast` / `const_cast` — when each applies
- RTTI and the runtime cost of `dynamic_cast`
- `typeid`
- Why C-style casts are discouraged

### 9. STL Internals & Complexity

- `vector` — growth, reallocation, capacity vs size
- `map` (red-black tree) vs `unordered_map` (buckets, load factor, hashing) — when to use which
- **Iterator invalidation rules** — classic gotcha
- Complexity of common containers/algorithms off the top of your head

### 10. Systems Fundamentals (OS layer)
Your multi-process browser experience maps directly here.

- Virtual memory & paging
- `malloc` / allocators (arenas, fragmentation), `mmap`
- Process vs thread, context switching, scheduling basics
- **IPC** — pipes, shared memory, sockets, message passing *(your home turf — Chromium's Mojo)*
- CPU caches — hierarchy, cache lines, spatial/temporal locality, what causes misses
- Futex underneath mutexes

---

## P2 — Deeper Rounds

### 11. Templates & Generic Programming

- Function & class templates
- Full and partial specialization
- Variadic templates
- Type traits, basic SFINAE
- **CRTP** (curiously recurring template pattern — common in systems code)
- Concepts (C++20)
- *Goal: read and reason about templated code comfortably — not metaprogramming mastery*

### 12. Memory Alignment & Padding
*Borderline P1 for cache-sensitive / GPU / embedded teams.* This is the practical lever behind cache behavior.

- Struct padding & member reordering to minimize size
- `alignas` / `alignof`
- Why alignment matters for performance (and correctness on some hardware)
- Connection to cache-line layout (ties back to #4 and #10)

### 13. Compilation & Linking Model
Systems roles expect you to understand the build pipeline at the language level.

- Translation units; the compile → link flow
- Declaration vs definition
- Internal vs external linkage
- **One Definition Rule (ODR)**
- Header guards / `#pragma once`; what `inline` really means for linkage

### 14. Modern C++ Fluency (C++11 → 20)
Signals you write C++ as it's used today.

- Lambdas & capture semantics
- `auto`, `constexpr`
- Structured bindings
- `std::optional` / `std::variant`
- `string_view`, `span`
- `std::function` and callbacks

### 15. `volatile` vs `atomic`
Common systems/embedded gotcha — and a frequent misconception.

- What `volatile` actually does (prevents certain compiler optimizations)
- Why `volatile` does **not** provide thread-safety or atomicity
- When `volatile` is legitimately needed (memory-mapped I/O, signal handlers)
- Use `std::atomic` for cross-thread visibility/ordering instead

### 16. Operator Overloading

- Mechanics beyond the copy/move assignment ops implied by rule-of-5
- Member vs non-member (free function) overloads
- Common cases: `==`, `<`, `<<`, `[]`, `()`
- The rule-of-thumb: overload to be intuitive, never surprising

### 17. Undefined Behavior
Separates people who've debugged real C++ from those who've only studied it.

- Common landmines: use-after-free, dangling references, uninitialized reads, signed overflow, strict aliasing, out-of-bounds
- *Why* UB exists — the compiler optimizes assuming it never happens

---

## P2 (B2 only) — Database / Infra Add-On

Layer these on **only if targeting** Yugabyte / ScyllaDB / MongoDB / Cloudera-type roles. Ordered by importance.

1. **Storage engines** — B-trees vs **LSM-trees** (memtables, SSTables, compaction); LSM powers most modern distributed DBs
2. **Transactions** — ACID, isolation levels, **MVCC**, pessimistic vs optimistic concurrency control, write-ahead logging (WAL)
3. **Distributed systems (conceptual)** — replication, consistency models, CAP theorem, **Raft** at a high level
4. **Indexing** & basic query processing
5. **Networking basics** — TCP/IP fundamentals, where latency comes from *(relevant for infra/DB internals)*

> Best single resource for this whole layer: **_Designing Data-Intensive Applications_** (Martin Kleppmann).

---

## P3 — Tools & Practices (signal, not core)

Mention these naturally; they prove you're a real systems dev. You've likely touched most on Chromium — say so.

- **Sanitizers** — AddressSanitizer, ThreadSanitizer, UBSan *(almost certainly used on Chromium)*
- **Debugging** — gdb, Valgrind
- **Profiling** — perf, flame graphs
- **Compiler Explorer (godbolt.org)** — practice reading the assembly your C++ generates
- **Build systems** — CMake / Ninja *(you know GN/Ninja from Chromium)*
- **`static` keyword** — its multiple meanings (storage, linkage, class members); a quick gotcha to have ready

---

## P0 (parallel track) — Weaponize Your Chromium Experience

Not a study topic — a **prepared narrative**. This is your unfair advantage; rehearse it like a flagship project. Most candidates here have generic CRUD or competitive-programming backgrounds; you've shipped production systems C++ in one of the most demanding codebases on earth.

Be ready to discuss, in depth:

- Multi-process architecture and **why** (stability, security via sandboxing)
- The IPC layer (Mojo) — how processes communicate
- Memory management & lifetime challenges in a huge, long-running C++ app
- Threading / task scheduling model (browser, renderer, GPU process)
- **One concrete war story** — a race condition, leak, or use-after-free you debugged. One real story beats ten textbook answers.

---

## Core Resources (don't over-collect)

| Resource | Use for | Priority |
|----------|---------|----------|
| **Effective Modern C++** — Scott Meyers | Language depth (P0/P1) | Essential |
| **C++ Concurrency in Action** — Anthony Williams | Concurrency layer | Essential |
| **Designing Data-Intensive Applications** — Kleppmann | B2 database topics | If B2 |
| cppreference.com | Reference | Daily |
| CppCon talks (YouTube) | Specific deep dives | As needed |

*Resist hoarding more — these cover the syllabus end to end.*

---

## Suggested Sequencing

Run this **alongside** DSA, front-loading the highest-ROI topics:

1. Memory & move semantics + smart pointers (P0 #1–3)
2. Concurrency & memory model (P0 #4) — highest leverage from your background
3. const correctness + polymorphism + exceptions + casting (P1 #5–8)
4. STL internals + systems fundamentals (P1 #9–10)
5. Templates, alignment, linking, modern C++, volatile, operators, UB (P2 #11–17)
6. Database add-on (B2 only)
7. Tools + Chromium narrative — polish continuously throughout
