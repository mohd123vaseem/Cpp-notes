# Topic 9 — Concurrency & Multithreading

Tier 2 #9 (~3 days). **Not a strength for me — teach thoroughly from basics.** High-frequency in interviews ("mutex vs atomic," "what causes a deadlock," "make this thread-safe"). My Chromium background is a *narrative* asset once the fundamentals are solid.

> Deep memory model (`memory_order`, happens-before, false sharing) is **Tier 3 #14**, not here. This topic covers threads, synchronization tools, and the core concepts/pitfalls.

## Why this topic matters

Modern CPUs have many cores; to use them, programs run multiple **threads** at once. But shared data + simultaneous access = **bugs that don't exist in single-threaded code** (data races, deadlocks) — subtle, timing-dependent, and hard to reproduce. This topic is about running things in parallel *correctly*.

---

## Progress tracker (sub-topics)

### Part A — Fundamentals
| # | Sub-topic | The classic question | Status |
|---|-----------|----------------------|--------|
| 1 | **Why concurrency / what a thread is** (process vs thread) | "Process vs thread?" | ✅ Done |
| 2 | **`std::thread`** — create, `join()` vs `detach()`, lifecycle | "What if you don't join a thread?" | ✅ Done |
| 3 | **The core problem: data races** | "What is a data race?" | ✅ Done |

### Part B — Synchronization tools
| # | Sub-topic | The classic question | Status |
|---|-----------|----------------------|--------|
| 4 | **`mutex` + `lock_guard`** (RAII locking) | "How does a mutex work?" | ✅ Done |
| 5 | **`unique_lock` vs `lock_guard` vs `scoped_lock`** | "When each?" | ✅ Done |
| 6 | **`condition_variable`** + producer-consumer + spurious wakeups | "Why `while` not `if` around `cv.wait`?" | ✅ Done |
| 7 | **`std::atomic`** + CAS (compare-and-swap) | "mutex vs atomic?" | ⬜ Pending |
| 8 | **`shared_mutex`** (reader-writer lock) | "Many readers, one writer?" | ⬜ Pending |
| 9 | **`std::call_once` / `once_flag`** (thread-safe init) | "Thread-safe singleton?" | ⬜ Pending |

### Part C — Concepts & pitfalls
| # | Sub-topic | The classic question | Status |
|---|-----------|----------------------|--------|
| 10 | **Data race vs race condition** | "Difference between them?" | ⬜ Pending |
| 11 | **Deadlock** (4 conditions, avoidance) + **livelock & starvation** | "What causes a deadlock, how to prevent?" | ⬜ Pending |
| 12 | **`volatile` vs `atomic`** | "Can I use `volatile` for thread sync?" | ⬜ Pending |
| 13 | **Thread-safe design** | "Make this class thread-safe." | ⬜ Pending |
| 14 | **`async` / `future` / `promise`** (lighter) | "What's `std::async`?" | ⬜ Pending |

> Deep memory model / `memory_order` / false sharing → **Tier 3 #14** (later).

---

## Sub-topic 1 — Why Concurrency / What a Thread Is

### Simple: what concurrency means
**Concurrency = doing multiple things at (or appearing to be at) the same time.** Several tasks make progress together instead of strictly one-after-another.
```
Single-threaded: [--- A ---][--- B ---][--- C ---]   (sequential)
Multi-threaded:  [--- A ---]
                 [--- B ---]   (progressing together)
                 [--- C ---]
```

### The WHY (two real needs)
1. **Use all CPU cores (speed).** ~2005 CPUs stopped getting faster (clock limits) and went **wide** — more cores (8, 16, …). A **single-threaded program uses only ONE core** → wastes the rest. To use the hardware you must split work across **threads**, one per core = **parallelism**.
2. **Stay responsive (don't freeze on slow work).** Slow ops (file/network/DB) would freeze a single-threaded program while waiting. Threads let one wait while others keep working. *(Browser: one thread downloads a page while another keeps the tab scrolling.)*

### What is a thread? (process vs thread)
- **Process** = a running program with its **own private, isolated memory** (own heap/address space).
- **Thread** = an execution path **inside** a process. A process starts with one thread (`main`) and can spawn more. All threads in a process **share the same memory** (heap, globals) but each has its **own stack**.
```
   PROCESS (isolated memory)
 ┌───────────────────────────────────────┐
 │  Shared: heap, globals, code           │
 │  Thread 1     Thread 2     Thread 3    │
 │  (own stack)  (own stack)  (own stack) │  ← own stacks, SHARED heap
 └───────────────────────────────────────┘
```

| | Process | Thread |
|---|---|---|
| Memory | **isolated** (own address space) | **shared** with other threads |
| Communication | hard (IPC: pipes, sockets) | easy (shared variables) |
| Create cost | heavy | light |
| Crash impact | isolated | one thread crash usually kills the whole process |
| Owns | everything | **just its own stack** (+ registers) |

### 🔑 Shared memory = the power AND the danger
- Threads share the heap → **cheap communication** (just read/write shared variables) — the power.
- Threads share the heap → **two threads can touch the same variable at once** → **data races** (sub-topic 3) — the danger.
> The rest of this topic = "shared memory lets threads cooperate cheaply, but unsynchronized shared access corrupts data — so here are the tools (mutex, atomic, condition_variable) to share **safely**."

### Summary
- Concurrency = multiple tasks progressing together.
- **Why:** use all cores (speed) + stay responsive (don't freeze on I/O).
- **Process** = isolated memory, heavy. **Thread** = shares the process's heap/globals, own stack only, light.
- **Shared memory is the central tension** — cheap communication, but unsynchronized access → data races (what the topic solves).

---

## Sub-topic 2 — `std::thread`: Creating Threads, `join()` vs `detach()`

### Simple: launching a thread
`std::thread` (`<thread>`) runs a function on a **new thread**, in parallel with the current one:
```cpp
#include <thread>
void sayHello() { std::cout << "Hello from a new thread!\n"; }

int main() {
    std::thread t(sayHello);   // launches sayHello() on a NEW thread — starts immediately
    // main keeps running here, at the same time as t
    t.join();                  // wait for t to finish
}
```
Also takes arguments and lambdas: `std::thread t(work, 42, "hi");` · `std::thread t([]{ ... });`

### 🔑 Two different things: the thread OBJECT vs the underlying THREAD
- **`std::thread` object** = a small **C++ handle** on your stack — like a **taxi booking receipt**. Doesn't do the work itself; it *represents/controls* the real thread.
- **underlying thread** = the actual **OS-level worker** the OS schedules onto a CPU core to *actually run* your function — the **taxi out on the road**.
```cpp
std::thread t(work);
//           ↑         ↑
//   the C++ OBJECT     the OS creates a real THREAD running work()
//   (handle 't')       (the actual worker)
```
The receipt (`t`) is **not** the taxi — it just controls it. Throwing away the receipt doesn't stop the taxi.

### `join()` vs `detach()` — dealing with the taxi
Before the `std::thread` object is destroyed, you MUST say what happens to its still-running underlying thread:
- **`join()`** — "wait here until the taxi returns" → **block** until the thread finishes, then continue. *(the common case)*
- **`detach()`** — "let the taxi drive off on its own forever" → thread runs independently in the background; you give up waiting/results. *(rare, fire-and-forget)*

### When does the destructor run?
`std::thread`'s destructor runs when the object goes **out of scope** (`}`) — and *that's* where the join-or-detach check happens:
```cpp
void bad() {
    std::thread t(work);   // receipt created; a real taxi (OS thread) starts running work()
    // ❌ never said join() or detach()
}   // '}' → t's DESTRUCTOR runs → checks "did you deal with the taxi?" → NO → CRASH
```

### ⚠️ THE rule (classic interview Q): join or detach, or crash
If a `std::thread` is destroyed while still **"joinable"** (a live taxi attached that you never handled) → C++ calls **`std::terminate()` → whole program crashes.** ("Joinable" = created but not yet joined/detached.)

**Why crash instead of guessing?** Both silent alternatives hide dangerous bugs:
- **Silently detach** → thread keeps running, but the local variables it used (on the now-destroyed stack) are gone → **use-after-free** chaos.
- **Silently join** → the `}` secretly **blocks** for an unknown time → surprising, can deadlock.

So C++'s philosophy: **don't guess, fail loudly.** Crashing immediately makes you fix the forgotten-thread bug instead of shipping a hidden race/hang.
> Interview answer: *"If you don't join() or detach() before the `std::thread` object is destroyed, its destructor calls `std::terminate()`. C++ forces an explicit choice because both silent alternatives (auto-detach → use-after-free, auto-join → hidden blocking) would hide dangerous bugs."*

### Exception-safety problem + the fix
A throw before `join()` skips it → destroyed joinable → terminate:
```cpp
std::thread t(work);
mightThrow();     // ⚠️ if this throws, we never reach join → terminate
t.join();
```
Fix: an RAII thread-guard (joins in its destructor), or **`std::jthread`** (C++20) which **auto-joins** in its destructor — the RAII-correct version, prefer it:
```cpp
std::jthread t(work);   // auto-joins at scope end, even on exception
```

### 💡 The object is a HANDLE; the thread is an OS-owned RESOURCE (deeper why)
The underlying thread is **not a C++ object** — it's a resource **owned and managed by the OS**. The `std::thread` object is just a **handle** (holds an ID) referring to it.
```
YOUR PROGRAM (C++ land)         OS land
 std::thread t ──(ID)──►         the real thread (own stack, scheduled on a core)
 governed by C++ scope ('}')     governed by the OS, NOT your scope
```
**Same pattern as a raw pointer → heap object** (Topic 1): `p` going out of scope destroys the *pointer*, not the heap `int` (that's a leak!). Likewise, the `std::thread` object going out of scope destroys **only the handle**, not the OS thread — because C++ scope only governs C++ objects, and the OS owns the thread's lifetime. (It survives because the **OS owns it**, not because of heap-vs-stack.)

That's *why* `terminate` exists: a forgotten thread would be like a "leaked" running thread (worse than a leaked int → use-after-free on the dead stack). So instead of allowing it silently, C++ forces you to `join`/`detach`.

### Who destroys the thread, and when? (the OS always does — 2 triggers)
| Trigger | What happens |
|---|---|
| **Thread's function returns** (natural) | thread finished → OS reclaims it cleanly |
| **Process exits** (`main` returns) | OS abruptly kills ALL remaining threads (detached too), mid-work |
| **`std::thread` object goes out of scope (`}`)** | destroys **only the handle**, NOT the thread → this is why you must join/detach |

> The **OS** always destroys the thread — triggered by the **function finishing** (clean) or the **process exiting** (abrupt). The **handle's scope ending is a separate event** that only affects the handle.

### Detached thread — lifetime & danger
- After `detach()`, the object and the OS thread are **divorced**: the object's destructor at `}` does nothing to the thread; the thread runs independently.
- **But** a detached thread dies when the **process exits** (`main` returns) — abruptly, maybe mid-work. *(That's why the demo needs a `sleep` to keep `main` alive long enough for a detached thread to print — a hack, not real sync.)*
- ⚠️ **Detach is dangerous** mainly due to **lifetime bugs**: a detached thread can **outlive the data it uses** (e.g. captures a local by reference; the function returns; the local is destroyed; the thread reads dead memory → **use-after-free**). You also lose all control/visibility over it.
- **Prefer `join()` / `std::jthread`.** Detach only for carefully-managed fire-and-forget; many codebases ban it.

### 💡 Concrete scenario: main → solve() → thread (when does it end?)
Two DIFFERENT functions — don't confuse them:
```cpp
void work()  { /* the function the THREAD runs */ }
void solve() { std::thread t(work); /* solve LAUNCHES the thread */ }
int  main()  { solve(); }
```
- **`solve`** = the *launcher* (creates the thread). **`work`** = the *worker* (what the thread runs).
- **The thread's life is tied to `work` finishing — NOT `solve` finishing.**

**What is a "process"?** One run of your whole program: begins when the OS launches the `.exe`, ends when **`main` returns**. `solve` returning is NOT the process ending — control just goes back to `main`; the program keeps running.

**When does the thread end?** Depends on what `solve` does with `t`:
| `solve` does | Result |
|---|---|
| **`t.join()`** | `solve` WAITS until `work()` finishes, then returns. Thread ends when `work` ends. |
| **`t.detach()`** | `solve` returns immediately; thread keeps running `work()` in the background → **outlives `solve`**. Ends when `work()` finishes OR when **`main` returns (process exits)**. |
| **neither** | at `solve`'s `}`, `t` is destroyed while joinable → **`std::terminate()` → crash**. |

**Endless-loop `work()`?** join → `solve` hangs forever; detach → loops in background until **`main` returns (process exit)** kills it.

```
main starts (PROCESS begins)
  ├─ solve(): create thread → work() starts;  t.detach();  solve RETURNS ┐
  ├─ back in main...                            work() STILL running ─────┤ (outlived solve!)
  └─ main RETURNS → PROCESS ends → OS kills work() if still running ──────┘
```
> Thread runs **`work`** (not `solve`); dies when **`work` finishes** or the **process (`main`) exits**. `solve` ending only matters if it `join()`ed (waited) or forgot to handle `t` (crash).

### Getting a result back
`std::thread` discards the function's return value. Options: write to a **shared variable** (needs sync) or use **`std::async`/`future`** (sub-topic 14, the clean way).

### Summary
- **`std::thread t(fn, args...)`** launches `fn` on a new thread immediately.
- **Object** = handle (receipt); **underlying thread** = the real OS worker (taxi). Object ≠ thread.
- Before the object is destroyed: **`join()`** (wait) or **`detach()`** (run free). Destructor runs at `}`.
- ⚠️ Neither → **`std::terminate()` → crash** (both silent defaults hide dangerous bugs).
- Not exception-safe → use RAII / **`std::jthread`** (C++20 auto-joins).
- Return values via shared variable (+sync) or `std::async`/`future`.

---

## Sub-topic 3 — The Core Problem: Data Races

### Simple: what a data race is
> **2+ threads access the same memory at the same time, ≥1 is writing, and there's no synchronization.**

Result = **undefined behavior**: corrupted values, wrong answers, crashes — and it's **timing-dependent** (works 99 times, fails the 100th).

### The WHY: the "simple" operation that isn't
```cpp
int counter = 0;
void increment() { counter++; }   // looks like ONE step...
```
`counter++` is actually **three** steps at the CPU level (read-modify-write):
```
1. READ  counter → register
2. ADD   1 to register
3. WRITE register → counter
```
The OS can **pause a thread between any of these steps** and run another. That gap is where it breaks.

### The race in action (lost update)
Two threads run `counter++` (start 0), expecting 2:
```
Thread A: READ counter (0)
Thread B: READ counter (0)     ← read before A wrote back!
Thread A: ADD → 1
Thread B: ADD → 1
Thread A: WRITE → counter = 1
Thread B: WRITE → counter = 1  ← overwrites A's work
Final: counter = 1   ❌ (should be 2 — one increment LOST)
```
Run a million increments across two threads → final count is **well below** two million. Some updates silently vanish. It's **non-deterministic** → appears randomly, hard to reproduce (a "heisenbug"). And it's **UB**, not just "a slightly-off number" — torn values, crashes, or the compiler assuming no race all possible.

### Root cause + fixes
Root cause = **unsynchronized access to shared memory** (sub-topic 1's tension). Two fixes (rest of the topic):
1. **Make the op atomic** — `std::atomic<int> counter;` → `counter++` becomes one indivisible hardware op (sub-topic 7).
2. **Serialize with a lock** — a `mutex` so only one thread touches `counter` at a time (sub-topics 4–5).
Both remove "unsynchronized simultaneous access."

### Summary
- **Data race** = concurrent same-memory access, ≥1 write, no sync → **UB** (lost updates, corruption, crashes).
- **Why:** ops like `counter++` are multiple steps (read→modify→write); a thread paused mid-way lets another interleave → **lost update**.
- Non-deterministic → heisenbug.
- **Fix:** make it **atomic**, or **serialize** with a **mutex**.

---

## Sub-topic 4 — `mutex` + `lock_guard`

### Simple: what a mutex is
A **mutex** (`<mutex>`, "**MUT**ual **EX**clusion") is a lock ensuring **only one thread at a time** touches shared data. Lock before touching; if another thread holds it, you **wait**; unlock when done.
```
Thread A: lock ─[ touches data safely ]─ unlock
Thread B:      lock (waits...) ───────────────► acquired ─[ its turn ]─ unlock
```
The protected region = the **critical section**. One thread inside at a time → no simultaneous access → no data race.

### The WHY
The race (sub-topic 3) happened because two threads did `counter++`'s read-modify-write *at the same time*. A mutex guarantees once a thread starts the critical section, it **finishes before any other thread enters** — turning a multi-step op into an uninterruptible section.

### Manual lock/unlock — works but dangerous
```cpp
std::mutex m;
void increment() {
    m.lock();
    if (somethingBad()) return;   // ❌ returns WITHOUT unlocking → mutex stuck forever → deadlock
    counter++;
    m.unlock();                   // skipped on the early-return path
}
```
Same "manual cleanup is error-prone" flaw as raw `new`/`delete` (Topic 1).

### The fix: `lock_guard` (RAII for a mutex) ✅
Locks in its constructor, **auto-unlocks in its destructor**:
```cpp
std::mutex m;
void increment() {
    std::lock_guard<std::mutex> guard(m);   // locks here
    counter++;                              // critical section
}   // ← guard destructor UNLOCKS automatically
```

### 🔑 How does it unlock on an early return? (statement vs destructor)
Key rule: **`return` does NOT skip destructors.** Leaving a scope by ANY path (early return, exception, normal end) **always runs local objects' destructors** as part of leaving.
| | Where unlock lives | On early `return` |
|---|---|---|
| **Manual** | a **statement** further down | **skipped** (control jumps past it) ❌ |
| **`lock_guard`** | the guard's **destructor** | **runs** (scope exit always destroys locals) ✅ |
A `return` can jump over *statements*, but never over *destructors of locals*. **Same mechanism that freed your `vector` on early return in Topic 1** — `lock_guard` is "RAII for a lock."

### Cost / design
Mutexes have overhead + serialize (others wait → less parallelism). So:
- **Keep critical sections small** — lock only around the shared access, not unrelated work.
- For a single value, an **atomic** (sub-topic 7) is cheaper; mutexes are for protecting **larger / multi-variable** operations atomics can't.

### Summary
- **Mutex** = only one thread in the critical section at a time → no data race.
- Manual `lock`/`unlock` is dangerous (early return/exception skips unlock → deadlock).
- **`lock_guard`** = RAII: locks in ctor, unlocks in dtor on **every** exit path (incl. exceptions). Prefer it.
- Keep critical sections **small**; single value → prefer `atomic`.

---

## Sub-topic 5 — `unique_lock` vs `lock_guard` vs `scoped_lock`

All three are **RAII lock wrappers** (lock in ctor, unlock in dtor). They differ in **flexibility** — pick the simplest that does the job.

| Wrapper | Is | Use when |
|---|---|---|
| **`lock_guard`** | simplest, "lock and forget" | just lock a scope (default, ~90%) |
| **`unique_lock`** | the flexible one | early unlock / re-lock / defer / move, or a `condition_variable` |
| **`scoped_lock`** (C++17) | the multi-mutex one | locking **2+ mutexes** at once (deadlock-free) |

### `lock_guard` — the default
Locks on construction, unlocks on destruction, **nothing else** → cheapest.
```cpp
std::lock_guard<std::mutex> g(m);   // lock; auto-unlock at scope end
```

### `unique_lock` — the flexible one
Everything `lock_guard` does, **plus**:
```cpp
std::unique_lock<std::mutex> lk(m);
lk.unlock();   // release EARLY (let others in)
lk.lock();     // re-acquire later
std::unique_lock<std::mutex> lk2(m, std::defer_lock);  // construct WITHOUT locking; lock later
```
Also movable. Slightly heavier (tracks whether it holds the lock) → use only when needed.

**#1 reason it exists: `condition_variable`.** To wait, a CV must **temporarily unlock** the mutex (so another thread can change the condition), then **re-lock** on wakeup. That unlock-and-relock needs `unique_lock` — `lock_guard` can't do it. So `cv.wait()` requires `unique_lock`.

### `scoped_lock` — multiple mutexes, deadlock-free (C++17)
Locks 2+ mutexes at once using a deadlock-avoidance algorithm.

**The problem it solves — multi-mutex deadlock:**
```cpp
std::mutex m1, m2;
// Thread A:                         // Thread B:
std::lock_guard g1(m1);              std::lock_guard g1(m2);   // ⚠️ locks in OPPOSITE order
std::lock_guard g2(m2);             std::lock_guard g2(m1);
// A holds m1, waits for m2  ┐
// B holds m2, waits for m1  ┘  → circular wait → DEADLOCK (both stuck forever)
```
**The fix:**
```cpp
// Both threads:
std::scoped_lock lock(m1, m2);   // ✅ locks BOTH atomically, no ordering deadlock
```
`scoped_lock` acquires all the mutexes together without the circular-wait problem. Whenever you hold **more than one** mutex, use it — never hand-lock them one by one. *(With a single mutex it's basically `lock_guard`.)*

### Decision guide
```
One mutex, simple?                         → lock_guard  (default)
Early unlock / re-lock / defer / move / CV? → unique_lock
2+ mutexes at once?                          → scoped_lock (deadlock-safe)
```

### Summary
- All RAII wrappers; differ in flexibility → use the simplest that works.
- **`lock_guard`** = default, lock-and-forget.
- **`unique_lock`** = flexible (unlock/re-lock/defer/move); **required by `condition_variable`**.
- **`scoped_lock`** = lock **multiple mutexes deadlock-free**.

---

## Sub-topic 6 — `condition_variable`

### The problem: waiting without wasting CPU
A thread often needs to **wait until something becomes true** (e.g. a worker waiting for an item in a queue). Without a CV you'd **busy-wait** (spin):
```cpp
while (queue.empty()) { /* check... check... check... */ }   // ❌ burns 100% CPU doing nothing
```
A **`condition_variable`** (`<condition_variable>`) lets a thread **sleep (zero CPU)** until another thread **signals** it. *"Don't call me, I'll call you."*
- **`wait()`** — go to sleep until notified.
- **`notify_one()` / `notify_all()`** — wake one / all waiters.

### Real-world picture (restaurant)
- **Producer = chef** — cooks dishes, puts them on a counter.
- **Consumer = waiter** — takes dishes to customers.
- **counter = shared queue.** When no dishes are ready, the waiter **waits**; the chef **rings a bell** (the CV) when a dish is ready.

### Complete producer-consumer (two real threads)
```cpp
queue<int> q;                  // shared counter (dishes)
mutex m;                       // protects the queue
condition_variable cv;         // the "bell"

void chef() {                                  // PRODUCER
    for (int dish = 1; dish <= 5; dish++) {
        this_thread::sleep_for(chrono::milliseconds(500));   // cooking takes time
        {
            lock_guard<mutex> lock(m);
            q.push(dish);                      // put dish on the counter
        }
        cv.notify_one();                       // RING THE BELL: "a dish is ready!"
    }
}

void waiter() {                                // CONSUMER
    for (int served = 1; served <= 5; served++) {
        unique_lock<mutex> lock(m);            // unique_lock — required by wait()
        cv.wait(lock, []{ return !q.empty(); });   // sleep until a dish exists
        int dish = q.front(); 
        q.pop();         // take the dish
    }
}

int main() {
    thread t1(chef), t2(waiter);   // two threads running at once
    t1.join(); t2.join();
}
```

### Breaking down `cv.wait(lock, []{ return !q.empty(); })`
**Piece 1 — `[]{ return !q.empty(); }` is a LAMBDA** (a tiny nameless function). Equivalent to:
```cpp
bool isDishReady() { return !q.empty(); }   // true when the queue is NOT empty
```
`[]` starts the lambda (empty capture — ignore for now); `{ ... }` is the body. It answers: *"is there a dish ready? (true/false)."*

**Piece 2 — `cv.wait(lock, condition)` means "sleep until the condition is true":**
> *"Go to sleep. Keep sleeping while the queue is EMPTY. Wake up and continue only when it's NOT empty."* `wait` **calls the lambda** to decide whether to keep sleeping.

**Piece 3 — what `wait` does step-by-step:**
```
1. call the lambda → queue non-empty?  YES → continue.  NO → step 2.
2. UNLOCK the mutex and SLEEP        (so the chef CAN lock it and add a dish)
3. notified → WAKE UP, RE-LOCK the mutex
4. call the lambda AGAIN → non-empty? YES → continue (grab dish). NO → back to step 2.
```

### Why `wait` needs a `unique_lock` (sub-topic 5 payoff)
The consumer holds the mutex protecting the queue. If it slept **while holding** the lock, the producer could never lock it to add an item → sleep forever = deadlock. So `wait` **unlocks while asleep** (letting the producer work) and **re-locks on wakeup** — needing a lock that can unlock+relock = `unique_lock` (not `lock_guard`).

> One line: `cv.wait(lock, []{ return !q.empty(); })` = *"sleep here until the queue has something; check via this little function; if still empty keep sleeping; when there's an item, wake and continue."* The lambda is just the condition you're waiting for.

### notify_one vs notify_all
- **`notify_one()`** — wake **one** waiter (cheaper). Use when only one can proceed (one item → one consumer).
- **`notify_all()`** — wake **all** waiters. Use when several could proceed, or waiters wait on different conditions.

### (Q1) How `wait` unlocks & re-locks — the internal code + timeline
It "knows" the mutex because **you pass it the `unique_lock`**; it calls `.unlock()`/`.lock()` on that object.
```cpp
// two-arg version (what you call):
void wait(unique_lock<mutex>& lock, Predicate pred) {
    while (!pred()) {   // !pred() = "queue EMPTY?" → loop = "sleep WHILE empty"
        wait(lock);
    }
}
// one-arg core:
void wait(unique_lock<mutex>& lock) {
    lock.unlock();     // (A) release
    // ===== SLEEP HERE =====   ← mutex is UNLOCKED this whole time
    lock.lock();       // (B) re-acquire — runs only AFTER waking
}
```
**Key:** `while (!pred())` = *sleep only WHILE the queue is EMPTY.* If there's an item, `!pred()` is false → loop **skipped** → no unlock, no sleep → grab the item under the lock. The unlock+sleep happens **only** when there's nothing to do.

**The re-lock (B) does NOT block the producer** — it runs *after* waking, which is *after* the producer already added the item and released its lock. During the whole sleep the mutex is FREE. Timeline:
```
TIME  CONSUMER (waiter)              PRODUCER (chef)           MUTEX
──────────────────────────────────────────────────────────────────────
 1    enters wait(), queue empty                              held by consumer
 2    (A) lock.unlock()                                       FREE ✅
 3    ...sleeping (stuck before B)...                         FREE
 4    ...sleeping...                  lock() → gets it         held by PRODUCER
 5    ...sleeping...                  q.push(item)             held by producer
 6    ...sleeping...                  unlock (lock_guard ends) FREE ✅
 7    ...sleeping...                  cv.notify_one()          FREE
 8    WAKES (because of notify)                               FREE
 9    (B) lock.lock() → gets it                               held by CONSUMER
10    re-check predicate: not empty!                          held by consumer
11    grab item, then unlock                                  FREE
```
Rows 2–7: while the consumer sleeps the mutex is **FREE**, so the producer adds the item with no blocking. (B) at row 9 runs only after the producer is done.

**Why re-lock at all?** After `wait` returns you need the lock to safely **re-check the predicate** (reads the queue) and **grab the item** (`front`/`pop` modify the queue). It holds the lock only microseconds, then releases.
> Trap to avoid: picturing "unlock → lock" as back-to-back. There's a **sleep in between**, and the producer does its work *during* that sleep while the mutex is free. The re-lock is on the far side of the sleep.

### Spurious wakeups (Q2) — and why `while`, not `if`
A **spurious wakeup** = a sleeping thread **wakes from `wait` even though NOBODY called `notify`** — it just wakes on its own.

**Why:** it's an OS/hardware reality. The low-level primitive `condition_variable` is built on (a "futex" on Linux) is *allowed* to wake spuriously, because *guaranteeing* "only wake on a real notify" would make the common case slower. So the standard says: **`wait` may return without a notification.** A wakeup does NOT mean the condition is true.

**Handled by the `while` loop** inside `wait`:
```cpp
while (!pred()) { wait(lock); }   // after waking, RE-CHECK; if still false → sleep again
```
- Spurious wakeup → re-check → still false → back to sleep. Safe.
- With `if` instead: you'd proceed on a false condition → e.g. pop an empty queue → crash.
- Also handles **stolen items**: 2 consumers wake on `notify_all`, one grabs the item; the other re-checks, sees empty, sleeps again.
> ⭐ **Rule: always re-check the condition in a `while` loop** (or use the predicate form `cv.wait(lock, pred)`, which does the loop for you). Never a bare `if`.

### 🔑 (Q3) Isn't the loop just busy-waiting? — NO (the key insight)
Both a busy-wait and the CV have a loop. The difference is **what happens between checks.**

**Busy-wait (bad):**
```cpp
while (q.empty()) { }   // checks MILLIONS of times/sec, thread keeps RUNNING → 100% of a core
```
**condition_variable (good):**
```cpp
while (!pred()) { wait(lock); }   // if false → wait() SLEEPS the thread (suspended by OS) → ~0% CPU
```
When the condition is false, the CV loop doesn't re-check immediately — `wait()` **puts the thread to sleep** (removed from the CPU scheduler → zero CPU). It only wakes (and loops back to check) on a `notify` (or rare spurious). So the loop body runs a **handful of times** (once per notify), not millions.

| | Busy-wait loop | condition_variable loop |
|---|---|---|
| Between checks | **keeps running** (spins) | **sleeps** (suspended) |
| Checks/sec | millions | a handful (per notify) |
| CPU while waiting | **100% of a core** | **~0%** |
| Woken by | nothing (never stops) | `notify` (or rare spurious) |

> Both loop — but `wait()` makes the thread **sleep (0% CPU)** between iterations instead of **spin (100%)**. The loop runs only when something happened, not continuously. *That's* how the CV saves CPU.

**Analogy:** busy-wait = shouting "ready?! ready?!" 10,000×/sec (exhausting, useless). CV = napping until the chef taps your shoulder; you check once, take the dish, nap again.

---

## Code examples in this folder

| File | Demonstrates |
|------|--------------|
| `threads_basics.cpp` | Launching threads (function + lambda), passing args, `join()`, `detach()`, interleaving proof; the forgot-join crash (commented) |
| `data_race.cpp` | Two threads increment a shared counter 1M times each → total comes out **wrong** (lost updates). The race, made visible. |
| `mutex_fix.cpp` | Same program **fixed** with `mutex` + `lock_guard` → exactly 2,000,000 every run. |
| `lock_wrappers.cpp` | `lock_guard` (default), `unique_lock` (early unlock / re-lock), `scoped_lock` (two mutexes at once) |
| `deadlock_scoped_lock.cpp` | Multi-mutex **deadlock** (opposite lock orders, guarded/hangs) + the **`scoped_lock` fix** |
| `producer_consumer.cpp` | Classic **producer-consumer** with `condition_variable`: chef produces dishes, waiter sleeps-until-notified & consumes; `done` flag to finish cleanly |
