> [!info]
> Title: Variables & Data Types
> Tags: C, basics, data-types
> Chapter 1
## 1. Variable
- Named storage in memory.
- Has a type, a name, a value, an address.
- Type decides size and how bits are read.
```c
#include <stdio.h>

int main(void) {
    int age = 25;
    printf("value: %d\n", age);
    printf("address: %p\n", (void *)&age);
    printf("size: %zu bytes\n", sizeof(age));
    return 0;
}
```

## 2. Declaration vs Definition vs Initialization
- **Declaration** → tells compiler name + type.
- **Definition** → allocates memory.
- **Initialization** → gives first value.

```c
#include <stdio.h>

extern int x;      // declaration only
int y;             // definition (global, auto 0)
int z = 10;        // definition + initialization
int x = 5;         // definition of x

int main(void) {
    int a;         // local, garbage value
    int b = 3;     // local, initialized
    printf("%d %d %d %d\n", x, y, z, b);
    return 0;
}
```

> [!warning] Uninitialized locals Local variables hold garbage. Globals and statics start at 0. Always initialize locals.

---

## 3. Naming Rules
- Letters, digits, underscore.
- Cannot start with digit.
- Case sensitive.
- Cannot use keywords (`int`, `return`, ...).
- Avoid leading `_` + capital. Reserved.
```c
int count;        // ok
int _tmp;         // ok
int Count;        // different from count
// int 2fast;     // error
// int int;       // error
```

Style for embedded:
- `snake_case` for variables.
- `UPPER_CASE` for macros/constants.
- Meaningful names. `adc_value` not `v`.

---

## 4. Basic Data Types

| Type          | Typical size  | Use                   |
| ------------- | ------------- | --------------------- |
| `char`        | 1 byte        | character / small int |
| `short`       | 2 bytes       | small int             |
| `int`         | 4 bytes*      | default int           |
| `long`        | 4 or 8 bytes* | larger int            |
| `long long`   | 8 bytes       | very large int        |
| `float`       | 4 bytes       | single precision      |
| `double`      | 8 bytes       | double precision      |
| `long double` | 8–16 bytes*   | extended precision    |
| `_Bool`       | 1 byte        | true / false          |

* Size depends on compiler and platform.
- Standard only guarantees: `sizeof(char) == 1`.
- Order: `char <= short <= int <= long <= long long`.

```c
#include <stdio.h>

int main(void) {
    printf("char      : %zu\n", sizeof(char));
    printf("short     : %zu\n", sizeof(short));
    printf("int       : %zu\n", sizeof(int));
    printf("long      : %zu\n", sizeof(long));
    printf("long long : %zu\n", sizeof(long long));
    printf("float     : %zu\n", sizeof(float));
    printf("double    : %zu\n", sizeof(double));
    return 0;
}
```

> [!note] Embedded On 8/16-bit MCUs `int` can be 2 bytes. On ARM Cortex-M `int` is 4 bytes. Never assume. Use fixed-width types.

---

## 5. Signed vs Unsigned
- `signed` → positive and negative. Default for `int`.
- `unsigned` → only 0 and positive. Double the positive range.
- 8-bit signed: -128 to 127.
- 8-bit unsigned: 0 to 255.
- Signed uses two's complement (almost always).

```c
#include <stdio.h>

int main(void) {
    signed char   s = -1;
    unsigned char u = 255;
    printf("%d %u\n", s, u);

    u = u + 1;                 // wraps around
    printf("wrap: %u\n", u);   // 0
    return 0;
}
```

> [!warning] Unsigned overflow wraps. Well defined. Signed overflow is **undefined behavior**.

> [!note] `char` signedness Plain `char` may be signed or unsigned. Depends on compiler/platform. Use `signed char` / `unsigned char` / `uint8_t` when it matters.

---

## 6. Limits

- `<limits.h>` → integer limits.
- `<float.h>` → float limits.

```c
#include <stdio.h>
#include <limits.h>
#include <float.h>

int main(void) {
    printf("INT_MIN  : %d\n", INT_MIN);
    printf("INT_MAX  : %d\n", INT_MAX);
    printf("UINT_MAX : %u\n", UINT_MAX);
    printf("CHAR_BIT : %d\n", CHAR_BIT);
    printf("FLT_MAX  : %e\n", FLT_MAX);
    printf("DBL_EPS  : %e\n", DBL_EPSILON);
    return 0;
}
```

---

## 7. Fixed-Width Types (`stdint.h`)

- Exact size. Same on every platform.
- **Use these in embedded code.**

| Type                   | Size    | Range              |
| ---------------------- | ------- | ------------------ |
| `int8_t` / `uint8_t`   | 1 byte  | -128..127 / 0..255 |
| `int16_t` / `uint16_t` | 2 bytes | ±32K / 0..65535    |
| `int32_t` / `uint32_t` | 4 bytes | ±2.1B / 0..4.29B   |
| `int64_t` / `uint64_t` | 8 bytes | very large         |

Other useful ones:

- `size_t` → result of `sizeof`. Unsigned.
- `uintptr_t` → integer big enough to hold a pointer.
- `ptrdiff_t` → pointer difference. Signed.

```c
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

int main(void) {
    uint8_t  reg8  = 0xFF;
    uint16_t reg16 = 0xABCD;
    uint32_t reg32 = 0xDEADBEEF;

    printf("%u\n", reg8);
    printf("0x%X\n", reg16);
    printf("0x%" PRIX32 "\n", reg32);   // portable format
    return 0;
}
```

---

## 8. Floating Point

- `float` → ~7 digits precision.
- `double` → ~15 digits precision.
- Stored as sign + exponent + mantissa (IEEE 754).
- Not exact. Never compare with `==`.

```c
#include <stdio.h>
#include <math.h>

int main(void) {
    float a = 0.1f + 0.2f;
    printf("%.10f\n", a);

    double x = 0.1 + 0.2;
    printf("%.17f\n", x);
    printf("equal? %d\n", x == 0.3);                  // 0
    printf("close? %d\n", fabs(x - 0.3) < 1e-9);      // 1
    return 0;
}
```

> [!note] Embedded Many MCUs have no FPU. Float math is slow (software emulation). Prefer integers or fixed-point when possible. Use `f` suffix: `3.14f` is float. `3.14` is double.

---

## 9. Boolean

- C99 → `_Bool`.
- `<stdbool.h>` gives `bool`, `true`, `false`.
- Any nonzero value is true in conditions.

```c
#include <stdio.h>
#include <stdbool.h>

int main(void) {
    bool flag = true;
    bool other = 5;          // becomes 1
    printf("%d %d\n", flag, other);
    return 0;
}
```

---

## 10. Characters

- `char` is a small integer.
- `'A'` is just the number 65 (ASCII).
- Single quotes → char. Double quotes → string.

```c
#include <stdio.h>

int main(void) {
    char c = 'A';
    printf("%c = %d\n", c, c);
    c = c + 1;
    printf("%c\n", c);       // B
    printf("%c\n", 'a' - 32);// A
    return 0;
}
```

---

## 11. Format Specifiers

|Specifier|Type|
|---|---|
|`%d` / `%i`|`int`|
|`%u`|`unsigned int`|
|`%ld`|`long`|
|`%lld`|`long long`|
|`%lu`|`unsigned long`|
|`%zu`|`size_t`|
|`%f`|`float` / `double`|
|`%lf`|`double` (for `scanf`)|
|`%e`|scientific|
|`%c`|`char`|
|`%s`|string|
|`%p`|pointer|
|`%x` / `%X`|hex|
|`%o`|octal|
|`%%`|literal `%`|

```c
#include <stdio.h>

int main(void) {
    int n = 255;
    printf("dec: %d\n", n);
    printf("hex: %x\n", n);
    printf("HEX: 0x%X\n", n);
    printf("oct: %o\n", n);
    printf("pad: [%5d]\n", n);
    printf("zero pad: [%05d]\n", n);
    printf("left: [%-5d]\n", n);
    printf("float: %.2f\n", 3.14159);
    return 0;
}
```

> [!warning] Wrong specifier = undefined behavior. Always match specifier with type.

---

## 12. Literals

Integer:

- Decimal: `10`
- Octal: `012` (leading 0)
- Hex: `0xFF`
- Binary: `0b1010` (GCC/C23, not standard before C23)

Suffixes:

- `u` → unsigned
- `l` → long
- `ll` → long long
- `f` → float

```c
#include <stdio.h>

int main(void) {
    int dec = 10;
    int oct = 012;      // 10
    int hex = 0xA;      // 10
    int bin = 0b1010;   // 10 (compiler ext.)

    unsigned int  u  = 100u;
    long          l  = 100000L;
    long long     ll = 5000000000LL;
    float         f  = 1.5f;

    printf("%d %d %d %d\n", dec, oct, hex, bin);
    printf("%u %ld %lld %.1f\n", u, l, ll, f);
    return 0;
}
```

> [!warning] Leading zero `010` is octal = 8. Not 10.

---

## 13. Constants

Three ways:

**1. `const`**
- Typed. Read-only variable.
- Compiler checks type.

**2. `#define`**
- Text replacement by preprocessor.
- No type. No memory. No scope.

**3. `enum`**
- Named integer constants.
- Good for related groups.

```c
#include <stdio.h>

#define MAX_USERS 10
#define PI 3.14159

enum Color { RED, GREEN, BLUE };          // 0, 1, 2
enum Level { LOW = 1, MID = 5, HIGH = 10 };

int main(void) {
    const int limit = 100;
    // limit = 200;       // error

    printf("%d %.2f\n", MAX_USERS, PI);
    printf("%d %d\n", GREEN, HIGH);
    return 0;
}
```

> [!note] Embedded `const` globals usually go in Flash, not RAM. `#define` for register addresses and masks. `enum` for states and error codes.

---

## 14. Type Conversion

### Implicit (automatic)

- Smaller type promoted to larger.
- `char`/`short` → `int` (**integer promotion**).
- Mixed int + float → float.
- Mixed signed + unsigned → signed becomes unsigned. **Dangerous.**

```c
#include <stdio.h>

int main(void) {
    int i = 5;
    float f = 2.0f;
    printf("%.2f\n", i / f);       // 2.50

    printf("%d\n", 5 / 2);         // 2 (int division)
    printf("%.2f\n", 5 / 2.0);     // 2.50
    return 0;
}
```

### Explicit (cast)

- You force the type.
- Syntax: `(type) value`.

```c
#include <stdio.h>

int main(void) {
    int a = 7, b = 2;
    printf("%d\n", a / b);              // 3
    printf("%.2f\n", (float)a / b);     // 3.50
    //printf("%.2f\n", (float)a / b);     // 3.50

    float pi = 3.99f;
    int trunc = (int)pi;                // drops decimal
    printf("%d\n", trunc);              // 3
    return 0;
}
```

### Signed/unsigned trap

```c
#include <stdio.h>

int main(void) {
    int a = -1;
    unsigned int b = 1;

    if (a < b)
        printf("a < b\n");
    else
        printf("a >= b  (surprise!)\n");   // this runs
    return 0;
}
```

- `-1` converts to `4294967295`.
- Avoid mixing signed and unsigned.

---

## 15. Overflow

- Unsigned → wraps. Defined.
- Signed → undefined behavior.
- Float → becomes `inf`.

```c
#include <stdio.h>
#include <stdint.h>
#include <limits.h>

int main(void) {
    uint8_t u = 250;
    u += 10;
    printf("%u\n", u);            // 4

    unsigned int x = UINT_MAX;
    x++;
    printf("%u\n", x);            // 0
    return 0;
}
```

---

## 16. `sizeof`

- Operator. Not a function.
- Returns `size_t`.
- Evaluated at compile time.
- Works on types and variables.

```c
#include <stdio.h>

int main(void) {
    int arr[10];
    printf("%zu\n", sizeof(int));
    printf("%zu\n", sizeof arr);                 // 40
    printf("%zu\n", sizeof arr / sizeof arr[0]); // 10 elements
    return 0;
}
```

---

## 17. Scope and Lifetime (quick intro)

|Kind|Where|Lifetime|
|---|---|---|
|Local|inside function/block|until block ends|
|Global|outside functions|whole program|
|Static local|inside function, `static`|whole program, local scope|

```c
#include <stdio.h>

int counter = 0;               // global

void tick(void) {
    static int calls = 0;      // keeps value between calls
    int temp = 0;              // resets each call
    calls++;
    temp++;
    counter++;
    printf("calls=%d temp=%d counter=%d\n", calls, temp, counter);
}

int main(void) {
    tick(); tick(); tick();
    return 0;
}
```

More in later note: _Storage classes_.

---

## 18. `volatile` (intro)

- Tells compiler: value can change anytime.
- Do not optimize reads/writes.
- Use for:
    - hardware registers
    - variables changed in ISR
    - shared memory

```c
volatile uint8_t flag = 0;      // set in ISR, read in main

// while (!flag) { }            // without volatile, compiler may loop forever
```

More in later note: _Qualifiers_.

---

## 19. Embedded Cheat Sheet

- Use `stdint.h` types. Always.
- `uint8_t` for bytes and registers.
- `uint32_t` for 32-bit registers.
- `size_t` for sizes and indexes.
- `bool` from `stdbool.h` for flags.
- Avoid `float` unless FPU exists.
- Avoid mixing signed and unsigned.
- Initialize every variable.
- Use `const` for read-only data.
- Use `volatile` for hardware and ISR shared data.
- Use `f`, `u`, `UL` suffixes on literals.

---

## 20. Common Mistakes

- Using uninitialized variable.
- `5 / 2` expecting `2.5`.
- Comparing floats with `==`.
- Mixing signed and unsigned.
- Wrong `printf` specifier.
- Assuming `int` is 4 bytes.
- Leading zero making octal.
- `char` signedness surprise.
- Signed overflow.

---

## 21. Practice

1. Print `sizeof` of all basic types on your machine.
2. Store `300` in a `uint8_t`. Print it. Explain.
3. Compute `7 / 2` and `7 / 2.0`. Explain both.
4. Print `255` in decimal, hex, octal.
5. Show the signed/unsigned comparison trap. Fix it with a cast.
6. Print `INT_MAX + 1` using `unsigned`. Explain wrap.
7. Create an `enum` for traffic light states. Print each value.
8. Write a function with a `static` counter. Call it 5 times.
9. Show `0.1 + 0.2 == 0.3` fails. Fix with epsilon.
10. Convert `'a'` to `'A'` using arithmetic only.

---

## Quick Revision

- Variable = typed named memory.
- Initialize always.
- Sizes vary. Use `stdint.h`.
- Unsigned wraps. Signed overflow is UB.
- Floats are inexact.
- `const`, `#define`, `enum` for constants.
- Casts are explicit. Promotions are silent.
- Match `printf` specifier to type.

**Next:** [[C-02-Operators]]