# C++ Interview Prep — Master Roadmap (Merged)

> Target: switching to any C++ company (~1 YOE, currently Chromium-based browser dev).
> Structure: tiered by ROI. Generic C++ first (works for every company), Chromium bonus track at the end, with DSA + behavioral as parallel tracks.
> Items marked **[ADDED]** are gaps filled in beyond the original list.
> Items marked **[MERGED]** were folded in from the two-tracks syllabus (advanced/systems depth) — see the Priority Analysis below.

---

## Merge note + Priority Analysis (why the [MERGED] topics sit where they do)

This doc is the **single canonical syllabus**. It keeps the roadmap's foundational coverage (pointers vs references, `nullptr` vs `NULL`, const member functions — which the two-tracks syllabus *assumed* you knew) and folds in the advanced/systems topics the two-tracks syllabus added. Placements below are **web-validated**, not guessed:

| [MERGED] topic | What the research said | Placement decided | Why |
|---|---|---|---|
| **Systems fundamentals (OS layer)** — process vs thread, virtual memory/paging, scheduling, deadlock, `malloc`/fragmentation, IPC, CPU caches, futex | "Important for systems/FAANG; tests under-the-hood understanding; virtual memory meets pointers/malloc." | **Tier 3** (senior/systems signal); treat as **P1 for systems/FAANG** targets | Not asked in *every* C++ round, but a strong differentiator for the roles you want; plays to your systems background. |
| **Operator overloading** | "Commonly appears in technical interviews." | **Tier 2** (differentiator) | Frequent enough to matter, not make-or-break. Pairs with Rule of 5. |
| **pimpl idiom** | Known/useful technique; about hiding impl + cutting build deps. | **Tier 3** (under compilation/polymorphism) | Niche but a nice senior signal; low frequency. |
| **Smart-pointer internals** ("write `shared_ptr` from scratch", atomic refcount, thread-safety) | Repeatedly flagged a top signal; "beyond manual memory management." | Deepened **Tier 1 #3** | Confirms it's core — added the from-scratch drill + control-block depth. |
| **Storage duration** (automatic/static/dynamic/thread_local) | Part of the object-model core. | Folded into **Tier 1 #1** | Rounds out the memory model. |
| **Browser Part B depth** (Mojo/IPC serialization, sandbox, site isolation, RBI/DOM mirroring, rendering pipeline) | Your niche edge for browser roles. | Expanded **Bonus Track** | Structured version of the browser add-on. |

*Sources consulted: GeeksforGeeks (C++ & OS interview question sets), InterviewBit, Turing — see conversation for links. General C++ core (OOP, memory, STL, smart pointers, multithreading, templates, exceptions) was cross-confirmed as the universally-asked base and already anchors Tiers 1–2.*

---

## How to study each topic (read this first)

For every topic, your daily note should answer four things:

1. **The concept + why it exists** — what problem it solves.
2. **A tiny code example you wrote yourself** — not copied.
3. **The classic interview question + your answer** — from the ⭐ below.
4. **godbolt it when relevant** (move semantics, vtables, RVO, struct padding) — seeing the assembly is what turns "I memorized it" into "I understand it."

---

## TIER 1 — Non-negotiable core
*Asked everywhere, highest ROI.*

> **Time estimates** in each heading assume **~1 hr/day** of C++ study. They're a "first solid pass" target — add ~30% later for revision + self-test drilling.
> **Tier 1 total: ~16 days** · Tier 2: ~16 days · Tier 3: ~12 days

### 1. ⭐ Memory: stack vs heap, RAII (4 days) ✅ DONE
- Stack vs heap (what lives where, lifetimes, cost)
- `new`/`delete`, `new[]`/`delete[]`
- **RAII** — the single most important C++ idiom (resource tied to object lifetime)
- Memory leaks, dangling pointers, double-free, use-after-free
- **[MERGED]** Object lifetime & **storage duration**: automatic, static, dynamic, thread-local
- ⭐ Q: "What is RAII and why does C++ rely on it?" / "How do you prevent leaks without a garbage collector?"

### 2. ⭐ Pointers, references, const correctness (3 days) ✅ DONE
- Pointer vs reference (when each)
- const pointer vs pointer-to-const (read const right-to-left)
- `nullptr` vs `NULL` vs `0`
- const member functions, const parameters/returns, `mutable`
- ⭐ Q: "Difference between `const int*`, `int* const`, `const int* const`?"

### 3. ⭐ Smart pointers (5 days) ✅ DONE *(move ctor/assign pending Topic 7)*
*One of the most-asked topics.*
- `unique_ptr` (sole ownership, move-only)
- `shared_ptr` (reference counting, the control block)
- `weak_ptr` (breaking reference cycles)
- `make_unique` / `make_shared` (and why preferred)
- When to use which
- **[MERGED]** Internals: why `unique_ptr` is zero-overhead; `shared_ptr` **control block + atomic refcount** (and its cost); thread-safety boundaries (refcount is atomic, the object is *not*)
- **[MERGED]** ⭐ **Write a basic `shared_ptr` from scratch** — a classic whiteboard ask
- ⭐ Q: "unique vs shared vs weak ptr?" / "How does shared_ptr's refcount work?" / "What's a shared_ptr cycle and how does weak_ptr fix it?"

### 4. ⭐ OOP + polymorphism + vtables (3 days)
*Ordered basics → high-ROI. SOLID + design patterns are deliberately NOT here — they're LLD-round material in Parallel Track B, a separate interview type.*

**Part A — Fundamentals (clear the basics fast):**
- **[ADDED]** The **4 pillars**, named: Encapsulation, Abstraction, Inheritance, Polymorphism
- Classes, access specifiers, **[ADDED]** `static` members/methods, `this` pointer
- Inheritance types (public/protected/private) + **[ADDED]** **composition vs inheritance** (is-a vs has-a, "prefer composition")

**Part B — The high-ROI meat:**
- **[ADDED]** ⭐ **Overloading vs Overriding vs Hiding** (one of the most-asked OOP questions)
- ⭐ Virtual functions, **vtable/vptr** (how dynamic dispatch works mechanically)
- ⭐ Virtual destructors (why a polymorphic base class needs one)
- Abstract classes, pure virtual functions (interfaces)
- Object slicing
- Static vs dynamic binding (early vs late binding)
- **[ADDED]** ⚠️ Gotcha: calling **virtual functions in constructors/destructors**
- **[ADDED]** RTTI and `dynamic_cast` (how does the runtime know the real type?)
- (Lighter: diamond problem, virtual inheritance)
- `override` / `final`
- **[MERGED]** **pimpl idiom** (pointer-to-implementation) — hides implementation, cuts build dependencies; ties to compilation model (#15)
- ⭐ Q: "What are the 4 pillars?" / "Overloading vs overriding?" / "How do virtual functions work under the hood?" / "Why does a polymorphic base class need a virtual destructor?" / "What is object slicing?" / "Composition vs inheritance — when each?"

### 5. ⭐ Constructors/destructors + Rule of 0/3/5 (2 days) — *do this NEXT (you just built Rule of 3 by hand)*
- Default, parameterized, copy, move constructors; destructor
- Copy constructor vs copy assignment; move constructor vs move assignment
- Deep vs shallow copy
- Rule of 0/3/5 (when you must define them)
- Member initializer lists, order of construction/destruction
- `explicit` keyword
- ⭐ Q: "Explain the Rule of Three/Five." / "When is the copy constructor called vs assignment?" / "Write a class that manages a raw pointer correctly."

### 6. ⭐ [ADDED] The four casts (2 days)
*Asked constantly — surprised this wasn't already in. Pairs naturally with Tier 1.*
- `static_cast` — compile-time, related types
- `dynamic_cast` — runtime, polymorphic downcasts (needs RTTI, returns nullptr/throws)
- `const_cast` — add/remove const (and when it's UB to write through it)
- `reinterpret_cast` — bit reinterpretation, the dangerous one
- Why C-style casts are discouraged (they silently pick whichever cast "works")
- ⭐ Q: "What are the four C++ casts and when do you use each?" / "Why avoid C-style casts?"

---

## TIER 2 — Strong differentiators
*Modern C++, STL internals, concurrency.*

### 7. ⭐ Move semantics & rvalue references (4 days)
*Modern-C++ favorite. → then finish the hand-written `shared_ptr` (Rule of 5).*
- lvalue vs rvalue
- rvalue references (`&&`)
- `std::move` (what it actually does — just a cast!)
- Move constructor/assignment in practice
- Copy elision / RVO
- (Lighter: `std::forward`, perfect forwarding)
- ⭐ Q: "What does std::move actually do?" / "Why are move semantics faster?" / "What is copy elision?"

### 8. ⭐ STL: containers, complexity, iterator invalidation (3 days)
- `vector`, `deque`, `list`, `map`, `unordered_map`, `set`, `unordered_set`, `stack`, `queue`, `priority_queue`
- Internal implementation + complexity (vector = dynamic array; map = red-black tree O(log n); unordered_map = hash table O(1) avg)
- When to use which
- Iterator invalidation (which operations invalidate iterators — very commonly asked)
- Iterator categories (input/forward/bidirectional/random)
- **[ADDED]** Common algorithms: `sort`, `find`, `lower_bound`, `accumulate`, the remove-erase idiom
- ⭐ Q: "map vs unordered_map — when and why?" / "When does a vector iterator get invalidated?" / "How is unordered_map implemented?"

### 9. ⭐ Concurrency & multithreading (2 days — your strength, should go fast)
*Critical for your systems background.*
- `std::thread`
- `mutex`, `lock_guard`, `unique_lock`, `scoped_lock`
- `condition_variable`
- `std::atomic` (basics first)
- Data race vs race condition
- Deadlock (the 4 conditions, how to avoid, lock ordering)
- Thread-safe design
- **[ADDED]** `volatile` vs `atomic` — the gotcha: "can I use `volatile` for thread sync?" (No — and knowing *why* is a strong signal)
- (Lighter: `async`, `future`, `promise`)
- ⭐ Q: "Difference between mutex and atomic?" / "What causes a deadlock and how do you prevent it?" / "Make this class thread-safe."
- 🔧 *Your material:* Redis project's `db_mutex` + the TSan hunt.

### 10. Templates & generics (3 days)
- Function templates, class templates
- Template specialization (full + partial)
- `auto`, `decltype`
- (Template metaprogramming → Tier 4)
- Q: "How do templates work / when are they instantiated?"

### 11. Modern C++ (C++11/14/17) essentials (2 days)
- Lambdas (captures by value/ref, closures, mutable lambdas)
- `auto`, range-based for
- Structured bindings
- `constexpr`
- `std::optional`, `std::variant`
- `enum class`, uniform/`{}` initialization
- **[ADDED]** `std::string_view` and `std::span` — extremely common now; classic dangling-reference footgun follow-up
- ⭐ Q: "How do lambda captures work?" / "What's the difference between `[=]` and `[&]`?" / "What's the danger with `string_view`?"

---

### 11b. ⭐ [MERGED] Operator overloading (2 days)
*Commonly appears; pairs naturally with the Rule of 5.*
- Mechanics beyond copy/move assignment
- Member vs non-member (free function) overloads
- Common cases: `==`, `<`, `<<`, `[]`, `()`
- Rule of thumb: intuitive, never surprising (mirror built-in semantics)
- ⭐ Q: "When would you make an operator a member vs a free function?" / "How do you overload `<<` for your type?"

---

## TIER 3 — Senior / systems signals

### 11c. ⭐ [MERGED] Systems fundamentals (OS layer) (2 days — your strength)
*Tier 3 in general, but treat as **P1 for systems / FAANG** targets — plays directly to your background.*
- Process vs thread; context switching; scheduling basics
- **Virtual memory & paging** (where the "virtual address space" from stack-vs-heap really lives)
- `malloc` / allocators — arenas, fragmentation; `mmap`
- **IPC** — pipes, shared memory, sockets, message passing
- CPU caches — hierarchy, cache lines, spatial/temporal locality, what causes misses
- Futex underneath mutexes
- ⭐ Q: "process vs thread?" / "what happens on a page fault?" / "how would you debug an OOM kill / a slow server?" / "what causes cache misses?"

### 12. Undefined behavior (1 day)
- What UB is and why it's dangerous
- Common sources: signed overflow, out-of-bounds, use-after-free, uninitialized reads, strict aliasing, dangling references
- Q: "Give examples of undefined behavior." / "Why is UB worse than a crash?"
- 🔧 *Your material:* the heisenbug story.

### 13. Exception handling & safety (2 days)
- `try`/`catch`/`throw`
- Exception safety guarantees (no-throw / strong / basic)
- RAII + exceptions (why destructors must not throw)
- `noexcept`
- Q: "What are the exception safety guarantees?" / "Why shouldn't destructors throw?"

### 14. The memory model & atomics (deeper) (3 days)
- `memory_order` (relaxed, acquire, release, seq_cst)
- happens-before relationship
- False sharing, cache lines (for performance-sensitive roles)
- Q: "Explain acquire/release semantics." / "What is false sharing?"

### 15. Compilation, linking, ODR (2 days)
- Compilation stages (preprocess → compile → assemble → link)
- Declaration vs definition, One Definition Rule
- `static` vs `extern`, internal/external linkage
- `inline`, header guards / `#pragma once`, forward declarations
- **[ADDED]** Static initialization order fiasco (and the Meyers singleton fix) — commonly asked, relevant to real codebases
- Q: "What's the difference between declaration and definition?" / "What does `inline` really mean?"

### 16. [ADDED] Memory layout & object model (2 days)
*You're a systems person — this plays to your strength.*
- Struct padding, alignment, `alignas`/`alignof`
- "What's the `sizeof` this struct and why?" (reorder members to shrink it)
- Object layout: where the vptr sits, empty base optimization
- Endianness basics
- Q: "Why is this struct 24 bytes not 13?" / "How would you reduce its size?"

---

## PARALLEL TRACK A — DSA / coding rounds
*This is the actual first gate for most switches. Mastering every concept above won't help if a medium graph problem filters you in round one. Schedule this as real daily practice, not an afterthought.*

- **Patterns:** two pointers, sliding window, binary search, BFS/DFS, backtracking, dynamic programming, heaps, intervals, prefix sums, union-find
- **Data structures to be fluent in:** arrays/strings, hashmaps, stacks/queues, linked lists, trees (BST, traversals), graphs, tries, heaps
- **Cadence:** aim for a steady rhythm of mediums; revisit wrong ones. Mix in a few hards once mediums feel comfortable.
- **C++-specific:** be fast with STL in a timed setting — `unordered_map`, `priority_queue`, `sort` with custom comparators, `lower_bound`. Interviewers notice clean idiomatic STL usage.

---

## PARALLEL TRACK B — OOP design + design patterns (LLD rounds)
*Tested in design rounds, not coding rounds. Run alongside the language topics.*

- **SOLID principles** (start here)
- **Creational:** Singleton, Factory, Abstract Factory, Builder
- **Structural:** Adapter, Decorator, Facade, Proxy
- **Behavioral:** Observer ⭐, Strategy ⭐, Command, Iterator
- **Practice designing:** LRU cache, thread-safe queue, rate limiter, logger, parking lot
- *(Singleton, Factory, Observer, Strategy are the most-asked — start with those.)*

---

## PARALLEL TRACK C — [ADDED] Behavioral & project storytelling
*At 1 YOE, "tell me about the hardest bug you debugged" is near-guaranteed. Prep 2–3 tight STAR-style stories instead of leaving them as footnotes.*

- **STAR format:** Situation → Task → Action → Result (keep each to ~2 min spoken).
- **Stories worth pre-writing:**
  - The TSan hunt / `db_mutex` race in the Redis project (concurrency depth)
  - The heisenbug / UB story (systems debugging instinct)
  - A shipping/ownership story from Chromium work (scope, impact, collaboration)
- **Standard prompts to have answers for:** hardest bug, disagreement with a teammate, something you'd do differently, why you're switching, a time you improved performance.

---

## BONUS TRACK — Chromium / browser-internals edge
*Re-scoped for a **browser-company** target (any tier, mid-level ok): this is no longer just "flavor for generic C++ roles" — it's a **core study block**, because browser interviews probe it directly. The **P0 → P2 ordering below is based on interview analysis**: Chromium onboarding/architecture docs, Chromium-internals writeups, and browser-role job descriptions, which consistently list "rendering, networking, multi-process architecture, system integrations, build systems." The most heavily and consistently probed area is **multi-process architecture + Mojo IPC** — everything else ranks under it.*
 
*Calibration for mid-level: you need **fluent literacy** across P0/P1 and the ability to tie each topic back to what you actually shipped — **not** mastery of every internal. Go deep only in the P2 area that matches your specific target company. The three C++-idiom items still double as browser-flavored answers to generic C++ questions — see the "Generic-topic mappings" at the end.*
 
### ▸ P0 — Core: expected in essentially every browser interview
 
#### B1. Multi-process architecture  *(promoted from "optional" — this IS the conversation)*
- Process types and roles: **browser** (privileged broker), **renderer** (one per site-instance, sandboxed), **GPU**, **utility / service**, **network service** — and **why** the split exists (stability + security, like a modern OS isolating apps: one bad tab can't take down the browser).
- The host/impl object model: `RenderProcessHost`/`RenderProcess`, `RenderFrameHost`/`RenderFrame`, and **routing IDs** (identifying a frame needs host + routing ID together).
- Renderer crash → "sad tab"; process-per-site-instance; process-reuse strategies (e.g. `window.open` same-origin sharing).
- ⭐ Q to nail: "Why is a browser multi-process?" / "What processes exist and what does each do?"
#### B2. Mojo IPC  *(the primary IPC system — legacy IPC is deprecated)*
- `.mojom` interface definitions, message pipes, browser↔renderer communication, passing object handles (file descriptors) safely across the boundary.
- **Async by default**; why **synchronous browser→renderer IPC is disallowed** (never block the UI on a flaky renderer); sync messages handled only on the I/O thread.
- Why Mojo replaced legacy IPC: endpoint-to-endpoint, no giant routing/plumbing classes, faster, fewer context switches.
- ⭐ Talking point: "structured data crosses the process boundary via mojom-generated serialization, and the browser validates it because the renderer is untrusted."
#### B3. In-process threading model
- Browser process: **UI thread**, **I/O thread** (handles IPC + resource requests off the UI thread so the UI stays responsive), plus worker/pool threads.
- Renderer: **main thread** (JS / DOM / style / layout) + **compositor** and **raster** threads.
- Sequences vs physical threads *(cross-ref the Concurrency mapping below — the key mental model)*; why the main thread must never block.
- ⭐ Talking point: "resource requests run on the I/O thread precisely so the UI thread never stalls."
### ▸ P1 — Strongly expected: know the high-level version even if you never touched that code
 
#### B4. Sandboxing & site isolation  *(promoted from "isolation companies only" — broadly asked now)*
- Sandbox as a security boundary: what a renderer **can't** do (no direct filesystem / network / display access) and how it asks the browser to broker those.
- **Site Isolation / OOPIFs** (out-of-process iframes): cross-site documents in distinct renderers; the browser mediates cross-process input, hit-testing, focus, navigation.
- ⭐ Q: "How does Chromium stop a compromised renderer from reading another site's data?"
#### B5. Rendering pipeline literacy  *(promoted from "graphics roles only")*
- Parse (HTML→DOM, CSS→CSSOM) → **style** → **layout** → **paint** (records display items, not pixels) → **composite** → **raster** → GPU/Viz.
- Main thread vs compositor thread; what triggers **reflow / repaint** and causes **jank**.
- ⭐ Q: "What happens between HTML arriving and pixels on screen?" — you need the *map*, not Blink internals.
#### B6. Navigation & page lifecycle  *(added — near-guaranteed "URL → pixels")*
- End-to-end: DNS / connection → network-service fetch → response → renderer selection/commit → parse → render.
- Which process participates at each step; what the browser brokers vs what the renderer does.
#### B7. Ownership model & callbacks  *(your existing strength — keep; cross-ref the C++ tiers)*
- `scoped_refptr` / `base::RefCounted`, `base::WeakPtr`, `raw_ptr` (MiraclePtr); `base::BindOnce` / `BindRepeating` with weak-ptr auto-cancel. Full compare/contrast in "Generic-topic mappings" below.
### ▸ P2 — Role-dependent differentiators: study the ONE matching your target company
 
#### B8. Build system & upstream tracking  *(added — heavily valued by any company that forks Chromium)*
- **GN** + **Ninja**, `depot_tools` / `gclient`, component vs static builds, incremental compilation, cross-platform build flags.
- Rebasing / merging a fork onto upstream Chromium — the recurring pain at every non-Google browser shop.
- ⭐ Why it matters: JDs explicitly list "build systems, CI/CD, alignment with upstream Chromium updates."
#### B9. Networking / resource loading  *(added)*
- The out-of-process **network service**, `URLLoaderFactory`, and **why** networking moved out of the browser process (shrink attack surface — HTTP parsing sits close to attacker-controlled content).
- Caching, resource-fetch flow, cookies.
#### B10. Embedding — CEF / WebView / content layer  *(added — central for enterprise / kiosk / embedded shops)*
- The **content** public API (embedder-facing layer), **CEF** (Chromium Embedded Framework), Android **WebView**.
- Where you hook customizations without patching the engine.
#### B11. Security deep-dive  *(only for isolation companies: Menlo, Island, Palo Alto, Cloudflare)*
- DOM as attack surface; Remote Browser Isolation (RBI) & **DOM mirroring** vs pixel streaming; sandbox-escape threat model; TLS/HTTPS proxies, traffic interception/inspection.
#### B12. Deep rendering internals  *(only for dedicated graphics teams: Chrome graphics, engines)*
- Blink layout/paint internals, LayoutNG, GPU command buffer / Viz, frame-timing.
---
 
### Generic-topic mappings  *(answer a generic C++ question, then add "…and here's how a browser does it")*
 
**Smart pointers → Chromium's ownership model**
- `scoped_refptr` / `base::RefCounted` — **intrusive** refcounting vs `shared_ptr`'s external control block (count lives *in* the object: saves an allocation, requires the type to opt in).
- `base::WeakPtr` — sequence-checked, invalidates safely on the owning sequence (vs `std::weak_ptr`, purely refcount lifetime).
- `raw_ptr` (MiraclePtr/BackupRefPtr) — hardened raw pointer for use-after-free mitigation.
- ⭐ "shared_ptr uses an external control block with atomic refcounts; scoped_refptr is intrusive — the count lives in the object, saving an allocation but requiring opt-in."
**Concurrency → the threading model**
- Sequences vs physical threads; `TaskRunner` / `SequencedTaskRunner`; `SequenceChecker` / `ThreadChecker` (thread-safety *asserted*, not locked) — avoids lock contention and whole classes of races by construction.
- ⭐ When asked "make this thread-safe": give the `std::mutex` answer, then "in a browser we'd often confine the object to one sequence and post tasks to it instead of locking."
**Callbacks → why not std::function**
- `base::BindOnce` / `BindRepeating`, `OnceCallback` / `RepeatingCallback`; move-only support, weak-ptr binding (auto-cancel if receiver is gone), explicit once-vs-repeating semantics.
- ⭐ "BindOnce gives move-only callbacks and can bind to a WeakPtr so the call is silently dropped if the target died — std::function can't express either cleanly."

---

## Priority order (if time is tight)

1. **DSA track** — it's the actual first gate.
2. **Tier 1** (incl. the four casts) — non-negotiable, asked everywhere.
3. **Tier 2** — the differentiators (move semantics, STL internals, concurrency).
4. **Behavioral stories** — cheap to prep, high payoff, guaranteed to come up.
5. **Tier 3 + LLD patterns** — senior signal.
6. **Chromium bonus track** — folds into the above as your edge; doesn't need its own block of time.
---

## [MERGED] Recommended resources
| Resource | Use for | Priority |
|----------|---------|----------|
| **Effective Modern C++** — Scott Meyers | Language depth (Tier 1/2) | Essential |
| **C++ Concurrency in Action** — Anthony Williams | Concurrency (#9, #14) | Essential |
| cppreference.com | Reference | Daily |
| CppCon talks (YouTube) | Deep dives | As needed |
| Compiler Explorer (godbolt.org) | See the assembly your C++ generates (move, vtables, RVO, padding) | When relevant |
