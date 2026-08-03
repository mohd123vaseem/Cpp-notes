# Topic 6 — The Four Casts

Tier 1 #6 (~2 days). *"What are the four C++ casts and when do you use each?"* is asked constantly. A **cast** = explicitly converting one type to another. C inherited one blunt tool (the C-style cast); C++ split it into **four named casts**, each with a specific, restricted job — so your *intent* is visible and the compiler can check it.

> `dynamic_cast` was already covered in Topic 4 (RTTI). Here we cover all four together and contrast them.

## Why four instead of one

The C-style cast `(int)x` does *whatever conversion makes it compile* — you can't tell from reading it whether it's a harmless numeric conversion or a dangerous bit-reinterpretation. The four C++ casts each say exactly **what kind** of conversion you mean, are **searchable**, and the compiler **rejects** misuse.

---

## Progress tracker (sub-topics)

| # | Sub-topic | The classic question | Status |
|---|-----------|----------------------|--------|
| 1 | **`static_cast`** — compile-time, related types | "When do you use static_cast?" | ✅ Done |
| 2 | **`dynamic_cast`** — runtime, polymorphic down-cast | "How is dynamic_cast different?" | ✅ Done (Topic 4) |
| 3 | **`const_cast`** — add/remove const | "When is const_cast valid vs UB?" | ✅ Done |
| 4 | **`reinterpret_cast`** — bit reinterpretation (dangerous) | "What does reinterpret_cast do?" | ✅ Done |
| 5 | **Why C-style casts are discouraged** | "Why avoid C-style casts?" | ✅ Done |

---

## Sub-topic 1 — `static_cast`

The **general-purpose, compile-time** cast for **sensible, related** conversions. The one you'll use most.
```cpp
static_cast<TargetType>(expression)
```

**Uses:**
```cpp
// 1. Numeric conversions
int i = static_cast<int>(3.99);              // 3 (explicit truncation)
double r = static_cast<double>(a) / b;       // force floating-point division

// 2. Up-cast (derived → base — always safe)
Animal* a = static_cast<Dog*>(dogPtr);

// 3. Down-cast WITHOUT runtime check (base → derived)
Animal* a = new Dog();
Dog* d = static_cast<Dog*>(a);   // works IF a really is a Dog — but NO check!
```
⚠️ Down-cast difference from `dynamic_cast`: `static_cast` does **no runtime verification**. If `a` is actually a `Cat`, this compiles and gives a broken `Dog*` → UB. `dynamic_cast` would return `nullptr`. Use `static_cast` down-casts only when 100% certain (skips the check for speed).

**Safety — refuses nonsensical conversions:** it only allows conversions where the types have a **real, defined relationship**; unrelated types → compile error.
```cpp
// ✅ related → allowed:
int i = static_cast<int>(3.14);          // int & double are both numbers
Animal* a = static_cast<Animal*>(dog);   // Dog is-a Animal (inheritance)

// ❌ nonsense → refused:
int* p = static_cast<int*>(3.14);        // a double is NOT an address — meaningless
Banana* b = static_cast<Banana*>(car);   // Car and Banana are unrelated
```
A C-style cast `(int*)d` would **silently allow** this nonsense (force the bits → garbage/UB); `static_cast` **stops you at compile time**. That refusal *is* the safety — for "reinterpret the bits regardless" you're forced to switch to `reinterpret_cast` and take responsibility.

**Summary:** compile-time cast for related conversions; no runtime check; rejects unrelated types; reach for this first.

---

## Sub-topic 3 — `const_cast`

The **only** cast that adds/removes **`const`** (or `volatile`). Changes nothing else.
```cpp
const int* cp = &x;
int* p = const_cast<int*>(cp);   // strip const → writable through p
```

**Legit use:** calling an old/C API that takes non-const but only reads:
```cpp
void legacyPrint(char* s);              // takes char*, only reads
const char* msg = "hello";
legacyPrint(const_cast<char*>(msg));    // strip const to fit the signature
```

**⚠️ The critical rule (UB):** removing const and *writing* is only safe if the object was **not originally const**:
```cpp
const int x = 10;
int* p = const_cast<int*>(&x);
*p = 20;               // ❌ UNDEFINED BEHAVIOR — x is truly const (may be read-only memory)

int y = 10;            // NOT const
const int* cp = &y;    // just a const VIEW of a non-const object
*const_cast<int*>(cp) = 20;   // ✅ OK — y was never really const
```
**Adding const** is legal too, but you rarely need `const_cast` for it — **adding const is always safe and implicit** (compiler does it for you):
```cpp
int* p = &x;
const int* cp = p;     // ✅ adding const — just works, NO cast needed
```
The one niche use for explicitly adding const is forcing **overload selection** (pick a `const` overload — what `std::as_const` does):
```cpp
void process(int* p);        // non-const overload
void process(const int* p);  // const overload
process(const_cast<const int*>(p));   // now picks the const version
```

**The asymmetry:**
| Direction | Safe? | Need `const_cast`? |
|---|---|---|
| **Add** const (`int*` → `const int*`) | ✅ always | ❌ no — implicit/automatic |
| **Remove** const (`const int*` → `int*`) | ⚠️ UB if you write to a truly-const object | ✅ yes — this is why `const_cast` exists |

Adding = promising *less* access → always fine. Removing = claiming *more* access than you were given → needs the explicit, greppable cast.

**Summary:** only cast for const/volatile; main use = const-incorrect legacy APIs; **removing** is the dangerous direction (**writing** through it is UB if the object was truly const); **adding** is safe/implicit; needing it often signals a design problem.

---

## Sub-topic 4 — `reinterpret_cast`

The **most dangerous** cast: reinterpret the **raw bit pattern** as a completely **unrelated** type. No conversion, no check.

**The key idea:** the bits in memory **never change** — but how you *interpret* them depends on the type. The same bits mean different values read as different types. `reinterpret_cast` switches the interpreting type.

**Example where the difference is obvious — read a `float`'s bits as an `int`:**
```cpp
float f = 3.14f;
int* asInt = reinterpret_cast<int*>(&f);   // read the SAME 4 bytes as an int
cout << "as float: " << f << "\n";         // 3.14
cout << "as int:   " << *asInt << "\n";    // 1078523331   ← same bytes, different value!
```
Nothing in memory changed — `float` and `int` just *encode* bytes differently, so the identical bytes read as `3.14` (float lens) or `1078523331` (int lens).

**Contrast with `static_cast` (the "aha"):**
```cpp
int a = static_cast<int>(f);             // 3            — CONVERTS the value (3.14 → 3)
int b = *reinterpret_cast<int*>(&f);     // 1078523331   — REINTERPRETS the bits
```
- `static_cast` = *"convert the meaning"* (keeps the value: 3.14 → 3).
- `reinterpret_cast` = *"keep the bits, change the label"* (keeps the bytes: they read as 1078523331).

**Mental model:** the bits are marks in an ambiguous script. Read as English → one meaning; read the *same marks* as another language → a different meaning. `reinterpret_cast` switches the language; `static_cast` would *translate* the meaning.
**Uses (rare, low-level):**
```cpp
uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);          // pointer ↔ integer
unsigned char* bytes = reinterpret_cast<unsigned char*>(&f); // view object as raw bytes (serialization, HW I/O)
```
**Dangerous because:** no conversion (bits taken as-is → garbage if types don't match); **not portable** (endianness/size/alignment); easy UB (strict aliasing).

**Summary:** reinterpret raw bits as an unrelated type; low-level only (serialization, hardware, pointer↔int); non-portable, easy UB — use sparingly. Its presence is a red flag.

---

## Sub-topic 5 — Why C-style casts are discouraged

The old `(Type)value` cast is a **blunt, silent tool**:
1. **Tries every cast until one works** — attempts `static_cast`, then `const_cast`, then `reinterpret_cast` (and combos), silently picking whatever compiles. `(Foo*)p` might be a harmless `static_cast` *or* a dangerous `reinterpret_cast` — you can't tell which.
2. **No intent, no safety** — a dangerous bit-reinterpretation looks identical to a safe numeric conversion.
3. **Not searchable** — you can grep for `reinterpret_cast` to audit dangerous casts; you can't grep `(Type)`.

The C++ way forces you to name the cast → explicit intent, restricted uses, dangerous ones stand out.
> **Rule: never use C-style casts in C++.**

---

## The four casts — master summary

| Cast | Purpose | Checked? | Danger |
|------|---------|----------|--------|
| **`static_cast`** | sensible related conversions (numeric, up/down-cast, `void*`) | compile-time only | low (down-cast unchecked) |
| **`dynamic_cast`** | safe polymorphic **down**-cast (RTTI) | **runtime** (nullptr/throw on fail) | low |
| **`const_cast`** | add/remove `const`/`volatile` | none | UB if you write to a truly-const object |
| **`reinterpret_cast`** | reinterpret raw bits as unrelated type | none | high — non-portable, easy UB |

**Rules of thumb:** default to `static_cast`; `dynamic_cast` for checked polymorphic down-casts; `const_cast` only for const-incorrect APIs; `reinterpret_cast` only for deliberate low-level bit work; **never** C-style.

---

## Code examples in this folder

| File | Demonstrates |
|------|--------------|
| _(added as we go)_ | |
