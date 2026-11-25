> [!info] 
> title: Preprocessor & Macros 
> tags: [C, preprocessor, macros] 
> chapter: 8

| previous: [[C-07 Structs, Unions, Enums]] | next: [[C-09 Memory Layout & Storage Classes]] |
| ----------------------------------------- | ---------------------------------------------- |
## 1. What is the Preprocessor

- Runs **before** the compiler.
- Works on **text**. Knows nothing about C types.
- Lines starting with `#` are directives.
- Output is plain C, with all directives expanded.

Build pipeline:

```text
main.c → [preprocess] → main.i → [compile] → main.s → [assemble] → main.o → [link] → app
          #include           C code        assembly       object        executable
          #define
          #if
```

Commands:

```text
gcc -E main.c -o main.i       # preprocess only. Read the result.
gcc -S main.c                 # to assembly
gcc -c main.c                 # to object file
gcc main.c -o app             # full build
gcc -save-temps main.c        # keep all stages
gcc -dM -E - < /dev/null      # list predefined macros
```

> [!note] When a macro misbehaves, run `gcc -E` and read what it became.

---

## 2. `#include`

- Pastes the file's text at that spot.

|Form|Searches|
|---|---|
|`#include <stdio.h>`|system include paths|
|`#include "my.h"`|current folder first, then system|

```c
#include <stdio.h>

int main(void) {
    printf("hello\n");
    return 0;
}
```

- Add search paths with `-I path`.
- Include only what you use.
- Headers hold declarations. Not definitions.

### Include guards

- Stop the same header from being pasted twice.

```text
#ifndef UART_H
#define UART_H

/* declarations */

#endif /* UART_H */
```

- Name: `FILENAME_H`. Unique per project.
- `#pragma once` also works. Not standard, but widely supported.

### C++ compatibility

```text
#ifdef __cplusplus
extern "C" {
#endif

void uart_init(void);

#ifdef __cplusplus
}
#endif
```

- Stops C++ from mangling C function names.

---

## 3. Object-like Macros: `#define`

- Name replaced by text.
- No `=`. No `;`. No type.

```c
#include <stdio.h>

#define PI          3.14159
#define MAX_USERS   10
#define GREETING    "hello"
#define NEWLINE     '\n'

int main(void) {
    printf("%.2f %d %s%c", PI, MAX_USERS, GREETING, NEWLINE);
    return 0;
}
```

### Parenthesize expressions

```c
#include <stdio.h>

#define BAD   10 + 5
#define GOOD  (10 + 5)

int main(void) {
    printf("%d\n", BAD * 2);     // 10 + 5 * 2 = 20
    printf("%d\n", GOOD * 2);    // 30
    return 0;
}
```

### `#undef`

```c
#include <stdio.h>

#define LIMIT 5

int main(void) {
    printf("%d\n", LIMIT);
#undef LIMIT
#define LIMIT 9
    printf("%d\n", LIMIT);
    return 0;
}
```

### Rules

- `UPPER_CASE` names.
- Add units in the name: `TIMEOUT_MS`, `CLOCK_HZ`.
- Use `U`, `UL`, `f` suffixes: `#define MASK 0xFFU`.
- Prefer `enum` or `const` when possible.

---

## 4. Function-like Macros

- Looks like a call. Is text substitution.
- **No space** between name and `(` in the definition.

```c
#include <stdio.h>

#define SQUARE(x)   ((x) * (x))
#define MAX(a, b)   ((a) > (b) ? (a) : (b))
#define MIN(a, b)   ((a) < (b) ? (a) : (b))

int main(void) {
    printf("%d\n", SQUARE(2 + 3));     // 25
    printf("%d\n", MAX(4, 9));         // 9
    printf("%d\n", MIN(4, 9));         // 4
    return 0;
}
```

### Rule: parenthesize every argument and the whole result

```c
#include <stdio.h>

#define SQ_BAD(x)    x * x
#define SQ_GOOD(x)   ((x) * (x))

int main(void) {
    printf("%d\n", SQ_BAD(2 + 3));      // 2 + 3 * 2 + 3 = 11
    printf("%d\n", SQ_GOOD(2 + 3));     // 25
    printf("%d\n", 100 / SQ_BAD(5));    // 100 / 5 * 5 = 100
    printf("%d\n", 100 / SQ_GOOD(5));   // 4
    return 0;
}
```

### Double evaluation

- Argument text is pasted **each time** it appears.
- Side effects run twice.

```c
#include <stdio.h>

#define MAX(a, b)   ((a) > (b) ? (a) : (b))

int main(void) {
    int i = 5, j = 3;
    int m = MAX(i++, j++);
    printf("m=%d i=%d j=%d\n", m, i, j);    // m=6 i=7 j=4
    return 0;
}
```

- Expected `m = 5`. Got `6`.
- Never pass `i++`, function calls with side effects, or `rand()`.

### Multi-statement macros → `do { } while (0)`

```c
#include <stdio.h>

#define SWAP_BAD(a, b)  int t = (a); (a) = (b); (b) = t;
#define SWAP(a, b)      do { int t = (a); (a) = (b); (b) = t; } while (0)

int main(void) {
    int x = 1, y = 2;

    if (x < y)
        SWAP(x, y);          // works as one statement
    else
        printf("no swap\n");

    printf("%d %d\n", x, y); // 2 1
    return 0;
}
```

- `SWAP_BAD` would break `if/else` and declare `t` twice.
- `do { } while (0)` makes the macro need a `;` and act as one statement.
- Multi-line macros end each line with `\`.

```c
#include <stdio.h>

#define PRINT_PAIR(a, b)                    \
    do {                                    \
        printf("a = %d\n", (a));            \
        printf("b = %d\n", (b));            \
    } while (0)

int main(void) {
    PRINT_PAIR(3, 4);
    return 0;
}
```

- No space after the `\`.

---

## 5. Stringify `#` and Token Paste `##`

- `#x` → turns argument into a string literal.
- `a ## b` → glues tokens into one name.

```c
#include <stdio.h>

#define STR(x)          #x
#define PRINT_INT(x)    printf(#x " = %d\n", (x))
#define CAT(a, b)       a##b

int main(void) {
    int speed = 120;
    int ab = 7;

    printf("%s\n", STR(hello world));     // hello world
    PRINT_INT(speed);                      // speed = 120
    PRINT_INT(speed * 2);                  // speed * 2 = 240
    printf("%d\n", CAT(a, b));             // value of ab → 7
    return 0;
}
```

### Generate code

```c
#include <stdio.h>

#define DECLARE_COUNTER(name)                       \
    static int counter_##name = 0;                  \
    static int next_##name(void) { return ++counter_##name; }

DECLARE_COUNTER(a)
DECLARE_COUNTER(b)

int main(void) {
    next_a();
    printf("%d %d\n", next_a(), next_b());     // 2 1
    return 0;
}
```

### Expand first, then stringify: two-level macro

- Plain `STR(VER)` gives `"VER"`.
- Extra layer expands `VER` first.

```c
#include <stdio.h>

#define STR(x)    #x
#define XSTR(x)   STR(x)

#define VERSION   3

int main(void) {
    printf("%s\n", STR(VERSION));      // VERSION
    printf("%s\n", XSTR(VERSION));     // 3
    return 0;
}
```

---

## 6. Variadic Macros and Logging

- `...` takes any number of arguments.
- `__VA_ARGS__` pastes them.

```c
#include <stdio.h>

#define LOG(fmt, ...) \
    printf("[%s:%d %s] " fmt "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)

int main(void) {
    LOG("start");
    LOG("value = %d, name = %s", 42, "adc");
    return 0;
}
```

- `##__VA_ARGS__` drops the trailing comma when there are no extra arguments. GNU extension (GCC / Clang).
- C23: `__VA_OPT__(,)` does the same in standard C.

### Log levels

```c
#include <stdio.h>

#define LOG_LEVEL_NONE   0
#define LOG_LEVEL_ERROR  1
#define LOG_LEVEL_DEBUG  2

#define LOG_LEVEL        LOG_LEVEL_DEBUG

#if LOG_LEVEL >= LOG_LEVEL_ERROR
#define LOG_E(...)  printf("E: " __VA_ARGS__)
#else
#define LOG_E(...)  ((void)0)
#endif

#if LOG_LEVEL >= LOG_LEVEL_DEBUG
#define LOG_D(...)  printf("D: " __VA_ARGS__)
#else
#define LOG_D(...)  ((void)0)
#endif

int main(void) {
    LOG_E("bad value %d\n", -1);
    LOG_D("x = %d\n", 5);
    return 0;
}
```

- Set `LOG_LEVEL` to `0` → both lines vanish from the binary. No Flash used.

---

## 7. Predefined Macros

|Macro|Value|
|---|---|
|`__FILE__`|current file name (string)|
|`__LINE__`|current line (int)|
|`__func__`|current function name (C99, not a macro)|
|`__DATE__`|compile date|
|`__TIME__`|compile time|
|`__STDC__`|`1` if standard compiler|
|`__STDC_VERSION__`|`199901L`, `201112L`, `201710L`, ...|
|`__cplusplus`|defined only in C++|
|`__GNUC__`|defined by GCC / Clang|
|`__ARM_ARCH`|defined by ARM compilers|

```c
#include <stdio.h>

int main(void) {
    printf("file : %s\n", __FILE__);
    printf("line : %d\n", __LINE__);
    printf("func : %s\n", __func__);
    printf("built: %s %s\n", __DATE__, __TIME__);
#ifdef __STDC_VERSION__
    printf("std  : %ld\n", (long)__STDC_VERSION__);
#endif
    return 0;
}
```

---

## 8. Conditional Compilation

- Include or drop code **at compile time**.
- Dropped code is not compiled and not in the binary.

|Directive|Meaning|
|---|---|
|`#if expr`|if expression is nonzero|
|`#ifdef NAME`|if `NAME` is defined|
|`#ifndef NAME`|if `NAME` is not defined|
|`#elif expr`|else if|
|`#else`|else|
|`#endif`|end|
|`defined(NAME)`|1 if defined. Use inside `#if`|

```c
#include <stdio.h>

#define MODE 2

int main(void) {
#if MODE == 1
    printf("mode 1\n");
#elif MODE == 2
    printf("mode 2\n");
#else
    printf("other\n");
#endif

#if defined(MODE) && MODE > 1
    printf("MODE is above 1\n");
#endif

#ifndef DEBUG
    printf("release build\n");
#endif
    return 0;
}
```

### Rules

- `#if` can only use integer constants and macros. No `sizeof`, no casts, no variables.
- Undefined name in `#if` counts as `0`. Silent bugs. Use `-Wundef`.
- `#if FEATURE` needs a value. `#ifdef FEATURE` only needs a definition.
    - Pick one style and keep it.
- `-DNAME` → defines `NAME` as `1`. `-DNAME=5` → defines as `5`.

### `#if 0` to disable code

- Better than `/* */` because it nests.

```c
#include <stdio.h>

int main(void) {
#if 0
    printf("this never builds\n");
#endif
    printf("done\n");
    return 0;
}
```

### `#error` and `#warning`

- Stop or warn at compile time.

```c
// #if !defined(BOARD_A) && !defined(BOARD_B)
// #error "Select a board: define BOARD_A or BOARD_B"
// #endif
//
// #warning "Using default clock"       // GNU extension, C23 standard
```

### Default values for build flags

```c
#include <stdio.h>

#ifndef BUF_SIZE
#define BUF_SIZE 32           // override with -DBUF_SIZE=64
#endif

static char buf[BUF_SIZE];

int main(void) {
    printf("%zu\n", sizeof buf);
    return 0;
}
```

---

## 9. Debug-only Code and Assertions

```c
#include <stdio.h>

#define DEBUG 1

#if DEBUG
#define ASSERT(cond) \
    do { if (!(cond)) printf("ASSERT FAILED: %s (%s:%d)\n", #cond, __FILE__, __LINE__); } while (0)
#else
#define ASSERT(cond) ((void)0)
#endif

int main(void) {
    int x = 5;
    ASSERT(x > 0);
    ASSERT(x > 10);          // prints a message
    return 0;
}
```

- Embedded `ASSERT` often: log, then loop forever or reset.

### `<assert.h>`

- `assert(cond)` aborts if false.
- Removed completely when `NDEBUG` is defined (`-DNDEBUG`).
- Do not put side effects inside `assert`.

```c
#include <stdio.h>
#include <assert.h>

static_assert(sizeof(int) >= 4, "int must be at least 4 bytes");   // compile time

int main(void) {
    int x = 3;
    assert(x == 3);                // runtime
    printf("ok\n");
    return 0;
}
```

---

## 10. X-Macros

- Define a list once.
- Expand it many times for different outputs.
- Keeps enum, strings, tables in sync.

```c
#include <stdio.h>

#define STATE_LIST(X)         \
    X(IDLE,  "idle")          \
    X(RUN,   "run")           \
    X(ERROR, "error")

// 1. enum
typedef enum {
#define X(name, str) STATE_##name,
    STATE_LIST(X)
#undef X
    STATE_COUNT
} State;

// 2. string table
static const char *const state_str[] = {
#define X(name, str) [STATE_##name] = str,
    STATE_LIST(X)
#undef X
};

int main(void) {
    for (int s = 0; s < STATE_COUNT; s++)
        printf("%d = %s\n", s, state_str[s]);
    return 0;
}
```

- Add one line to `STATE_LIST` → enum and strings both update.
- Common for error codes, commands, register maps, event names.

---

## 11. Useful Macro Toolkit

```c
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

#define BIT(n)                (1U << (n))
#define ARRAY_SIZE(a)         (sizeof(a) / sizeof((a)[0]))
#define MIN(a, b)             ((a) < (b) ? (a) : (b))
#define MAX(a, b)             ((a) > (b) ? (a) : (b))
#define CLAMP(x, lo, hi)      MIN(MAX((x), (lo)), (hi))
#define UNUSED(x)             ((void)(x))
#define IS_POW2(n)            ((n) != 0 && (((n) & ((n) - 1)) == 0))
#define ALIGN_UP(x, a)        (((x) + ((a) - 1U)) & ~((a) - 1U))
#define KB(n)                 ((n) * 1024UL)

int main(void) {
    int data[10];
    printf("%u\n", BIT(5));                    // 32
    printf("%zu\n", ARRAY_SIZE(data));         // 10
    printf("%d\n", CLAMP(150, 0, 100));        // 100
    printf("%d\n", IS_POW2(64));               // 1
    printf("%u\n", ALIGN_UP(13U, 8U));         // 16
    printf("%lu\n", KB(4));                    // 4096
    UNUSED(data);
    return 0;
}
```

### `container_of`

- Get the struct pointer from a member pointer.
- Used in the Linux kernel and RTOS lists.

```c
#include <stdio.h>
#include <stddef.h>

#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

typedef struct { int a; int b; } Pair;

int main(void) {
    Pair p = {10, 20};
    int *pb = &p.b;
    Pair *pp = container_of(pb, Pair, b);
    printf("%d %d\n", pp->a, pp->b);      // 10 20
    return 0;
}
```

### Safe `MIN` with GNU extensions

- `__typeof__` + statement expression `({ ... })`.
- Evaluates each argument once.
- GCC / Clang only.

```c
#include <stdio.h>

#define MIN_SAFE(a, b) ({           \
    __typeof__(a) _a = (a);         \
    __typeof__(b) _b = (b);         \
    _a < _b ? _a : _b;              \
})

int main(void) {
    int i = 5;
    int m = MIN_SAFE(i++, 10);
    printf("m=%d i=%d\n", m, i);     // m=5 i=6
    return 0;
}
```

---

## 12. `#pragma` and Attributes

- `#pragma` → compiler-specific hints. Unknown pragmas are ignored (often silently).

```c
#include <stdio.h>
#include <stdint.h>

#pragma pack(push, 1)
typedef struct {
    uint8_t  a;
    uint32_t b;
    uint8_t  c;
} Packed;
#pragma pack(pop)

int main(void) {
    printf("%zu\n", sizeof(Packed));      // 6
    return 0;
}
```

Common GCC / Clang attributes:

|Attribute|Use|
|---|---|
|`__attribute__((packed))`|no struct padding|
|`__attribute__((aligned(n)))`|force alignment|
|`__attribute__((section(".name")))`|place in a linker section|
|`__attribute__((weak))`|overridable default (e.g. handlers)|
|`__attribute__((used))`|keep symbol even if unused|
|`__attribute__((noreturn))`|function never returns|
|`__attribute__((always_inline))`|force inline|
|`__attribute__((unused))`|silence unused warning|
|`__attribute__((noinline))`|block inlining|

```c
#include <stdio.h>

__attribute__((weak)) void on_error(void) {
    printf("default handler\n");          // user can override
}

__attribute__((noreturn)) void fatal(void) {
    printf("fatal\n");
    for (;;) { }
}

int main(void) {
    on_error();
    (void)fatal;                          // not called here
    return 0;
}
```

> [!note] Embedded `weak` default ISR handlers, `section` for placing RAM functions, `aligned` for DMA buffers. Wrap attributes in macros so code ports across compilers: `#define PACKED __attribute__((packed))`

---

## 13. Macros vs `inline` vs `const` vs `enum`

|Need|Use|
|---|---|
|Integer constant, related set|`enum`|
|Typed constant, debug-friendly|`static const`|
|Value used in `#if`, array size, or case label|`#define` / `enum`|
|Short typed function|`static inline`|
|Stringify, paste, generate code, conditional builds|macro|
|Works on any type|macro (or `_Generic`)|
|Compile-time check|`_Static_assert`|

Why `inline` beats function-like macros:

- Type checked.
- Arguments evaluated once.
- Shows up in the debugger.
- No precedence bugs.

> [!note] MISRA C Discourages function-like macros. Prefer `static inline`.

---

## 14. Macro Pitfalls

```c
// 1. Missing parentheses
// #define DOUBLE(x) x + x
// DOUBLE(3) * 2        → 3 + 3 * 2 = 9, not 12

// 2. Double evaluation
// MAX(i++, j++)

// 3. Trailing semicolon inside macro
// #define INC(x) ((x)++);
// if (a) INC(b); else ...      → syntax error

// 4. Multi-statement without do { } while (0)

// 5. Comma in an argument not protected by parentheses
// MAX(f(1, 2), 3)              ok: parentheses protect the comma
// FOO(int a, b)                breaks: macro sees 2 args

// 6. Hidden control flow
// #define CHECK(x) if (!(x)) return -1;    // returns from the caller!

// 7. Name clashes: macro replaces every use of the word
// #define count 10             // breaks any variable named count

// 8. No type checking, weird error messages
```

- Macro names are global. Use a project prefix: `UART_`, `APP_`.
- Never `#define` a keyword or a standard function name.
- Do not use leading underscore + capital (`_Foo`). Reserved.

---

## 15. Embedded Patterns

### Board / feature selection

```c
#include <stdio.h>

#define BOARD_NUCLEO 1
// #define BOARD_CUSTOM 1

#define USE_UART 1
#define USE_SPI  0

#if defined(BOARD_NUCLEO)
#define LED_PIN   5
#define CLOCK_HZ  84000000UL
#elif defined(BOARD_CUSTOM)
#define LED_PIN   13
#define CLOCK_HZ  48000000UL
#else
#error "No board selected"
#endif

int main(void) {
    printf("led=%d clock=%lu\n", LED_PIN, CLOCK_HZ);
#if USE_UART
    printf("uart enabled\n");
#endif
#if USE_SPI
    printf("spi enabled\n");
#endif
    return 0;
}
```

### Pin and register helpers

```c
#include <stdio.h>
#include <stdint.h>

#define SET_BIT(reg, n)     ((reg) |=  (1U << (n)))
#define CLR_BIT(reg, n)     ((reg) &= ~(1U << (n)))
#define TOG_BIT(reg, n)     ((reg) ^=  (1U << (n)))
#define GET_BIT(reg, n)     (((reg) >> (n)) & 1U)

#define MASK(width)                 ((1U << (width)) - 1U)
#define FIELD_GET(reg, pos, width)  (((reg) >> (pos)) & MASK(width))
#define FIELD_SET(reg, pos, width, val) \
    ((reg) = ((reg) & ~(MASK(width) << (pos))) | (((val) & MASK(width)) << (pos)))

int main(void) {
    uint32_t reg = 0;
    SET_BIT(reg, 3);
    FIELD_SET(reg, 8, 4, 0xA);
    printf("0x%X\n", reg);                    // 0xA08
    printf("%u\n", FIELD_GET(reg, 8, 4));     // 10
    return 0;
}
```

Hardware form of a register macro:

```c
// #define REG32(addr)     (*(volatile uint32_t *)(addr))
// #define GPIOA_ODR       REG32(0x40020014UL)
//
// SET_BIT(GPIOA_ODR, 5);
```

### Critical section macros

```c
// #define ENTER_CRITICAL()   __disable_irq()
// #define EXIT_CRITICAL()    __enable_irq()
```

- Keep macros small. Hide the platform call behind a stable name.

### Size and layout checks

```c
#include <stdint.h>

typedef struct { uint8_t id; uint8_t len; uint16_t crc; } Header;

_Static_assert(sizeof(Header) == 4, "Header size changed");

int main(void) { return 0; }
```

### Header layout for a module

> [!note] Multi-file. Not for Code Runner.

```text
/* uart.h */
#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <stddef.h>

#define UART_BAUD_DEFAULT   115200U

typedef enum { UART_OK = 0, UART_ERR_BUSY = -1 } UartStatus;

UartStatus uart_init(uint32_t baud);
UartStatus uart_write(const uint8_t *data, size_t len);

#endif /* UART_H */
```

- Include guard.
- Include only what the header itself needs.
- Declarations only. No variable definitions.
- Constants and types here. Private details in the `.c` file.

---

## 16. Common Mistakes

- Macro args not parenthesized.
- Whole macro expression not parenthesized.
- `;` at the end of a macro definition.
- `=` in `#define NAME = 5`.
- Space between macro name and `(`: `#define F (x) ...` is an object-like macro.
- Side effects in macro arguments.
- Multi-statement macro without `do { } while (0)`.
- Missing include guard. Duplicate definitions.
- Definitions (not declarations) in headers.
- Undefined macro silently treated as 0 in `#if`.
- `#if SIZEOF_X` uses `sizeof`. Not allowed.
- Hidden `return` / `goto` inside a macro.
- Macro name colliding with a variable or function.
- Long macros hard to debug. Use `gcc -E`.
- `STR(MACRO)` expecting expansion. Use two levels.
- Depending on `##__VA_ARGS__` in portable code.
- Using `assert` with side effects.
- Different build flags between files. Struct layouts differ.

---

## 17. Embedded Cheat Sheet

- `gcc -E` when a macro looks wrong.
- Guard every header. Keep headers declarations-only.
- `enum` and `static const` before `#define` for constants.
- `static inline` before function-like macros.
- Parenthesize arguments and the result.
- `do { } while (0)` for multi-statement macros.
- Prefix macro names. Add units to constant names.
- Wrap attributes: `PACKED`, `ALIGNED(n)`, `SECTION(x)`.
- Build flags: `-DBOARD_X`, `-DNDEBUG`, `-DLOG_LEVEL=2`. Defaults with `#ifndef`.
- Use `-Wundef` to catch undefined names in `#if`.
- `#error` when no board or feature is selected.
- `_Static_assert` for sizes and offsets.
- X-macros to keep enums and tables in sync.
- Log macros that compile out in release.
- Never put side effects in `assert` or macro args.

---

## 18. Practice

1. Write `SQUARE`, `CUBE`, `ABS` macros. Break them with bad arguments, then fix.
2. Show the double evaluation bug in `MAX(i++, j++)`.
3. Write `SWAP(a, b)` with `do { } while (0)`. Use it in an `if/else`.
4. Run `gcc -E` on a file with macros. Read the output.
5. Print a variable name and value with `#x`.
6. Use `##` to generate three counter functions.
7. Show `STR(VERSION)` vs `XSTR(VERSION)`.
8. Write a `LOG` macro with file, line, function. Test with and without arguments.
9. Add log levels. Verify with `gcc -E` that disabled logs vanish.
10. Print `__FILE__`, `__LINE__`, `__func__`, `__DATE__`.
11. Use `#if` / `#elif` / `#else` to choose a clock frequency by board.
12. Add `#error` for "no board selected". Test by removing the define.
13. Build with `-DDEBUG` and without. Compare output.
14. Use `-DBUF_SIZE=64` with an `#ifndef` default.
15. Build an X-macro for an enum, a name table, and a "handler" table.
16. Write `BIT`, `SET_BIT`, `CLR_BIT`, `FIELD_GET`, `FIELD_SET`.
17. Write `ALIGN_UP` and test with 5, 8, 13.
18. Write `container_of`. Use it on a struct member.
19. Compare `MIN` macro, `static inline` MIN, and a GNU `MIN_SAFE` with `i++`.
20. Add `_Static_assert` to lock a struct size.
21. Write a header with include guard and a C++ `extern "C"` block.
22. Write an `ASSERT` macro that logs and loops forever.
23. Use `#pragma pack` and `__attribute__((packed))`. Compare sizes.
24. Write a weak default handler. Override it in another file.
25. Explain why `#define F (x) ((x)+1)` fails.

---

## Quick Revision

- Preprocessor = text substitution before compile. `gcc -E` shows the result.
- `#include <>` system. `#include ""` local. Guard every header.
- `#define NAME value`. No `=`, no `;`.
- Function-like macros: parenthesize each argument and the whole body.
- Arguments are pasted as text. Side effects can run twice.
- Multi-statement macro → `do { ... } while (0)`.
- `#x` → string. `a##b` → glue. Two-level macro to expand first.
- `__FILE__`, `__LINE__`, `__func__` for logs.
- `#if`, `#ifdef`, `#elif`, `#else`, `#endif` choose code at build time.
- `#error` for bad configurations. `-D` sets flags. `-Wundef` catches typos.
- X-macros → one list, many outputs.
- Prefer `enum`, `const`, `static inline`. Use macros for what only macros can do.
- `#pragma` and `__attribute__` are compiler-specific. Wrap them.
- `_Static_assert` checks sizes at compile time.

| previous: [[C-07 Structs, Unions, Enums]] | next: [[C-09 Memory Layout & Storage Classes]] |
| ----------------------------------------- | ---------------------------------------------- |

