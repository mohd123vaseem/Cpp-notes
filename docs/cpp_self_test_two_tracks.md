# C++ Self-Test — Two Tracks

Companion to the two-track syllabus. Numbering matches it exactly (Part A = core C++; Part B = browser add-on).

**How to use:** read each question, answer it *out loud* as if in an interview, then check the "hits" line. If you can't cover those points fluently, that topic isn't cold — send it back to the study pile.

| Target | Test yourself on |
|--------|------------------|
| Generic / FAANG / systems C++ | **Part A only** |
| Browser / isolation / rendering | **Part A + Part B** |

> Checkpoints are deliberately terse — a grading rubric, not model answers. The goal is to expose gaps, not to read. DSA is your strength (revision track) and isn't tested here.

---
---

# PART A — CORE C++

## A-P0 — Core

### 1. Memory & Object Model
1. **What is RAII and why does it matter?**
   *Hits:* resource tied to object lifetime, acquired in constructor / released in destructor, automatic cleanup even on exceptions, no manual `delete`.
2. **Walk me through the rule of 3 / 5 / 0. When do you need each?**
   *Hits:* define one of destructor/copy-ctor/copy-assign → usually need all three (rule of 3); add move-ctor/move-assign for rule of 5; rule of 0 = design so you need none.
3. **Stack vs heap — what lives where, and the trade-offs?**
   *Hits:* stack = automatic lifetime, fast, limited, LIFO; heap = manual/dynamic, larger, slower, fragmentation, must free.
4. **Order of constructor/destructor calls with members + a base class?**
   *Hits:* base before derived (ctor); members in declaration order; destruction in exact reverse.

### 2. Move Semantics
1. **Difference between `std::move` and `std::forward`?**
   *Hits:* `move` = unconditional cast to rvalue; `forward` = conditional cast preserving value category, for perfect forwarding.
2. **Does `std::move` actually move anything?**
   *Hits:* no — just a cast; the move happens when a move-ctor/assign is invoked.
3. **When does a move happen vs silently fall back to a copy?**
   *Hits:* move when source is rvalue and a move op exists; falls back to copy if no move op, move isn't `noexcept` (containers), or object is const.
4. **What is RVO/NRVO and how does it interact with move?**
   *Hits:* compiler elides copy/move on return; mandatory C++17 for prvalues; don't `std::move` a local on return — can disable NRVO.

### 3. Smart Pointers
1. **`unique_ptr` vs `shared_ptr` — when each, cost difference?**
   *Hits:* unique = sole ownership, zero overhead; shared = shared ownership via control block + atomic refcount (heavier).
2. **How does `shared_ptr` refcounting stay thread-safe? Is the object thread-safe too?**
   *Hits:* control block's refcount is atomic; the managed object is NOT protected — concurrent mutation needs your own sync.
3. **What problem does `weak_ptr` solve?**
   *Hits:* breaks reference cycles; non-owning observer; must `lock()` to use.
4. **`make_shared` vs `new` + `shared_ptr` — why prefer it?**
   *Hits:* single allocation (object + control block), exception-safe; downside: memory held until last weak_ptr dies.
5. **(Stretch) Sketch a minimal `shared_ptr`.**
   *Hits:* pointer + heap control block with count, increment on copy, decrement on destroy, delete at zero.

### 4. Concurrency & Memory Model
1. **Data race vs race condition vs deadlock — define each.**
   *Hits:* data race = unsynchronized concurrent access, ≥1 write (UB); race condition = correctness depends on timing; deadlock = circular wait on locks.
2. **`mutex` vs `atomic` — when each?**
   *Hits:* atomic for single-variable lock-free ops; mutex for larger critical sections / multi-variable invariants.
3. **Memory ordering: relaxed vs acquire-release vs seq_cst?**
   *Hits:* relaxed = atomicity only; acquire-release = synchronizes paired ops; seq_cst = single total order, strongest + slowest (default).
4. **What's false sharing and how do you fix it?**
   *Hits:* two threads write different vars on the same cache line → contention; fix with padding/alignment to separate lines.
5. **How do you avoid deadlock with multiple locks?**
   *Hits:* consistent lock ordering, `std::scoped_lock`/`std::lock`, timeouts, avoid holding locks across calls.

---

## A-P1 — Differentiators

### 5. const Correctness
1. **`const int*` vs `int* const` vs `const int* const`?**
   *Hits:* read right-to-left — pointer to const int / const pointer to int / const pointer to const int.
2. **What does a `const` member function guarantee, and what's `mutable` for?**
   *Hits:* won't modify observable state (can't change non-mutable members, can't call non-const members); `mutable` exempts a member (cache, lock).

### 6. Polymorphism Internals
1. **What happens in memory when you call a virtual function?**
   *Hits:* object holds vptr → class vtable → indirect lookup of the function pointer → call; one extra indirection.
2. **Why must a base class destructor be virtual?**
   *Hits:* `delete base_ptr` to a derived object skips the derived destructor → leak / UB.
3. **What is object slicing?**
   *Hits:* copying a derived object into a base value drops the derived part; happens with by-value passing/assignment.
4. **What's the pimpl idiom and why use it?**
   *Hits:* hide impl behind an opaque pointer; reduces compile deps / ABI stability / faster builds.

### 7. Exception Handling & Exception Safety
1. **Define the three exception-safety guarantees.**
   *Hits:* basic = no leak, valid-but-unspecified state; strong = commit-or-rollback; nothrow = never throws.
2. **What does `noexcept` do and why does it matter for moves?**
   *Hits:* promises no throw; containers only move (vs copy) on realloc if the move is `noexcept`, else copy for safety.
3. **Why never let an exception escape a destructor?**
   *Hits:* during unwinding, two live exceptions → `std::terminate`.
4. **How does RAII relate to exception safety?**
   *Hits:* unwinding runs destructors → RAII guarantees cleanup, no leaks on the throw path.

### 8. Casting & RTTI
1. **Name the four casts and when each is appropriate.**
   *Hits:* static (related types, compile-checked), dynamic (safe down-cast in polymorphic hierarchy, runtime-checked), const (add/remove const), reinterpret (bit reinterpretation, dangerous).
2. **How does `dynamic_cast` work and what's its cost?**
   *Hits:* uses RTTI to check actual type at runtime; null (pointers) / throws (refs) on failure; runtime overhead, needs polymorphic type.
3. **Why are C-style casts discouraged?**
   *Hits:* silently try several casts incl. reinterpret/const — unclear intent, unsearchable, unsafe.

### 9. STL Internals & Complexity
1. **`map` vs `unordered_map` — internals and when to choose each.**
   *Hits:* map = red-black tree, ordered, O(log n); unordered_map = hash buckets, avg O(1) / worst O(n); choose by ordering need + worst-case tolerance.
2. **What happens to `vector` capacity as it grows? Effect on iterators?**
   *Hits:* geometric growth; reallocation copies/moves elements → invalidates all iterators/pointers/refs.
3. **Which operations invalidate iterators for vector vs map/list?**
   *Hits:* vector — realloc invalidates all, erase invalidates from the point on; node-based — only the erased element's iterator.

### 10. Systems Fundamentals (OS layer)
1. **Process vs thread — what's shared, what isn't?**
   *Hits:* threads share address space/heap/fds, own stack/registers; processes isolated → IPC needed.
2. **What does a cache miss cost, and how does data layout affect it?**
   *Hits:* miss → fetch from slower level/RAM (tens–hundreds of cycles); contiguous/aligned data improves locality.
3. **Name IPC mechanisms and a trade-off of each.**
   *Hits:* pipes (simple, stream), shared memory (fastest, needs sync), sockets (network-capable, overhead), message queues.
4. **What is virtual memory / paging buying you?**
   *Hits:* isolation, illusion of large contiguous space, demand paging, protection.

---

## A-P2 — Deeper Rounds

### 11. Templates & Generic Programming
1. **Function vs class template; what's specialization?**
   *Hits:* templates generate code per type at compile time; full/partial specialization = custom impls for specific types.
2. **What is SFINAE / when do you use type traits?**
   *Hits:* substitution failure isn't an error → overload selection by type properties; `enable_if`, `is_*` traits, or Concepts (C++20).
3. **What's CRTP and why use it?**
   *Hits:* derived passes itself as template arg to base → static polymorphism, no vtable cost.

### 12. Memory Alignment & Padding
1. **Why does this struct take more bytes than the sum of its members?**
   *Hits:* padding so members sit on aligned addresses; reorder largest-to-smallest to shrink.
2. **What do `alignas` / `alignof` do, and when does alignment matter?**
   *Hits:* query/force alignment; matters for performance (cache lines, SIMD) and correctness on strict-alignment hardware.

### 13. Compilation & Linking Model
1. **Walk me from source files to an executable.**
   *Hits:* preprocess → compile each TU to object file → link (resolve symbols) → executable.
2. **What is the ODR? Give a way to violate it.**
   *Hits:* each entity defined once (inline/templates excepted); duplicate non-inline definition in two TUs violates it.
3. **Internal vs external linkage — how do you control it?**
   *Hits:* internal = TU-local (`static` at file scope, anonymous namespace); external = visible across TUs (default for non-const globals/functions).

### 14. Modern C++ Fluency
1. **Lambda capture by value vs reference, and the dangling trap?**
   *Hits:* `[=]` copies, `[&]` references; capturing by ref a local that outlives scope → dangling.
2. **`std::optional` / `std::variant` — what problems do they solve?**
   *Hits:* optional = maybe-a-value without sentinels/nulls; variant = type-safe union.
3. **What does `std::function` cost vs a raw pointer or lambda?**
   *Hits:* type-erased, may heap-allocate, indirect call — convenient but not free.

### 15. `volatile` vs `atomic`
1. **Does `volatile` make a variable thread-safe?**
   *Hits:* no — only blocks certain compiler optimizations; no atomicity or cross-thread ordering. Use `std::atomic`.
2. **When is `volatile` actually right?**
   *Hits:* memory-mapped hardware I/O, `sig_atomic_t` signal-handler flags.

### 16. Operator Overloading
1. **When should an operator be a member vs free function?**
   *Hits:* member when it needs the left operand to be the class + internal access; free (often friend) for symmetry, esp. when left operand may differ (e.g. `<<`).
2. **Rule of thumb for overloading?**
   *Hits:* behave like the built-in / least-surprise; don't overload to be clever.

### 17. Undefined Behavior
1. **Name several sources of UB and what the compiler may do.**
   *Hits:* use-after-free, dangling refs, uninitialized reads, signed overflow, OOB, strict aliasing; compiler assumes UB never happens → reorder/delete code.
2. **Why does UB exist at all?**
   *Hits:* lets the compiler optimize without checking impossible cases; the cost of C++'s performance model.

---
---

# PART B — BROWSER / ISOLATION ADD-ON
### (test only when targeting browser-niche companies)

## B-P0 — Browser Core

### B1. Async, Callbacks & Ownership (browser dialect)
1. **A callback captures `this`, the object is destroyed, then the callback fires — what happens, and how do you prevent it?**
   *Hits:* use-after-free / UB; prevent with a weak pointer that invalidates on destroy (callback becomes a no-op), or bind ownership so the object outlives the callback.
2. **What's a task-based / message-loop concurrency model, and how does it differ from raw threads + locks?**
   *Hits:* work posted as tasks to a loop/sequence, run one-at-a-time on that sequence → no shared-state locking needed on that sequence; async, event-driven.
3. **`base::WeakPtr`-style weak pointer vs `std::weak_ptr` — what's the difference?**
   *Hits:* intrusive, single-threaded/sequence-bound, auto-invalidates when the owner dies (no `lock()`/refcount); `std::weak_ptr` is thread-safe, tied to `shared_ptr`'s control block, needs `lock()`.
4. **Intrusive refcounting (`scoped_refptr`/`RefCounted`) vs `shared_ptr` — trade-offs?**
   *Hits:* intrusive = count lives inside the object (one allocation, count survives raw-pointer round-trips); `shared_ptr` = external control block, can't safely recreate from a raw pointer.
5. **One-shot vs repeating callbacks — why does the distinction matter?**
   *Hits:* one-shot can own/move its state and run once (safe to consume resources); repeating must remain valid across many calls (stricter lifetime rules).

### B2. Multi-Process Architecture & IPC Depth
1. **Why is a browser multi-process? What breaks if it's single-process?**
   *Hits:* stability (one tab crash doesn't kill all) + security (sandboxed renderers); single-process = one crash/exploit takes everything.
2. **Name the process types and their roles.**
   *Hits:* browser (privileged, coordinates), renderer (untrusted, per-site, runs web content), GPU, network, utility.
3. **How do two processes communicate, and how does structured data cross safely?**
   *Hits:* IPC via Mojo; messages serialized/marshalled across the boundary, validated on receipt (never trust the renderer side).
4. **What is the sandbox and what does it protect against?**
   *Hits:* restricts the untrusted renderer's OS access → an exploited renderer still can't touch the filesystem/kernel directly; escapes require chaining a sandbox bug.
5. **What is site isolation?**
   *Hits:* separate process per site-instance so a compromised renderer can't read another site's data (Spectre-class + cross-site protection).

---

## B-P1 — Browser Differentiators

### B3. Browser Security / Isolation Concepts
1. **What makes the DOM an attack surface?**
   *Hits:* parses/executes untrusted remote content (HTML/JS/CSS) → memory-safety bugs and logic flaws become exploit vectors.
2. **RBI: DOM mirroring vs pixel streaming — trade-offs?**
   *Hits:* pixel streaming = send images (safe, higher latency/bandwidth, worse fidelity); DOM mirroring = re-render a sanitized DOM to the endpoint (better fidelity/interactivity, must strip active content safely).
3. **How does isolating web content in the cloud stop endpoint compromise?**
   *Hits:* active code executes remotely; only safe display data reaches the device → malware never touches the endpoint.

### B4. Rendering Pipeline (light)
1. **Walk the pipeline from HTML to pixels.**
   *Hits:* parse → DOM/CSSOM → layout → paint → composite; GPU process does the final compositing.
2. **What does the GPU process do and why is it separate?**
   *Hits:* isolates GPU driver crashes/exploits; handles compositing/rasterization off the main renderer.

---

## Narrative Self-Test — Your Chromium Story
Not technical recall — rehearse until smooth. Record yourself; aim for clear, structured 60–90 sec answers.

1. **Why is a browser multi-process? What breaks if it's single-process?**
2. **Walk me through how two Chromium processes communicate.** (Mojo / IPC)
3. **Tell me about a hard bug you debugged.** (race / leak / use-after-free — symptom → investigation → root cause → fix.)
4. **Trickiest memory or lifetime problem you hit, and how you reasoned about it?**
5. **How is work scheduled across threads/processes in the browser?**

> One concrete, specific war story beats five textbook answers. Have at least two ready.

---

## Scoring Yourself
Rate each question honestly:
- **Cold** — answered fully, fluently, unprompted. Move on.
- **Shaky** — got the gist, missed points or fumbled. One more pass.
- **Cold-miss** — couldn't answer. Back to the syllabus for that topic.

Re-test Shaky/Cold-miss weekly. A topic is interview-ready when every question under it is **Cold** two sessions running.
