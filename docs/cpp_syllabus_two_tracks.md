# C++ Interview Syllabus — Two Tracks

Structured so the two targets never mix:

- **PART A — Core C++.** The universal syllabus for **any C++-heavy company** — generic product / FAANG / systems roles. Complete on its own.
- **PART B — Browser / Isolation Add-On.** An extra layer studied **on top of Part A**, only when targeting browser-niche companies (Chrome/Edge/Island/Menlo/Palo Alto/rendering). This is where your Chromium background is the edge.

**How to use:**
| Target | Study |
|--------|-------|
| Generic / FAANG / systems C++ company | **Part A only** |
| Browser / isolation / rendering company | **Part A + Part B** |

**On DSA:** You're already strong (750+ solved, DP/graphs/trees). This is a **revision** track, not a grind — keep it sharp so generic/FAANG doors stay wide open. All prep below is the *C++ half*; DSA runs in parallel as maintenance.

**Priority legend** (applies within each Part):

| Tier | Meaning | Action |
|------|---------|--------|
| **P0** | Asked in almost every interview. Make-or-break core. | Master deeply. Explain *and* implement. |
| **P1** | Frequently asked, strong differentiator. | Know well, explain out loud. |
| **P2** | Deeper rounds / specific teams. | Solid working knowledge. |
| **P3** | Edge / nice-to-have. | Familiarity; mention naturally. |

Everything within each tier is ordered by importance. Work top to bottom.

---
---

# PART A — CORE C++
### (for every C++-heavy company: generic, FAANG, systems)

## A-P0 — Master First (highest ROI)

### 1. Memory & Object Model
- **RAII** — resource lifetime tied to object lifetime; why it's the central idiom
- Object lifetime & storage duration (automatic, static, dynamic, thread-local)
- **Rule of 0 / 3 / 5**
- Stack vs heap (language *and* OS level)
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
- **Write a basic `shared_ptr` from scratch** — a classic ask

### 4. Concurrency & Memory Model
- `std::thread` lifecycle, joining/detaching
- Locking family: `lock_guard`, `unique_lock`, `scoped_lock`
- `condition_variable` + producer–consumer pattern
- `std::atomic` and the **memory model**: relaxed / acquire-release / seq_cst
- Data race vs race condition vs deadlock vs livelock
- `async` / `future` / `promise`
- False sharing & cache-line contention
- *(Lock-free structures NOT required unless a role explicitly demands it.)*

---

## A-P1 — Strong Differentiators

### 5. const Correctness
- `const` member functions; `const` objects
- Const refs / pointers — pointer-to-const vs const-pointer (read right-to-left)
- `mutable`
- How const-correctness propagates through an API

### 6. Polymorphism Internals
- **vtable / vptr** mechanics — virtual dispatch at the binary level
- Virtual destructors (why they matter)
- Object slicing
- Diamond problem & virtual inheritance
- `override` / `final`
- **pimpl idiom**

### 7. Exception Handling & Exception Safety
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
- `map` (red-black tree) vs `unordered_map` (buckets, load factor, hashing)
- **Iterator invalidation rules** — classic gotcha
- Complexity of common containers/algorithms off the top of your head

### 10. Systems Fundamentals (OS layer)
- Process vs thread, context switching, scheduling basics
- Virtual memory & paging
- `malloc` / allocators (arenas, fragmentation), `mmap`
- **IPC (general)** — pipes, shared memory, sockets, message passing
- CPU caches — hierarchy, cache lines, spatial/temporal locality, what causes misses
- Futex underneath mutexes

---

## A-P2 — Deeper Rounds

### 11. Templates & Generic Programming
- Function & class templates
- Full and partial specialization
- Variadic templates
- Type traits, basic SFINAE
- **CRTP**
- Concepts (C++20)
- *Goal: read/reason about templated code comfortably — not metaprogramming mastery. Slightly higher weight for FAANG generic rounds.*

### 12. Memory Alignment & Padding
*Borderline P1 for cache-sensitive / GPU / embedded teams.*
- Struct padding & member reordering to minimize size
- `alignas` / `alignof`
- Why alignment matters (performance + correctness on some hardware)
- Connection to cache-line layout (ties to #4, #10)

### 13. Compilation & Linking Model
- Translation units; compile → link flow
- Declaration vs definition
- Internal vs external linkage
- **One Definition Rule (ODR)**
- Header guards / `#pragma once`; what `inline` really means for linkage

### 14. Modern C++ Fluency (C++11 → 20)
- Lambdas & capture semantics (incl. the dangling-capture trap)
- `auto`, `constexpr`
- Structured bindings
- `std::optional` / `std::variant`
- `string_view`, `span`
- `std::function` & callbacks *(goes much deeper in Part B for browser roles)*

### 15. `volatile` vs `atomic`
- What `volatile` actually does (blocks certain compiler optimizations)
- Why `volatile` does **not** provide thread-safety or atomicity
- When `volatile` is legitimately needed (memory-mapped I/O, signal handlers)
- Use `std::atomic` for cross-thread visibility/ordering

### 16. Operator Overloading
- Mechanics beyond copy/move assignment (rule-of-5)
- Member vs non-member (free function) overloads
- Common cases: `==`, `<`, `<<`, `[]`, `()`
- Rule-of-thumb: intuitive, never surprising

### 17. Undefined Behavior
- Landmines: use-after-free, dangling references, uninitialized reads, signed overflow, strict aliasing, out-of-bounds
- *Why* UB exists — the compiler optimizes assuming it never happens

---

## A-P3 — Tools & Practices (signal, not core)
- **Sanitizers** — AddressSanitizer, ThreadSanitizer, UBSan
- **Debugging** — gdb, Valgrind
- **Profiling** — perf, flame graphs
- **Compiler Explorer (godbolt.org)** — read the assembly your C++ generates
- **Build systems** — CMake / Ninja
- **`static` keyword** — its multiple meanings (storage, linkage, class members)

---

## Part A — Core Resources
| Resource | Use for | Priority |
|----------|---------|----------|
| **Effective Modern C++** — Meyers | Language depth (P0/P1) | Essential |
| **C++ Concurrency in Action** — Williams | Concurrency (#4) | Essential |
| cppreference.com | Reference | Daily |
| CppCon talks (YouTube) | Deep dives | As needed |

---
---

# PART B — BROWSER / ISOLATION ADD-ON
### (study ON TOP of Part A, only for browser-niche companies)

> This is your edge. These topics are the C++ dialect Chromium-style codebases run on — you already use them daily, while most candidates have never seen them. For pure-browser roles, treat B-P0 as genuinely P0.

## B-P0 — Browser Core

### B1. Async, Callbacks & Ownership (the browser dialect)
- **Task-based async & message loops** — posting work to loops/threads (Chromium's `PostTask` model); event-driven execution
- **Callbacks & binding** — `std::function`, `std::bind` / lambda callbacks; one-shot vs repeating (`BindOnce` / `BindRepeating`)
- **Ownership across async boundaries** — the core hard problem: a callback that outlives its target object
- **Weak pointers for dangling prevention** — invalidate-on-destroy (`base::WeakPtr`); how it differs from `std::weak_ptr`
- **Intrusive reference counting** — `scoped_refptr` / `RefCounted` vs `shared_ptr` (intrusive vs external control block)
- *Whiteboard drill: "a callback captures `this`, the object is destroyed, the callback fires — what happens, how do you prevent it?"*

### B2. Multi-Process Architecture & IPC Depth
- **Process model** — browser vs renderer vs GPU vs network vs utility; **why** (stability + security via sandboxing)
- **IPC / Mojo** — how processes communicate; **serialization / marshalling across process boundaries** (how structured data crosses safely)
- **Sandbox as a security boundary** — what the renderer can/can't do
- **Site isolation** — process-per-site-instance and why

---

## B-P1 — Browser Differentiators

### B3. Browser Security / Isolation Concepts
*(Especially for isolation companies: Menlo, Island, Palo Alto, Cloudflare.)*
- The DOM as an attack surface
- Remote Browser Isolation (RBI) & **DOM mirroring** vs pixel streaming
- Exploit/sandbox-escape threat model at a high level
- TLS/HTTPS, proxies, how traffic is intercepted/inspected

### B4. Rendering Pipeline (light)
*(For rendering-heavy roles: Chrome graphics, Figma, engines.)*
- Parse → layout → paint → composite, at a high level
- GPU process role; frame timing basics
- *Keep light unless targeting a dedicated graphics team.*

---

## B — Weaponize Your Chromium Experience (parallel track)
Not study — a **prepared narrative**, your single biggest asset. Overlaps B1/B2, so prep together. Rehearse until smooth.
- Multi-process architecture and **why**
- The IPC layer (Mojo) — communication + serialization
- Memory management & lifetime in a huge, long-running C++ app
- Threading / task scheduling across processes
- **One concrete war story** — a race condition, leak, or use-after-free you debugged

> **Open-source proof:** land **2–3 Chromium CLs** (GoodFirstBug → Gerrit). The key resume-shortlisting signal — and you can do it far faster than newcomers because you already build and navigate the codebase.

---

## Part B — Resources
| Resource | Use for |
|----------|---------|
| Chromium docs (`docs/`, design docs) | B1, B2, narrative |
| Target companies' engineering blogs (Menlo/Island/Cloudflare) | B3 — speak their architecture |

---
---

# Sequencing

**If targeting generic / FAANG / systems (Part A only):**
1. A-P0 core — memory, move, smart pointers (#1–3)
2. Concurrency (#4)
3. const + polymorphism + exceptions + casting (#5–8)
4. STL internals + systems fundamentals (#9–10)
5. Templates, alignment, linking, modern C++, volatile, operators, UB (#11–17)
6. Tools (P3) — polish throughout
7. DSA revision runs in parallel (your edge here)

**If targeting browser / isolation (Part A + Part B):**
1. Do the Part A P0/P1 core first (it underpins everything)
2. B1 + B2 — async/ownership + multi-process/IPC (your edge; prep with the narrative)
3. B3/B4 as the specific company demands
4. Land your 2–3 Chromium CLs early — highest-leverage single move
5. Finish remaining Part A P2 topics
6. DSA stays light here — your niche depth carries the interview

---

*Note: other C++-heavy sub-domains (databases/infra, embedded/firmware, networking, HFT) each need their own small add-on beyond Part A. Not included here to keep focus — ask for a specific add-on only if you decide to target one seriously.*
