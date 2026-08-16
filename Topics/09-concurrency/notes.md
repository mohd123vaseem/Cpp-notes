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
| 7 | **`std::atomic`** + CAS (compare-and-swap) | "mutex vs atomic?" | ✅ Done |
| 8 | **`shared_mutex`** (reader-writer lock) | "Many readers, one writer?" | ✅ Done |
| 9 | **`std::call_once` / `once_flag`** (thread-safe init) | "Thread-safe singleton?" | ✅ Done |

### Part C — Concepts & pitfalls
| # | Sub-topic | The classic question | Status |
|---|-----------|----------------------|--------|
| 10 | **Data race vs race condition** | "Difference between them?" | ✅ Done |
| 11 | **Deadlock** (4 conditions, avoidance) + **livelock & starvation** | "What causes a deadlock, how to prevent?" | ✅ Done |
| 12 | **`volatile` vs `atomic`** | "Can I use `volatile` for thread sync?" | ✅ Done |
| 13 | **Thread-safe design** | "Make this class thread-safe." | ✅ Done |
| 14 | **`async` / `future` / `promise`** (lighter) | "What's `std::async`?" | ✅ Done |

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

**`if` vs `while` in one line:** `if` checks the condition **once before sleeping**, then trusts the wakeup blindly → proceeds on a false condition → crash. `while` **re-checks every time it wakes** → if still false, sleeps again. You re-check because **a wakeup does NOT guarantee the condition is true** — it can be (1) a **spurious wakeup** (nobody notified) or (2) a **stolen item** (2 consumers wake on one `notify_all`, the first takes the item, the second must see it's gone and sleep again). A wakeup is a *hint to re-check*, not a *guarantee*.

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

## Sub-topic 7 — `std::atomic` + CAS

### Simple: atomic = indivisible
An **atomic operation** happens in **one uninterruptible step** — no thread sees it half-done, none can slip in the middle. `std::atomic<T>` (`<atomic>`) wraps a variable so its operations are atomic:
```cpp
std::atomic<int> counter = 0;
counter++;    // ONE indivisible operation — can't be interrupted
```

### The WHY
The data race (sub-topic 3) came from `counter++` being 3 steps (read→modify→write) with a race window between them. A **mutex** fixes it but is heavyweight for a single variable (lock/unlock, others block). **`std::atomic` is the lighter fix**: it makes `counter++` a **single indivisible hardware instruction** — no interleaving, no lost updates, **no lock**.
> Mutex serializes with a *lock*; atomic makes the operation itself *indivisible* at the hardware level. For one variable, atomic is cheaper.

```cpp
int              counter;   counter++;   // ❌ 3 steps → race
std::mutex m;    counter++;              // ✅ works, but lock/block overhead
std::atomic<int> counter;   counter++;   // ✅ one atomic hardware op, no lock, no blocking
```
Fixing the sub-topic-3 counter: `std::atomic<int> counter = 0;` → the 1M-increment loop gives exactly 2,000,000, no mutex.

### How it's atomic without a lock
CPUs have hardware instructions that do read-modify-write as one uninterruptible step (e.g. `LOCK XADD` on x86). `std::atomic` compiles to these → **lock-free** for basic types (no mutex, just one CPU instruction the hardware won't split).

### Operations
`x = 5` and `int a = x` are **shorthand** for `store`/`load` (they call them under the hood). The explicit forms exist so you can pass a `memory_order` (Tier 3 #14); the shorthand always uses the strongest ordering.

| Op | What it does | Example |
|---|---|---|
| **`store(v)`** | atomic write | `x.store(5)` → x becomes 5 |
| **`load()`** | atomic read | `int a = x.load()` |
| **`fetch_add(n)`** | atomically add `n`, **return the OLD value** | `x.fetch_add(3)` (what `x += 3` calls) |
| **`fetch_sub(n)`** | atomically subtract, return old value | `x.fetch_sub(2)` |
| **`exchange(v)`** | atomically set to `v`, **return the old value** | `int old = x.exchange(10)` |
| **`compare_exchange_strong`** | CAS — set if it matches expected | the retry-loop primitive (below) |

The "return the **old** value" ops are useful when you need what was there before you changed it (e.g. `fetch_add` on a counter gives you *your* index).

### CAS — Compare-And-Swap (the fundamental primitive)
> **"Atomically: if the variable still equals what I EXPECT, replace it with a NEW value; else fail and tell me the actual value — change nothing."**
```cpp
std::atomic<int> x = 10;
int expected = 10;
bool ok = x.compare_exchange_strong(expected, 20);
// if x == 10 → set x = 20, return true.
// if x != 10 → set expected = x's actual value, return false.
```
**Why CAS:** to update a variable **based on its current value** safely (another thread might change it between your read and write). CAS applies your update **only if nobody changed it since you read it**. Classic use = **retry loop** (basis of lock-free algorithms):
```cpp
int old = x.load();
while (!x.compare_exchange_strong(old, old * 2)) {
    // failed → someone changed x → `old` now holds x's new value → retry
}
```
Keeps retrying until the update lands on an unchanged value. "Check-then-act" done as one indivisible step.

### atomic vs mutex (interview Q)
| | `std::atomic` | `std::mutex` |
|---|---|---|
| Protects | a **single variable** | any code / **multiple variables** |
| Mechanism | one hardware instruction (**lock-free**) | a software **lock** (threads block) |
| Cost | cheap (no blocking) | heavier (lock/unlock, waiting) |
| Use for | counters, flags | critical section over multiple vars |

Need a **mutex**, not atomic, when things must change together:
```cpp
balance -= amount;      // these two must happen as ONE consistent unit →
log.push_back(amount);  // atomic protects each alone, not the pair → use a mutex
```

### Summary
- **`std::atomic<T>`** = indivisible ops on a variable, **no lock** (lock-free basic types).
- **Why:** light fix for a single-variable race — `counter++` becomes one atomic op vs a 3-step race.
- Ops: `load/store/fetch_add/exchange` + **CAS** (`compare_exchange_strong`).
- **CAS** = "if still equals expected, swap; else fail" → retry loops, lock-free algorithms.
- **atomic** = single var, cheap; **mutex** = multiple vars / critical section, heavier.

---

## Sub-topic 8 — `shared_mutex` (Reader-Writer Lock)

### Simple: a lock that distinguishes readers from writers
`std::shared_mutex` (`<shared_mutex>`, C++17) has **two** locking modes:
- **Shared (read) lock** — **many** threads hold it at once → for *readers*.
- **Exclusive (write) lock** — only **one** thread, **no readers** allowed → for *writers*.
```
Many readers together:  R R R R    ← all allowed at once (shared lock)
One writer alone:        W          ← exclusive; blocks ALL readers & writers
```

### The WHY
Data races need **≥1 writer** (sub-topic 3). Two threads just **reading** (nobody writing) is **safe** — reads don't change anything. But a plain `mutex` blocks *everyone*, serializing even pure readers → needless bottleneck for **read-heavy** data (config, caches, lookup tables).
> Insight: concurrent reads are safe; only writes need exclusivity. `shared_mutex` lets readers run in parallel and locks exclusively only for writes.

### Code
```cpp
std::shared_mutex rw;
int data = 0;

int read() {
    std::shared_lock<std::shared_mutex> lock(rw);   // SHARED — readers don't block each other
    return data;
}
void write(int v) {
    std::unique_lock<std::shared_mutex> lock(rw);   // EXCLUSIVE — alone, no readers
    data = v;
}
```
- **`std::shared_lock`** → shared (read) lock. **`std::unique_lock`/`lock_guard`** → exclusive (write) lock.

### Rules (who holds it when)
| While held as… | Another reader can join? | A writer can enter? |
|---|---|---|
| **Shared (read)** | ✅ yes | ❌ no (waits for readers to leave) |
| **Exclusive (write)** | ❌ no | ❌ no |
Readers coexist; a writer needs the place to itself.

### When to use / not
- **Use** for **read-heavy** data (reads ≫ writes) — reader parallelism pays off.
- **Don't** for balanced/write-heavy — `shared_mutex` has **more overhead** than plain `mutex` (tracks reader counts); a plain `mutex` is faster. Single value → `atomic`.
- **Caveat: writer starvation** — constant readers can starve a waiting writer (good impls mitigate by blocking new readers once a writer waits).

### 🔑 Q: if concurrent reads are safe, why lock readers at all? Just don't lock them!
The shared lock on readers is **NOT** to protect readers from each other (they're fine together). It's to protect readers from **WRITERS**. Without any lock, a **writer could modify the data *while* a reader is reading it** → the reader sees **half-updated / torn / inconsistent** data (a **read-write data race**).

The shared and exclusive locks are **mutually exclusive with each other**:
- While readers hold the **shared** lock, a writer's **exclusive** lock **must wait** → no writing mid-read.
- While a writer holds the **exclusive** lock, readers' **shared** locks **must wait** → no reading mid-write.

So readers take the shared lock **not to coordinate with other readers, but to block writers** (and be blocked by a writer). If readers took no lock, nothing would stop a writer from scribbling on the data they're reading → corruption.
> Your hypothesis is exactly right: the lock exists because of the **reader↔writer race** (reading a section about to be overwritten). Readers can share *with each other*, but must still exclude *writers* — which is what taking the shared lock does.

### Summary
- **`shared_mutex`** = reader-writer lock: **shared** (many readers) vs **exclusive** (one writer, no readers).
- **Why:** reads are safe together; a plain mutex needlessly serializes them.
- **Readers lock to exclude WRITERS**, not each other — prevents the read↔write race (reading mid-write = torn data).
- Use for **read-heavy** data; plain `mutex` for balanced/write-heavy; `atomic` for a single value. Watch for **writer starvation**.

---

## Sub-topic 9 — `std::call_once` / `once_flag`

### Simple: run something exactly once, across all threads
`std::call_once` (`<mutex>`) runs a piece of code **exactly one time**, no matter how many threads reach it. The **first** thread runs it; others **wait** for it to finish, then **skip** it (and just use the result).
```cpp
std::once_flag flag;
void init() {
    std::call_once(flag, []{ /* runs EXACTLY once, ever */ });
}
```
`std::once_flag` = a bookkeeping object remembering "already done?".

### The WHY — thread-safe lazy initialization
You want to set something up **once, on first use** (load config, open a DB, build a table). Single-threaded is easy:
```cpp
if (res == nullptr) res = new Resource();   // init once
```
But multi-threaded this is a **data race** (check-then-act):
```
Thread A: res==nullptr? TRUE, about to create...
Thread B: res==nullptr? TRUE too! (A not done yet)
A: res = new Resource()   → resource #1
B: res = new Resource()   → resource #2 (overwrites → leak / two DB connections!)
```
Both see null → both initialize → two setups.

### call_once vs mutex-every-call
```cpp
// mutex works but locks on EVERY call forever (wasteful long after init):
std::lock_guard lock(m); if (!res) res = new Resource();

// call_once — purpose-built: init once, near-free afterward:
std::call_once(flag, []{ res = new Resource(); });
```
First thread inits; others wait; after that, `call_once` just checks the flag with almost no overhead. Exactly one init, guaranteed.
> Core idea: **the first thread creates it; every later thread/call just USES the already-created thing** (threads arriving mid-init wait, then use it — never re-create).

### Classic use: thread-safe Singleton
```cpp
class Singleton {
    static Singleton* instance;
    static std::once_flag flag;
public:
    static Singleton* get() {
        std::call_once(flag, []{ instance = new Singleton(); });  // created once, thread-safe
        return instance;
    }
};
```
Without `call_once`, two first-time callers could create **two** singletons.

### 🔑 Modern alternative: the Meyers singleton (C++11)
A function-local **`static`** is guaranteed **initialized exactly once, thread-safely**, since C++11 (compiler inserts the equivalent of `call_once`):
```cpp
Singleton& get() {
    static Singleton instance;   // once + thread-safe, on first call
    return instance;
}
```
Cleanest for a simple singleton. Use `call_once` for more general one-time init. **Know both for interviews.**

### Summary
- **`std::call_once(flag, fn)`** = run `fn` once across all threads (others wait, then use the result). Needs `std::once_flag`.
- **Why:** thread-safe **lazy init** — naive `if(!init) init()` is a data race; mutex-every-call is wasteful; `call_once` inits once, near-free after.
- **First thread creates it; the rest just use the already-created thing.**
- Use: **thread-safe singleton**. Modern one-liner: **Meyers singleton** (`static` local, C++11 thread-safe).

---

## Sub-topic 10 — Data Race vs Race Condition

### Simple: they sound the same, they're not
- **Data race** = *low-level, technical*: **unsynchronized concurrent access to the same memory, ≥1 write.** About *memory access*. Always **UB**.
- **Race condition** = *high-level, logical*: **program correctness depends on thread timing/ordering.** About *outcomes*.

They **overlap but neither contains the other** — you can have one without the other.

### Data race — memory-level (sub-topic 3)
```cpp
int counter = 0;
// A: counter++;  B: counter++;   // unsynchronized, both write → UB
```
Fix = **synchronization** (mutex/atomic). Once added, the data race is gone.

### Race condition — logic-level (subtle!)
Different results depending on thread order — and **you can have it with NO data race.** Bank withdrawal:
```cpp
std::atomic<int> balance = 100;   // atomic! NO data race on balance
void withdraw(int amount) {
    if (balance >= amount)        // (1) CHECK — atomic, safe alone
        balance -= amount;        // (2) ACT   — atomic, safe alone
}
```
Two threads `withdraw(100)` on balance 100:
```
A: check 100>=100 → TRUE
B: check 100>=100 → TRUE   (A hasn't subtracted yet)
A: balance -= 100 → 0
B: balance -= 100 → -100   ❌ overdrawn!
```
**No data race** (every access atomic), but a **race condition** — the CHECK and ACT are separate steps and another thread slipped between. Classic **check-then-act** race.

### 🔑 Fixing the data race does NOT fix the race condition
Making `balance` atomic removed the data race but did nothing for the race condition — the problem is that **check+act must be ONE indivisible unit.** Fix with a **mutex**:
```cpp
std::mutex m; int balance = 100;
void withdraw(int amount) {
    std::lock_guard<std::mutex> lock(m);   // whole check+act = one critical section
    if (balance >= amount) balance -= amount;
}
```
Now nothing slips between check and act. *(This is why atomic-on-one-variable isn't always enough — sub-topic 7.)*

### The four combinations
| | Data race? | Race condition? | Example |
|---|---|---|---|
| **Both** | ✅ | ✅ | unsynchronized `counter++` |
| **RC, no data race** | ❌ | ✅ | atomic `balance` check-then-act (the subtle case) |
| **Data race, no RC** | ✅ | ❌ | benign unsynchronized write logic doesn't depend on (still UB) |
| **Neither** | ❌ | ❌ | properly synchronized |

### 🔑 CRUCIAL clarification: why atomic fixes the counter but NOT the bank
**Atomic makes a SINGLE operation indivisible.** So:
- **`counter++` = ONE operation** → `atomic<int>` makes it indivisible → the "both read 0" gap is gone → reaches exactly 2,000,000. ✅ **Atomic fully fixes it.** (Two threads *cannot* both read 0: an atomic increment is one uninterruptible hardware instruction; the other thread is locked out mid-op and reads the already-updated value.)
- **bank `withdraw` = TWO operations** (check THEN act). Atomic makes each *alone* indivisible, but **cannot fuse check-and-act into one unit** → another thread slips between → still wrong → needs a **mutex** (or a CAS loop) to make the *whole sequence* indivisible.

> Rule: **`atomic` → single operation. `mutex` → multi-step (compound) operation.** The counter is single; the withdrawal is compound.

### 🔑 "Wrong result" ≠ "data race" (terminology)
- **Data race** = a statement about **HOW memory is accessed** (unsynchronized access + write). The *mechanism*.
- **Race condition** = a statement about **the OUTCOME** (wrong result depending on timing). The *result*.

A wrong result does **not** by itself mean "data race." The atomic bank ending at −100 is a **race condition** but **NOT a data race** — because every access *was* synchronized (atomic). Compare:
| | Access synced? | Result wrong? | Data race? | Race condition? |
|---|---|---|---|---|
| plain `counter++` | ❌ | ✅ (<2M) | ✅ **yes** | ✅ yes |
| atomic `counter++` | ✅ | ❌ (=2M) | ❌ | ❌ |
| atomic bank `withdraw` | ✅ | ✅ (−100) | ❌ **no** | ✅ yes |
Both atomic rows → **not** data races (accesses synced). Counter is fully fixed; bank is still a **race condition** (compound op). Don't equate "data is wrong" with "data race" — data race = *unsynchronized access*, not *wrong answer*.

### Summary
- **Data race** = memory-level, unsynchronized access + write → UB. Fix: synchronization.
- **Race condition** = logic-level, correctness depends on timing → wrong results.
- **Overlap but differ**; subtle case = **race condition with NO data race** (check-then-act on an atomic).
- **Fixing the data race ≠ fixing the race condition** — atomic protects one access; a **mutex** makes a whole operation indivisible.

---

## Sub-topic 11 — Deadlock, Livelock & Starvation

### Deadlock — everyone stuck forever
Two+ threads each **wait for a resource the other holds** → none can proceed.
```cpp
std::mutex m1, m2;
// Thread A: lock(m1); lock(m2);   // A holds m1, waits for m2  ┐
// Thread B: lock(m2); lock(m1);   // B holds m2, waits for m1  ┘ → circular wait → FROZEN
```

### ⭐ The 4 Coffman conditions (ALL must hold)
1. **Mutual exclusion** — a resource is held by only one thread at a time.
2. **Hold and wait** — a thread holds one resource while waiting for another.
3. **No preemption** — a resource can't be forcibly taken; the holder must release it.
4. **Circular wait** — a cycle of threads each waiting for what the next holds (A→B→A).
> All four required → **break ANY one → no deadlock possible.**

### Preventing deadlock (break a condition)
1. **Lock ordering (break circular wait)** — most common fix. Always acquire mutexes in a **consistent global order** everywhere:
   ```cpp
   lock(m1); lock(m2);   // EVERY thread same order → no cycle
   ```
2. **Lock both at once (break hold-and-wait)** — `std::scoped_lock lock(m1, m2);` (grabs all together, deadlock-free).
3. **try-lock + back-off** — `try_lock`; if it fails, release what you hold and retry.
4. **Fewer locks** — never hold two at once → no hold-and-wait.
> Practical rule: **consistent lock ordering** (or `scoped_lock`) prevents almost all real deadlocks.

### Livelock — busy but no progress
Threads are **actively running and changing state, but making no real progress** — they keep reacting to each other forever. *(Two people in a hallway both step left, both step right, both step left… never passing.)* Often from naive deadlock avoidance: grab lock → sense conflict → release → retry, all in lockstep.
> **Deadlock = frozen** (blocked, doing nothing). **Livelock = active** (running) but achieving nothing — CPU looks busy, zero progress.

### Starvation — one thread never gets a turn
A thread is **perpetually denied** a resource because others keep getting it first (it's not blocked forever — it just never wins). Causes: **priority scheduling** (high-priority threads jump the queue), **writer starvation** (constant readers on a `shared_mutex`), **unfair (non-FIFO) locks**. **Fix:** fair/FIFO locks, or **aging** (boost a long-waiting thread's priority).

### The three side by side
| | What's happening | Threads are… |
|---|---|---|
| **Deadlock** | circular wait, each holds what the other needs | **frozen** (blocked forever) |
| **Livelock** | reacting to each other, repeating forever | **active** but no progress |
| **Starvation** | one thread perpetually denied its turn | **running**, one left out |

### Summary
- **Deadlock** = circular wait → all frozen. **4 Coffman conditions** (mutual exclusion, hold-and-wait, no preemption, circular wait); break any one → no deadlock.
- **Prevent:** consistent **lock ordering** or **`scoped_lock`**; try-lock/back-off; fewer locks.
- **Livelock** = active but no progress (keep reacting). **Starvation** = one thread perpetually denied (priority/unfair locks) → fix with fairness/FIFO or aging.

---

## Sub-topic 12 — `volatile` vs `atomic`

### The trap
> **In C++, `volatile` is NOT for thread synchronization. `std::atomic` is.** (Java/C# `volatile` *does* sync — the source of the confusion.)

### What `volatile` actually does
Tells the compiler: **"don't optimize away reads/writes to this — always really read it from memory."** Real purpose: **memory-mapped hardware & signal handlers** (values that change outside normal control flow).
```cpp
volatile int* sensor = (volatile int*)0x4000;
int a = *sensor;
int b = *sensor;   // volatile forces a REAL re-read (without it, compiler may reuse a)
```
That's all it does — **prevents access optimizations**, for hardware/signals.

### Why `volatile` does NOT give thread safety
It's missing the three things thread safety needs:
1. **No atomicity** — `volatile int x; x++;` is still a 3-step read-modify-write → data race remains. Volatile forces memory access but doesn't make `x++` indivisible.
2. **No memory-ordering guarantees** — CPUs/compilers reorder ops; thread safety needs ordering rules about what other threads see. `volatile` gives none; `atomic` does.
3. **No mutual exclusion** — locks nothing.

### What `std::atomic` gives that volatile doesn't
- **Atomicity** — `atomic<int> x; x++;` = one indivisible op (no lost updates).
- **Memory-ordering guarantees** (the memory model, Tier 3 #14).
- (Also implies "don't over-optimize," so you don't need `volatile` too.)

### Side-by-side
| | `volatile` | `std::atomic` |
|---|---|---|
| Purpose | stop compiler **optimizing away** accesses | **thread-safe** shared access |
| Atomic ops? | ❌ | ✅ |
| Ordering guarantees? | ❌ | ✅ |
| For | hardware registers, signal handlers | **multithreading** |
| Thread-safe? | ❌ **NO** | ✅ |

### ⭐ Interview answer
> "Can I use `volatile` for thread sync?" → **No.** `volatile` only stops the compiler optimizing away accesses (for hardware/signals) — **no atomicity, no ordering** → `volatile int x; x++;` still races. Use **`std::atomic`** (or a mutex) for thread safety. Unrelated concerns.

### Summary
- **`volatile`** = "always really read/write from memory"; for **hardware/signals**, not threads.
- No atomicity, no ordering, no mutual exclusion → **not thread-safe**.
- **`std::atomic`** = the real tool: atomic ops + ordering guarantees.
- Java/C# `volatile` syncs; C++ `volatile` does not.

---

## Sub-topic 13 — Thread-Safe Design

**Thread-safe** = behaves correctly when used by multiple threads at once (no data races, race conditions, or corrupted state). This is the **synthesis** of the whole topic — designing code safe by construction. Classic task: *"make this class thread-safe."*

### Principle 1 — Identify the shared MUTABLE state, protect that
Ask: *"what data is shared AND mutable?"* — only that needs protection.
- **Shared + mutable** → synchronize (mutex/atomic).
- **Immutable** (never changes after construction) → **always thread-safe**, no protection.
- **Not shared** (each thread its own) → no protection.

### Principle 2 — Protect the whole OPERATION, not each access
Check-then-act (sub-topic 10) must be one critical section:
```cpp
// ❌ atomic balance, but a GAP between check and act → race condition
std::atomic<int> balance;
if (balance >= amt) balance -= amt;

// ✅ whole check+act under one lock
int balance; std::mutex m;
{ std::lock_guard<std::mutex> lock(m); if (balance >= amt) balance -= amt; }
```

### Principle 3 — Encapsulate the lock (keep it internal)
Private mutex; lock **inside** methods; callers don't know it exists.
```cpp
class Counter {
    int value = 0;
    mutable std::mutex m;                       // private detail; `mutable` → lockable in const methods
public:
    void increment() { std::lock_guard<std::mutex> lock(m); value++; }
    int  get() const { std::lock_guard<std::mutex> lock(m); return value; }  // READS need the lock too!
};
```
- **`mutable std::mutex`** — lock inside `const` methods (the Topic 2 `mutable` use).
- **Reads need the lock too** — reading while another thread writes = data race.

### Principle 4 — Keep critical sections small
Lock only around the shared access, not slow unrelated work (sub-topic 4). Long locks kill parallelism + raise deadlock risk.

### Principle 5 — Prefer immutability / confinement (best sync = no sharing)
- **Immutable objects** are automatically thread-safe (no writes = no races).
- **Confinement** — give each thread its own copy; nothing shared to protect. *(Browser model: confine an object to one sequence, pass messages, avoid locks — bonus-track edge.)*

### Principle 6 — Beware the INTERFACE, not just the implementation
Individually-safe methods can still race when **combined**:
```cpp
if (!stack.empty()) {   // A: not empty ✅
    // ← B pops the last item HERE
    stack.pop();        // A: pops empty → crash
}
```
Check-then-act across two calls races. Fix: provide a **combined atomic operation** (e.g. `try_pop()` that checks+pops under one lock) instead of separate `empty()`+`pop()`.

### Interview approach: "make this class thread-safe"
1. Identify shared mutable state. 2. Add a **private mutex**. 3. Lock in **every** method touching it (incl. `const` reads → `mutable` mutex). 4. Protect **whole operations** (check-then-act under one lock). 5. Keep critical sections **small**. 6. Watch the **interface** (offer combined ops). 7. Consider **not sharing** at all (immutability/confinement).

### Summary
- Protect **shared mutable state** only; immutable/unshared needs nothing.
- Protect **whole operations**, not single accesses.
- **Encapsulate** the lock (private `mutable std::mutex`; lock reads too).
- Small critical sections; prefer **immutability/confinement**.
- **Interface-level** races: individually-safe methods can race when combined → provide combined atomic ops.

---

## Sub-topic 14 — `async` / `future` / `promise` (lighter)

### The problem
Raw `std::thread` **can't return a value** (the function's return is discarded). Getting a result back manually needs a shared variable + mutex + condition_variable — lots of boilerplate.

### `std::async` + `std::future` — run a task, collect the result
`std::async` (`<future>`) runs a function (maybe on another thread) and returns a **`std::future`** — a placeholder for a result that isn't ready yet. Call `.get()` to retrieve it (waiting if needed).
```cpp
int compute() { return 42; }

std::future<int> result = std::async(compute);   // starts compute()
// ... do other work concurrently ...
int value = result.get();                        // blocks until ready → 42
```
**Mental model:** a `future` = a **claim ticket** at a coat check. `async` starts the work and gives you the ticket; `.get()` hands it in (instant if ready, else waits).

### Why it's nicer
```cpp
// ❌ raw thread: manual result + mutex + condition_variable + ready flag...
// ✅ async: one line —
auto fut = std::async(compute);
int result = fut.get();
```
Hides all the synchronization. Also: **arguments** work (`std::async(add, 3, 4)`), and if the task **throws**, the future **re-throws** it on `.get()` (clean cross-thread error handling).

### Launch policies
```cpp
std::async(std::launch::async,    compute);  // FORCE a new thread — concurrent now
std::async(std::launch::deferred, compute);  // LAZY — runs on the caller when you .get()
std::async(compute);                         // default: impl picks either (gotcha!)
```
Default may **not** run in parallel — pass **`std::launch::async`** if you need guaranteed concurrency.

### `std::promise` — the manual version
Lower-level: set the value in one thread, read it via a linked future in another.
```cpp
std::promise<int> p;
std::future<int> fut = p.get_future();     // future linked to this promise
std::thread t([&p]{ p.set_value(42); });   // fulfill the promise → future ready
int value = fut.get();                     // waits, then 42
t.join();
```
A **promise/future pair** = a one-way channel: **promise = writing end** (`set_value`), **future = reading end** (`get`).

> Relationship: **`promise`** = manual producer end; **`future`** = consumer end; **`async`** = convenience that creates both and runs a function to fill the promise. Use `async` normally; `promise` when you must set the result yourself.

### Summary
- Raw `thread` can't return a value; `async`/`future` is the clean way.
- **`std::async(fn)`** → returns a **`future`**; **`future.get()`** blocks until ready, re-throws exceptions.
- **Launch policies:** `async` (new thread) vs `deferred` (lazy); default may pick either → use `launch::async` for real parallelism.
- **`std::promise`** = manual producer end paired with a `future` reading end; `async` is the convenience layer over it.

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
| `atomic_cas.cpp` | Counter race fixed with `std::atomic` (no mutex) + a **CAS retry loop** (atomically double a value across 10 threads → 1024) |
| `shared_mutex_demo.cpp` | Reader-writer lock: 4 readers share the read lock concurrently; 1 writer takes the exclusive lock (no readers during writes) |
| `call_once_demo.cpp` | 8 threads call `getResource()`; the initializer runs **exactly once** (first thread creates it, the rest reuse it) |
| `thread_safe_counter.cpp` | A self-protecting `Counter` class: private `mutable mutex`, locked reads+writes, a combined `decrementIfPositive()` op → 10 threads → exactly 1,000,000 |
| `async_future.cpp` | `std::async`+`future` (get a return value back), exceptions propagating through `.get()`, and `std::promise`/`future` (manual result channel) |
