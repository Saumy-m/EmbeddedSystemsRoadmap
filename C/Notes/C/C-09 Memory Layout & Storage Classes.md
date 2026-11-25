> [!info] 
> title: Memory Layout & Storage Classes 
> tags: [C, memory, storage-class, linker, startup] 
> chapter: 9

| previous: [[C-08 Preprocessor & Macros]] | next: [[C-10 File IO & Standard Library]] |
| ---------------------------------------- | ----------------------------------------- |
## 1. Why This Matters

- Every variable lives somewhere.
- Where it lives decides:
    - how long it exists,
    - who can see it,
    - how much Flash and RAM it costs,
    - whether it is initialized.
- Embedded: Flash is small. RAM is smaller. You must know both.

---

## 2. Program Sections

|Section|Holds|Initial value from|
|---|---|---|
|`.text`|machine code|Flash|
|`.rodata`|`const` data, string literals|Flash|
|`.data`|globals / statics with a nonzero initial value|copied Flash → RAM at startup|
|`.bss`|globals / statics with no initial value (or `= 0`)|zeroed at startup|
|heap|`malloc` memory|none (garbage)|
|stack|locals, call frames|none (garbage)|

PC process (low → high address):

```text
 low  ┌──────────────┐
      │ .text        │  code
      │ .rodata      │  const, literals
      │ .data        │  initialized globals
      │ .bss         │  zero-init globals
      │ heap  ↓      │  grows up
      │              │
      │ stack ↑      │  grows down
 high └──────────────┘
```

MCU (e.g. Cortex-M):

```text
 FLASH  0x0800_0000                     RAM  0x2000_0000
 ┌──────────────────┐                   ┌──────────────────┐
 │ vector table     │                   │ .data            │ ← copied from Flash
 │ .text            │                   │ .bss             │ ← zeroed
 │ .rodata          │                   │ heap  →          │
 │ .data (init copy)│ ───── copy ─────► │         ...      │
 └──────────────────┘                   │ ← stack          │ top of RAM
                                        └──────────────────┘
```

Cost of each kind of variable:

|Definition|Flash|RAM|
|---|---|---|
|`const int t[100] = {...};`|400 B|0|
|`int t[100] = {1, 2, ...};`|400 B (init image)|400 B|
|`int t[100];`|0|400 B|
|`void f(void) { int t[100]; }`|0|400 B of stack during the call|
|string literal `"hello"`|6 B|0|

> [!note] A global with an initial value costs **Flash and RAM**. A `const` global costs **Flash only**. A zero-init global costs **RAM only**.

---

## 3. Seeing Where Things Live

```c
#include <stdio.h>
#include <stdlib.h>

int         g_init   = 5;         // .data
int         g_uninit;             // .bss
const int   g_const  = 7;         // .rodata
static int  s_file   = 1;         // .data (file scope)

void func(void) { }               // .text

int main(void) {
    int          local   = 0;     // stack
    static int   s_local = 2;     // .data
    const char  *lit     = "hi";  // literal in .rodata
    void        *heap    = malloc(8);

    printf("func     (text)   : %p\n", (void *)func);
    printf("literal  (rodata) : %p\n", (const void *)lit);
    printf("g_const  (rodata) : %p\n", (const void *)&g_const);
    printf("g_init   (data)   : %p\n", (void *)&g_init);
    printf("s_local  (data)   : %p\n", (void *)&s_local);
    printf("g_uninit (bss)    : %p\n", (void *)&g_uninit);
    printf("malloc   (heap)   : %p\n", heap);
    printf("local    (stack)  : %p\n", (void *)&local);
    (void)s_file;
    free(heap);
    return 0;
}
```

- Exact values change on each run (ASLR).
- Order is usually: text < rodata < data < bss < heap < ... < stack.

### Measure a build

- `size` prints section totals.
- Numbers below are examples. Yours will differ.

```text
$ gcc main.c -o app
$ size app
   text    data     bss     dec     hex filename
   1523     600       8    2131     853 app

$ arm-none-eabi-size firmware.elf
   text    data     bss     dec     hex filename
  18432     124    2052   20608    5080 firmware.elf
```

- Flash used = `text + data`.
- RAM used (static) = `data + bss`.
- Heap and stack are extra, **not** in this number.

Other tools:

```text
nm --size-sort -S firmware.elf       # biggest symbols
objdump -h firmware.elf              # section headers
gcc ... -Wl,-Map=firmware.map        # full memory map
gcc ... -fstack-usage                # per-function stack use (.su files)
```

---

## 4. Scope, Lifetime, Linkage

Three different ideas. Do not mix them.

|Idea|Question|Values|
|---|---|---|
|**Scope**|where can I use the name?|block, file, function prototype|
|**Lifetime**|how long does the object exist?|automatic, static, allocated|
|**Linkage**|does another `.c` file see it?|none, internal, external|

|Declaration|Scope|Lifetime|Linkage|
|---|---|---|---|
|local `int x;`|block|automatic|none|
|local `static int x;`|block|static|none|
|global `int x;`|file|static|external|
|global `static int x;`|file|static|internal|
|`extern int x;`|file|static|external|
|function `int f(void)`|file|static|external|
|function `static int f(void)`|file|static|internal|
|`malloc` block|—|until `free`|—|

---

## 5. Storage-Class Specifiers

C has: `auto`, `register`, `static`, `extern`, `_Thread_local`. (`typedef` is grouped with them syntactically.)

### `auto`

- Default for locals. Nobody writes it.
- Stack. Garbage until set. Gone when the block ends.

### `register`

- Hint: keep in a CPU register.
- Cannot take its address.
- Modern compilers ignore it. Skip it.

```c
// register int i = 0;
// int *p = &i;          // error: address of register variable
```

### `static` (two meanings)

**1. Static local:** keeps value between calls.

```c
#include <stdio.h>

void visit(void) {
    static int count = 0;       // initialized once, before main
    int fresh = 0;              // initialized on every call
    count++;
    fresh++;
    printf("count=%d fresh=%d\n", count, fresh);
}

int main(void) {
    visit(); visit(); visit();       // count: 1 2 3, fresh: 1 1 1
    return 0;
}
```

**2. Static at file scope:** hides the name from other files.

```c
#include <stdio.h>

static int hidden = 10;          // private to this file

static void helper(void) {       // private function
    printf("%d\n", hidden);
}

int main(void) {
    helper();
    return 0;
}
```

- Same keyword. Different effect: **lifetime** for locals, **linkage** for globals.

### `extern`

- "This is defined somewhere else."
- Declares, does not allocate.

```c
#include <stdio.h>

extern int counter;          // declaration

int main(void) {
    printf("%d\n", counter);
    return 0;
}

int counter = 42;            // definition
```

### `_Thread_local` (C11)

- One copy per thread.
- Rare in firmware. Some RTOSes use their own thread-local storage.

---

## 6. Initialization Rules

|Kind|Default value|
|---|---|
|global / static|`0` (bss)|
|local|garbage|
|`malloc`|garbage|
|`calloc`|`0`|

- Static initializer must be a **constant expression**.

```c
#include <stdio.h>

int a = 5;                 // ok
int b = 2 * 3;             // ok (constant expression)
// int c = a + 1;          // error in C: not constant
// static int d = rand();  // error: not constant

int main(void) {
    static int e = 7;      // ok
    int f = a + 1;         // ok for locals: runs at runtime
    printf("%d %d %d %d\n", a, b, e, f);
    return 0;
}
```

---

## 7. Block Scope and Shadowing

- Inner name hides outer name.
- Compile with `-Wshadow` to catch accidents.

```c
#include <stdio.h>

int x = 1;                          // global

int main(void) {
    printf("%d\n", x);              // 1
    int x = 2;                      // shadows global
    {
        int x = 3;                  // shadows again
        printf("%d\n", x);          // 3
    }
    printf("%d\n", x);              // 2
    return 0;
}
```

---

## 8. Globals Across Files

Rules:

- **Define** once in one `.c`.
- **Declare** with `extern` in the header.
- Everyone else includes the header.

> [!note] Multi-file. Not for Code Runner.

`config.h`

```text
#ifndef CONFIG_H
#define CONFIG_H

extern int g_baud;               /* declaration */

#endif
```

`config.c`

```text
#include "config.h"

int g_baud = 115200;             /* definition */
```

`main.c`

```text
#include <stdio.h>
#include "config.h"

int main(void) {
    printf("%d\n", g_baud);
    return 0;
}
```

Build: `gcc main.c config.c -o app`

### Link errors

- Two definitions of the same name → **multiple definition**.
- Declared, never defined → **undefined reference**.
- `int x;` in a header included by two files → multiple definition.
- `static int x;` in a header → each file gets its **own** copy. Silent bug.

### Encapsulate with `static` + accessors

```c
#include <stdio.h>

static int s_speed = 0;                  // private state

void speed_set(int v) { s_speed = (v < 0) ? 0 : v; }
int  speed_get(void)  { return s_speed; }

int main(void) {
    speed_set(-5);
    printf("%d\n", speed_get());         // 0
    speed_set(80);
    printf("%d\n", speed_get());         // 80
    return 0;
}
```

- Prefer this over open globals.
- Easier to protect against ISR races and bad values.

---

## 9. `const` and Where it Lives

- `const` globals go in `.rodata` → Flash on an MCU.
- Writing to them (by casting away `const`) → fault or UB.
- String literals are read-only too.

```c
#include <stdio.h>

static const int  table[4] = {10, 20, 30, 40};     // Flash
static const char msg[]    = "boot ok";            // Flash

int main(void) {
    printf("%d %s\n", table[2], msg);

    // ((int *)table)[0] = 99;     // UB. May hard-fault on MCU.
    return 0;
}
```

### Embedded gotchas

- `const` on a **local** array: still copied to the stack each call (compiler dependent). Add `static`.

```c
// void f(void) {
//     const uint8_t lut[256] = {...};          // may copy 256 B to stack
//     static const uint8_t lut[256] = {...};   // stays in Flash
// }
```

- Pointer to Flash data: `const T *`.
- Table of pointers in Flash: `static const char *const names[] = {...};`
    - Last `const` makes the **pointer array** read-only too. Else it lands in RAM.
- Some MCUs (AVR) need special macros to read Flash data (`PROGMEM`). Cortex-M maps Flash into the normal address space.

---

## 10. `volatile`

- Tells the compiler: this value can change **outside** this code.
- Every read and write must really happen. No caching. No reordering past other volatile accesses.

Use for:

- hardware registers,
- variables shared with ISRs (or signal handlers),
- memory shared with DMA or another core,
- busy-wait counters you do not want deleted.

Without `volatile`, the optimizer may turn this:

```c
// uint8_t ready = 0;                // set by ISR
// while (!ready) { }                // compiler: ready never changes in loop
//                                    //  → reads once → infinite loop
```

With `volatile`:

```c
// volatile uint8_t ready = 0;
// while (!ready) { }                // reads memory every pass
```

Runnable analogue using a signal handler as the "interrupt":

```c
#include <stdio.h>
#include <signal.h>

static volatile sig_atomic_t flag = 0;

static void handler(int sig) {
    (void)sig;
    flag = 1;                         // like an ISR setting a flag
}

int main(void) {
    signal(SIGINT, handler);
    raise(SIGINT);                    // "interrupt" fires

    while (!flag) { }                 // main loop waits on the flag
    printf("flag seen\n");
    return 0;
}
```

### Where `volatile` goes

|Declaration|Meaning|
|---|---|
|`volatile int x;`|x is volatile|
|`volatile uint32_t *p;`|pointer to volatile data (register)|
|`uint32_t *volatile p;`|the pointer itself is volatile|
|`volatile uint32_t *const p;`|fixed address, volatile data|
|`const volatile uint32_t *p;`|read-only hardware register|

> [!warning] `volatile` is not atomic A 32-bit `volatile` on a 32-bit MCU is usually one load or store. A 64-bit one is not. `x++` on a volatile is still read-modify-write. An ISR can slip in between. `volatile` gives no locking. Use disabled interrupts, atomics, or an RTOS primitive.

> [!warning] Do not use `volatile` as a substitute for a memory barrier between cores or between threads.

---

## 11. `restrict`

- C99 promise to the compiler: "only this pointer touches this memory in this scope".
- Lets the compiler optimize better (vectorize, keep values in registers).
- Breaking the promise → UB.

```c
#include <stdio.h>
#include <stddef.h>

void add_arrays(int *restrict dst, const int *restrict a,
                const int *restrict b, size_t n) {
    for (size_t i = 0; i < n; i++)
        dst[i] = a[i] + b[i];
}

int main(void) {
    int a[4] = {1, 2, 3, 4};
    int b[4] = {10, 20, 30, 40};
    int d[4];
    add_arrays(d, a, b, 4);
    for (int i = 0; i < 4; i++) printf("%d ", d[i]);   // 11 22 33 44
    printf("\n");
    return 0;
}
```

- `memcpy` has `restrict` parameters. `memmove` does not. That is why `memmove` is overlap-safe.
- Do not call `add_arrays(a, a, b, n)` with overlap unless you drop `restrict`.

---

## 12. The Stack

- One contiguous block of RAM.
- Each call pushes a **frame**: return address, saved registers, locals, spilled args.
- Return pops it.
- Grows toward lower addresses on ARM and x86.
- Stack overflow on an MCU → no warning. It silently overwrites `.bss`, `.data`, or heap.

What eats stack:

- Deep or unbounded recursion.
- Big local arrays or structs.
- VLAs and `alloca`.
- Many nested calls.
- Interrupts: the CPU pushes registers on the active stack.
- `printf` family with floats. Can use hundreds of bytes.
- RTOS: **each task has its own stack**. Size each one.

Rules:

- Know the worst-case depth.
- Big buffers → `static` or global.
- Pass structs by pointer.
- Keep ISRs small.
- Check with `-fstack-usage`, `-Wstack-usage=N`, `-Wframe-larger-than=N`.

### Stack painting: measure real usage

- Fill the stack with a known pattern at boot.
- Later scan from the far end. Untouched pattern = unused stack.

```c
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define STACK_SIZE  64
#define PAINT       0xA5

static uint8_t fake_stack[STACK_SIZE];     // stands in for the real stack area

static void stack_paint(void) {
    memset(fake_stack, PAINT, sizeof fake_stack);
}

static size_t stack_used(void) {
    size_t free_bytes = 0;
    while (free_bytes < STACK_SIZE && fake_stack[free_bytes] == PAINT)
        free_bytes++;                      // lowest addresses are the deepest
    return STACK_SIZE - free_bytes;
}

int main(void) {
    stack_paint();

    // pretend the program used the top 24 bytes (stack grows down)
    memset(&fake_stack[STACK_SIZE - 24], 0x00, 24);

    printf("used: %zu of %d bytes\n", stack_used(), STACK_SIZE);   // 24 of 64
    return 0;
}
```

- Real use: paint from the stack limit to just below the current SP at reset.
- Log the high-water mark. Keep a safety margin (e.g. 25–30%).
- Many RTOSes do this per task (FreeRTOS: `uxTaskGetStackHighWaterMark`).
- A coincidental `0xA5` in real data makes the measure slightly optimistic.

---

## 13. The Heap

- `malloc` / `free` manage a RAM region.
- On bare metal, `malloc` asks `_sbrk()` for memory. The linker script sets the heap size.
- Problems on small systems:
    - **Fragmentation**: free memory split into pieces too small to use.
    - **Failure**: `malloc` can return `NULL` at runtime.
    - **Timing**: allocation time varies.
    - **Leaks**: forgotten `free`.
    - **Overhead**: each block has a header.

Alternatives:

- Static buffers.
- Fixed-size **memory pool**.
- Allocate once at startup, never free.
- RTOS block allocators.

### Fixed-block pool

```c
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#define POOL_BLOCKS  4
#define BLOCK_SIZE   16

static _Alignas(8) uint8_t pool[POOL_BLOCKS][BLOCK_SIZE];
static uint8_t used[POOL_BLOCKS];

void *pool_alloc(void) {
    for (int i = 0; i < POOL_BLOCKS; i++) {
        if (!used[i]) {
            used[i] = 1;
            return pool[i];
        }
    }
    return NULL;                           // pool empty
}

void pool_free(void *p) {
    if (p == NULL) return;
    for (int i = 0; i < POOL_BLOCKS; i++) {
        if (p == (void *)pool[i]) { used[i] = 0; return; }
    }
}

int main(void) {
    void *b[5];
    for (int i = 0; i < 5; i++) {
        b[i] = pool_alloc();
        printf("alloc %d -> %s\n", i, b[i] ? "ok" : "NULL");   // 5th is NULL
    }

    pool_free(b[1]);
    void *again = pool_alloc();
    printf("reused slot 1: %d\n", again == b[1]);              // 1
    return 0;
}
```

- O(1)-ish, no fragmentation, deterministic, fixed worst case.
- Not thread/ISR safe as written. Add a critical section.

---

## 14. Startup: What Runs Before `main`

On reset a Cortex-M:

1. Loads the **stack pointer** from vector table word 0.
2. Loads the **PC** from word 1 → `Reset_Handler`.

`Reset_Handler` (vendor startup file) then:

1. Copies `.data` initial values from Flash to RAM.
2. Zeros `.bss`.
3. Calls `SystemInit()` (clocks, FPU, etc.).
4. Runs C library init and static constructors (`__libc_init_array`).
5. Calls `main()`.

Typical code:

```c
// extern uint32_t _sidata, _sdata, _edata, _sbss, _ebss;   // from linker script
//
// void Reset_Handler(void) {
//     uint32_t *src = &_sidata;
//     uint32_t *dst = &_sdata;
//     while (dst < &_edata) *dst++ = *src++;      // copy .data
//
//     for (dst = &_sbss; dst < &_ebss; ) *dst++ = 0;   // zero .bss
//
//     SystemInit();
//     __libc_init_array();
//     main();
//     for (;;) { }
// }
```

Same logic, simulated so it runs on a PC:

```c
#include <stdio.h>
#include <stdint.h>

#define N 3

static const uint32_t flash_data_image[N] = {10, 20, 30};   // initial values in "Flash"
static uint32_t ram_data[N];                                 // ".data" in "RAM"
static uint32_t ram_bss[N] = {0xDEAD, 0xDEAD, 0xDEAD};       // power-on garbage

static void startup(void) {
    const uint32_t *src = flash_data_image;
    uint32_t *dst = ram_data;
    while (dst < ram_data + N) *dst++ = *src++;          // copy .data

    for (dst = ram_bss; dst < ram_bss + N; ) *dst++ = 0; // zero .bss
}

int main(void) {
    startup();
    for (int i = 0; i < N; i++)
        printf("data[%d]=%u bss[%d]=%u\n", i, ram_data[i], i, ram_bss[i]);
    return 0;
}
```

Why it matters:

- Big `.data` → slow boot (copy) and wasted RAM.
- Globals are zero **because the startup code zeroes `.bss`**. Custom startup or a "noinit" section changes that.
- Code that runs before `main` (fault handlers, clock setup) must not depend on initialized globals yet.

---

## 15. Linker Script Basics

- Tells the linker where memory is and where sections go.

```text
MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 512K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
}

_estack = ORIGIN(RAM) + LENGTH(RAM);       /* stack top */

SECTIONS
{
    .isr_vector : { KEEP(*(.isr_vector)) } > FLASH
    .text       : { *(.text*) *(.rodata*) } > FLASH

    _sidata = LOADADDR(.data);
    .data : {
        _sdata = .;
        *(.data*)
        _edata = .;
    } > RAM AT > FLASH             /* lives in RAM, stored in Flash */

    .bss : {
        _sbss = .;
        *(.bss*) *(COMMON)
        _ebss = .;
    } > RAM

    ._user_heap_stack : {
        . = ALIGN(8);
        . = . + _Min_Heap_Size;
        . = . + _Min_Stack_Size;       /* link fails if RAM is too small */
    } > RAM
}
```

Ideas:

- **VMA** = address at run time. **LMA** = address stored in the image.
- `.data` has VMA in RAM and LMA in Flash. That is why startup copies it.
- Symbols like `_sdata` are addresses, not variables. In C you take `&_sdata`.
- Reserving heap and stack gives a link error if RAM is too full.

Place data or code in a named section:

```c
// __attribute__((section(".ramfunc")))  void fast_isr(void) { ... }   // run from RAM
// __attribute__((section(".noinit")))   uint32_t boot_count;            // survives reset
// __attribute__((section(".ccmram")))   uint8_t dsp_buf[2048];          // fast core RAM
```

- Needs matching entries in the linker script and (for `.noinit`) no zeroing in startup.

---

## 16. Typical Cortex-M Address Map

|Range|Region|
|---|---|
|`0x0000_0000` – `0x1FFF_FFFF`|Code (Flash, boot alias)|
|`0x2000_0000` – `0x3FFF_FFFF`|SRAM|
|`0x4000_0000` – `0x5FFF_FFFF`|Peripherals|
|`0x6000_0000` – `0x9FFF_FFFF`|External RAM / devices|
|`0xA000_0000` – `0xDFFF_FFFF`|External devices|
|`0xE000_0000` – `0xE00F_FFFF`|Core system (NVIC, SysTick, SCB)|

- Vendors place real Flash at e.g. `0x0800_0000` (STM32) and alias it to `0x0` at boot.
- Peripheral registers are just addresses. This is why `*(volatile uint32_t *)0x40020014` works.
- Read your chip's reference manual memory map.

---

## 17. Quick Decision Guide

|I need...|Use|
|---|---|
|value to persist between calls, hidden to the function|`static` local|
|helper or variable private to a file|`static` at file scope|
|variable shared across files|define once, `extern` in header|
|read-only lookup table|`static const` global|
|table that must not copy to stack|`static const` inside function|
|variable touched by ISR|`volatile` + keep access short or atomic|
|hardware register|`volatile` at fixed address|
|big buffer|static / global, not local|
|dynamic objects on firmware|pool allocator, not `malloc`|
|survive a reset|`.noinit` section|
|fast function|RAM section|

---

## 18. Common Mistakes

- Returning a pointer to a local.
- Using an uninitialized local.
- Assuming `.bss` is zero in a custom startup.
- Writing to a string literal or `const` data.
- Big local arrays → stack overflow.
- Deep recursion.
- Forgetting each RTOS task has its own stack.
- Defining a global in a header → multiple definition.
- `static` variable in a header → hidden duplicate copies.
- `extern` declaration without a matching definition → undefined reference.
- Declaration and definition types differ across files.
- Missing `volatile` on registers or ISR flags.
- Thinking `volatile` makes access atomic.
- `static` local used in code called from ISR and main → race.
- `const` local array copied to the stack each call.
- Relying on `register`.
- Using `malloc` after init in firmware with no failure path.
- Heap and stack growing into each other.
- Ignoring `size` output until Flash is full.
- Shadowed variables hiding a bug.

---

## 19. Embedded Cheat Sheet

- Flash = `text + data`. RAM = `data + bss + heap + stack`.
- `const` → Flash. Zero-init → RAM only. Init'd global → both.
- `static` local → persistent but not reentrant.
- `static` global/function → private to file.
- Define once. `extern` in the header.
- Wrap state in `static` + accessor functions.
- Make tables `static const`. Make pointer tables `const` twice.
- `volatile` for registers and ISR-shared data. Not atomic. Not a lock.
- Big buffers are static. Avoid VLAs and `alloca`.
- Paint the stack. Log the high-water mark.
- Pools over `malloc`. No dynamic allocation in ISRs.
- Know the startup sequence and the linker script.
- Check `size` and the map file every build.
- Use `-fstack-usage`, `-Wstack-usage`, `-Wshadow`.
- Name linker sections for RAM code, `.noinit`, and DMA buffers.

---

## 20. Practice

1. Print addresses of a global, static, local, literal, and `malloc` block. Predict the order.
2. Compile a program. Run `size`. Add a `const` table. Run `size` again. What changed?
3. Make `int t[1000];` global, then `= {1}`, then `const`. Compare `size` each time.
4. Write `visit()` with a `static` counter and a normal counter.
5. Split code into two files. Share a global with `extern`. Cause a "multiple definition" error. Fix it.
6. Put `static int x;` in a header included twice. Show two separate copies.
7. Wrap a variable in `static` plus `get` / `set` functions. Clamp the value.
8. Shadow a variable on purpose. Build with `-Wshadow`.
9. Write to a string literal. Observe the crash. Fix with `char s[]`.
10. Make a static lookup table with `static const` inside a function.
11. Simulate an ISR with `signal` and a `volatile sig_atomic_t` flag.
12. Remove `volatile` and compile with `-O2`. Read the assembly (`-S`).
13. Write a function with `restrict` pointers. Call it with overlap and read the warnings.
14. Paint a fake stack. Fill part of it. Compute high-water mark.
15. Use `-fstack-usage`. Find the biggest stack user.
16. Write a fixed-block pool. Test full, empty, and reuse.
17. Simulate startup: copy `.data`, zero `.bss`.
18. Read a real STM32 or CMSIS `startup_*.s` file. Find `Reset_Handler`.
19. Read a linker script. Find `_sdata`, `_edata`, `_sbss`, `_ebss`, `_estack`.
20. Open a `.map` file. Find your largest function and variable.
21. Place a variable in `.noinit`. Explain what the startup must not do.
22. Draw the memory map of your target MCU.
23. Explain why an init'd global costs both Flash and RAM.
24. Explain why `static` in a header is a trap.

---

## Quick Revision

- Sections: `.text` code, `.rodata` const, `.data` init'd, `.bss` zero, heap, stack.
- Flash holds code, const, and `.data` init image. RAM holds `.data`, `.bss`, heap, stack.
- Startup copies `.data`, zeros `.bss`, then calls `main`.
- Scope ≠ lifetime ≠ linkage.
- `static` local → lifetime. `static` global → linkage.
- `extern` declares. One `.c` defines.
- Local = stack = garbage. Global/static = zero. `malloc` = garbage.
- `const` → Flash. `static const` for tables. Pointer tables need two `const`.
- `volatile` → real reads and writes. Not atomic.
- `restrict` → no aliasing promise.
- Stack overflows silently on MCUs. Measure it by painting.
- Heap on small MCUs → pools.
- Linker script maps sections to Flash and RAM. `size` and the map file show the cost.

| previous: [[C-08 Preprocessor & Macros]] | next: [[C-10 File IO & Standard Library]] |
| ---------------------------------------- | ----------------------------------------- |
