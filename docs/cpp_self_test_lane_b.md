# C++ Self-Test — Systems & Database Roles (Lane B1/B2)

Companion to the syllabus. Topic numbering matches it exactly. **How to use:** read each question, answer it *out loud* as if in an interview, then check the "strong answer hits" line. If you can't cover those points fluently, that topic isn't cold yet — send it back to the study pile.

> The checkpoints are deliberately terse — they're a grading rubric, not model answers. The goal is to expose gaps, not to read.

---

## P0 — Core

### 1. Memory & Object Model
1. **What is RAII and why does it matter?**
   *Strong answer hits:* resource tied to object lifetime, acquired in constructor / released in destructor, automatic cleanup even on exceptions, no manual `delete`.
2. **Walk me through the rule of 3 / 5 / 0. When do you need each?**
   *Hits:* if you define one of destructor/copy-ctor/copy-assign you usually need all three (rule of 3); add move-ctor/move-assign for rule of 5; rule of 0 = design so you need none (let members manage themselves).
3. **Stack vs heap — what lives where, and what are the trade-offs?**
   *Hits:* stack = automatic lifetime, fast, limited size, LIFO; heap = manual/dynamic, larger, slower, fragmentation, must free.
4. **In what order are constructors and destructors called for a class with members and a base class?**
   *Hits:* base before derived (ctor); members in declaration order; destruction in exact reverse.

### 2. Move Semantics
1. **What's the difference between `std::move` and `std::forward`?**
   *Hits:* `move` = unconditional cast to rvalue; `forward` = conditional cast preserving lvalue/rvalue-ness, used in perfect forwarding with universal references.
2. **Does `std::move` actually move anything?**
   *Hits:* no — it's just a cast; the move happens when a move-ctor/assign is invoked.
3. **When does a move happen instead of a copy? When does it silently fall back to a copy?**
   *Hits:* move when source is an rvalue and a move op exists; falls back to copy if no move op, or if move op isn't `noexcept` (containers), or object is const.
4. **What is RVO/NRVO and how does it interact with move?**
   *Hits:* compiler elides the copy/move entirely on return; mandatory in C++17 for prvalues; don't `std::move` a local on return — it can disable NRVO.

### 3. Smart Pointers
1. **`unique_ptr` vs `shared_ptr` — when each, and what's the cost difference?**
   *Hits:* unique = sole ownership, zero overhead; shared = shared ownership via control block + atomic refcount (heavier).
2. **How does `shared_ptr`'s reference counting stay thread-safe? Is the pointed-to object thread-safe too?**
   *Hits:* the control block's refcount is atomic; the managed object is NOT protected — concurrent mutation still needs your own sync.
3. **What problem does `weak_ptr` solve?**
   *Hits:* breaks reference cycles (which leak); non-owning observer; must `lock()` to use.
4. **`make_shared` vs `new` + `shared_ptr` — why prefer `make_shared`?**
   *Hits:* single allocation (object + control block together), exception-safe; downside: memory held until last weak_ptr dies.
5. **(Stretch) Sketch a minimal `shared_ptr`.**
   *Hits:* pointer + heap control block with count, increment on copy, decrement on destroy, delete at zero.

### 4. Concurrency & Memory Model
1. **Data race vs race condition vs deadlock — define each.**
   *Hits:* data race = unsynchronized concurrent access with ≥1 write (UB); race condition = correctness depends on timing; deadlock = circular wait on locks.
2. **`mutex` vs `atomic` — when would you reach for each?**
   *Hits:* atomic for single-variable lock-free ops; mutex for protecting larger critical sections / invariants across multiple variables.
3. **Explain memory ordering: relaxed vs acquire-release vs seq_cst.**
   *Hits:* relaxed = atomicity only, no ordering; acquire-release = synchronizes paired operations; seq_cst = single total order, strongest + slowest (default).
4. **What's false sharing and how do you fix it?**
   *Hits:* two threads write different variables on the same cache line → contention; fix with padding/alignment to separate cache lines.
5. **How do you avoid deadlock with multiple locks?**
   *Hits:* consistent lock ordering, `std::scoped_lock`/`std::lock` for multiple, timeouts, avoid holding locks across calls.

---

## P1 — Differentiators

### 5. const Correctness
1. **`const int*` vs `int* const` vs `const int* const` — explain each.**
   *Hits:* read right-to-left — pointer to const int / const pointer to int / const pointer to const int.
2. **What does a `const` member function guarantee, and what is `mutable` for?**
   *Hits:* won't modify observable object state (can't change non-mutable members, can't call non-const members); `mutable` exempts a member (e.g. cache, lock).

### 6. Polymorphism Internals
1. **What happens in memory when you call a virtual function?**
   *Hits:* object holds a vptr → points to the class's vtable → indirect lookup of the function pointer → call; one extra indirection.
2. **Why must a base class destructor be virtual?**
   *Hits:* `delete base_ptr` to a derived object skips the derived destructor without it → leak / UB.
3. **What is object slicing?**
   *Hits:* copying a derived object into a base value drops the derived part; happens with by-value passing/assignment.
4. **What's the pimpl idiom and why use it?**
   *Hits:* hide implementation behind an opaque pointer; reduces compile dependencies / ABI stability / faster builds.

### 7. Exception Handling & Exception Safety
1. **Define the three exception-safety guarantees.**
   *Hits:* basic = no leak, valid-but-unspecified state; strong = commit-or-rollback (state unchanged on throw); nothrow = never throws.
2. **What does `noexcept` do and why does it matter for move operations?**
   *Hits:* promises no throw; containers only move (vs copy) on reallocation if the move is `noexcept`, else they copy for safety.
3. **Why should you never let an exception escape a destructor?**
   *Hits:* during unwinding two live exceptions → `std::terminate`.
4. **How does RAII relate to exception safety?**
   *Hits:* stack unwinding runs destructors → RAII guarantees cleanup automatically, no leaks on the throw path.

### 8. Casting & RTTI
1. **Name the four C++ casts and when each is appropriate.**
   *Hits:* static (related types, compile-checked), dynamic (safe down-cast in polymorphic hierarchy, runtime-checked), const (add/remove const), reinterpret (bit reinterpretation, dangerous).
2. **How does `dynamic_cast` work and what's its cost?**
   *Hits:* uses RTTI to check the actual type at runtime; returns null (pointers) / throws (refs) on failure; runtime overhead, needs polymorphic type.
3. **Why are C-style casts discouraged?**
   *Hits:* they silently try several casts including reinterpret/const — unclear intent, unsearchable, unsafe.

### 9. STL Internals & Complexity
1. **`map` vs `unordered_map` — internals and when to choose each.**
   *Hits:* map = red-black tree, ordered, O(log n); unordered_map = hash buckets, average O(1) / worst O(n), unordered; choose by ordering need + worst-case tolerance.
2. **What happens to `vector` capacity as it grows? What does that mean for iterators?**
   *Hits:* geometric growth (e.g. ×2), reallocation copies/moves elements → invalidates all iterators/pointers/refs.
3. **Which operations invalidate iterators for vector, and for map/list?**
   *Hits:* vector — realloc invalidates all, erase invalidates from the point on; node-based (map/list/set) — only the erased element's iterator.

### 10. Systems Fundamentals (OS layer)
1. **Process vs thread — what's shared, what isn't?**
   *Hits:* threads share address space/heap/file descriptors, have own stack/registers; processes are isolated → IPC needed.
2. **Walk me through what a cache miss costs and how data layout affects it.**
   *Hits:* miss → fetch from slower level/RAM (tens–hundreds of cycles); contiguous/aligned data improves locality, reduces misses.
3. **Name IPC mechanisms and a trade-off of each.**
   *Hits:* pipes (simple, stream), shared memory (fastest, needs sync), sockets (network-capable, overhead), message queues. *(Tie to Chromium's Mojo.)*
4. **What is virtual memory / paging buying you?**
   *Hits:* isolation, illusion of large contiguous space, demand paging, protection.

---

## P2 — Deeper Rounds

### 11. Templates & Generic Programming
1. **Function template vs class template; what's specialization?**
   *Hits:* templates generate code per type at compile time; full/partial specialization provides custom impls for specific types.
2. **What is SFINAE / when do you use type traits?**
   *Hits:* substitution failure isn't an error — enables overload selection based on type properties; `enable_if`, `is_*` traits, or Concepts (C++20) as the modern replacement.
3. **What's CRTP and why use it?**
   *Hits:* derived passes itself as template arg to base → static polymorphism, no vtable cost.

### 12. Memory Alignment & Padding
1. **Why does this struct take more bytes than the sum of its members?**
   *Hits:* padding inserted so members sit on aligned addresses; reorder largest-to-smallest to shrink.
2. **What do `alignas` / `alignof` do, and when does alignment matter?**
   *Hits:* query/force alignment; matters for performance (cache lines, SIMD) and correctness on strict-alignment hardware.

### 13. Compilation & Linking Model
1. **Walk me from source files to an executable.**
   *Hits:* preprocess → compile each translation unit to object file → link (resolve symbols) → executable.
2. **What is the ODR? Give a way to violate it.**
   *Hits:* each entity defined once across the program (inline/templates excepted); duplicate non-inline definition in two TUs violates it.
3. **Internal vs external linkage — how do you control it?**
   *Hits:* internal = TU-local (`static` at file scope, anonymous namespace); external = visible across TUs (default for non-const globals/functions).

### 14. Modern C++ Fluency
1. **Explain lambda capture: by value vs by reference, and the dangling trap.**
   *Hits:* `[=]` copies, `[&]` references; capturing by ref a local that outlives scope → dangling.
2. **`std::optional` / `std::variant` — what problems do they solve?**
   *Hits:* optional = maybe-a-value without sentinels/nulls; variant = type-safe union.
3. **What does `std::function` cost vs a raw function pointer or lambda?**
   *Hits:* type-erased, may heap-allocate, indirect call — convenient but not free.

### 15. `volatile` vs `atomic`
1. **Does `volatile` make a variable thread-safe?**
   *Hits:* no — it only blocks certain compiler optimizations; gives no atomicity or cross-thread ordering. Use `std::atomic`.
2. **When is `volatile` actually the right tool?**
   *Hits:* memory-mapped hardware I/O, `sig_atomic_t`-style signal handler flags.

### 16. Operator Overloading
1. **When should an operator be a member vs a free function?**
   *Hits:* member when it needs the left operand to be the class and access to internals; free (often friend) for symmetry, esp. when left operand may be another type (e.g. `<<`).
2. **What's the rule of thumb for overloading?**
   *Hits:* behave like the built-in / least-surprise; don't overload to be clever.

### 17. Undefined Behavior
1. **Name several sources of UB and what the compiler is allowed to do.**
   *Hits:* use-after-free, dangling refs, uninitialized reads, signed overflow, OOB, strict aliasing; compiler may assume UB never happens → reorder/delete code.
2. **Why does UB exist at all?**
   *Hits:* lets the compiler optimize aggressively without checking impossible cases; the cost of C++'s performance model.

---

## P2 (B2 only) — Database / Infra

1. **B-tree vs LSM-tree — trade-offs?**
   *Hits:* B-tree = read-optimized, in-place updates; LSM = write-optimized (append + compaction), better for high write throughput. LSM powers most modern distributed DBs.
2. **Explain MVCC and what problem it solves.**
   *Hits:* versioned rows → readers don't block writers; supports snapshot isolation.
3. **What is a write-ahead log (WAL) for?**
   *Hits:* durability — log the change before applying; replay on crash recovery.
4. **CAP theorem — what's the real-world trade-off?**
   *Hits:* under a partition you choose consistency or availability; the practical axis is CP vs AP.
5. **Why does Raft exist / what does consensus give you?**
   *Hits:* agree on a replicated log across nodes despite failures → consistent replication, leader election.

---

## Narrative Self-Test — Your Chromium Story

Not technical recall — rehearse these until they're smooth. Record yourself; aim for clear, structured, 60–90 sec answers.

1. **Why is a browser multi-process? What breaks if it's single-process?**
2. **Walk me through how two Chromium processes communicate.** (Mojo / IPC)
3. **Tell me about a hard bug you debugged.** (Pick one: race condition / leak / use-after-free. State symptom → investigation → root cause → fix.)
4. **What was the trickiest memory or lifetime problem you hit, and how did you reason about it?**
5. **How is work scheduled across threads/processes in the browser?**

> One concrete, specific war story beats five textbook answers. Have at least two ready.

---

## Scoring Yourself

For each question, rate honestly:
- **Cold** — answered fully, fluently, unprompted. Move on.
- **Shaky** — got the gist, missed points or fumbled. One more pass.
- **Cold-miss** — couldn't answer. Back to the syllabus for that topic.

Re-test the Shaky/Cold-miss set weekly. You're interview-ready on a topic when every question under it is **Cold** two sessions in a row.
