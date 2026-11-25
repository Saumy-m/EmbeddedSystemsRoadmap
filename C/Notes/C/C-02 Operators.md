> [!info] ---
> title : Operators
> tags: [C, basics, operators]
> Chapter 2

| previous: [[C-01 Variables & Data Types]] | next: [[C-03 Control Flow]] |
| ----------------------------------------- | --------------------------- |
## 1. What is an Operator

- Symbol that works on values.
- Values are called **operands**.
- Unary → 1 operand. Binary → 2. Ternary → 3.

Groups:

- Arithmetic
- Relational
- Logical
- Bitwise
- Assignment
- Increment / Decrement
- Conditional (`?:`)
- Others (`sizeof`, `,`, `&`, `*`, cast)

---

## 2. Arithmetic Operators

|Op|Meaning|
|---|---|
|`+`|add|
|`-`|subtract|
|`*`|multiply|
|`/`|divide|
|`%`|remainder (ints only)|

```c
#include <stdio.h>

int main(void) {
    int a = 17, b = 5;
    printf("%d\n", a + b);   // 22
    printf("%d\n", a - b);   // 12
    printf("%d\n", a * b);   // 85
    printf("%d\n", a / b);   // 3
    printf("%d\n", a % b);   // 2
    return 0;
}
```

### Integer division

- int / int → int. Decimal dropped.
- Truncates toward zero (C99+).
- Make one operand float to get decimals.

```c
#include <stdio.h>

int main(void) {
    printf("%d\n", 7 / 2);        // 3
    printf("%.2f\n", 7 / 2.0);    // 3.50
    printf("%d\n", -7 / 2);       // -3
    printf("%d\n", -7 % 2);       // -1
    printf("%d\n", 7 % -2);       // 1
    return 0;
}
```

- Sign of `%` result follows the **left** operand.
- `a == (a / b) * b + (a % b)` always.

> [!warning] Divide by zero → undefined behavior (ints). `%` does not work on floats. Use `fmod()`. `INT_MIN / -1` overflows. UB.

---

## 3. Relational Operators

|Op|Meaning|
|---|---|
|`==`|equal|
|`!=`|not equal|
|`<` `>`|less / greater|
|`<=` `>=`|less or equal / greater or equal|

- Result is `int`: `1` true, `0` false.

```c
#include <stdio.h>

int main(void) {
    int a = 5, b = 10;
    printf("%d\n", a == b);   // 0
    printf("%d\n", a != b);   // 1
    printf("%d\n", a < b);    // 1
    printf("%d\n", a >= b);   // 0
    return 0;
}
```

> [!warning] `=` vs `==` `=` assigns. `==` compares. `if (x = 5)` assigns 5, always true. Tip: write `if (5 == x)`. Typo becomes compile error.

> [!warning] Chained comparison `1 < x < 10` does not work like math. It is `(1 < x) < 10` → always true. Write `1 < x && x < 10`.

---

## 4. Logical Operators

| Op   | Meaning |
| ---- | ------- |
| `&&` | AND     |
| \|\| | OR      |
| `!`  | NOT     |

- Any nonzero is true.
- Result is `0` or `1`.

```c
#include <stdio.h>

int main(void) {
    int a = 1, b = 0;
    printf("%d\n", a && b);   // 0
    printf("%d\n", a || b);   // 1
    printf("%d\n", !a);       // 0
    printf("%d\n", !5);       // 0
    printf("%d\n", !!5);      // 1  (normalize to 0/1)
    return 0;
}
```

### Short-circuit

- `&&` → stops if left is false.
- `||` → stops if left is true.
- Right side may never run.

```c
#include <stdio.h>

int check(const char *msg) {
    printf("%s\n", msg);
    return 1;
}

int main(void) {
    if (0 && check("A")) { }      // A not printed
    if (1 || check("B")) { }      // B not printed
    if (1 && check("C")) { }      // C printed
    return 0;
}
```

Useful pattern: guard before use.

```c
// if (ptr != NULL && ptr->value > 0)
```

---

## 5. Bitwise Operators

|Op|Meaning|
|---|---|
|`&`|AND|
|`\|`|OR|
|`^`|XOR|
|`~`|NOT (invert)|
|`<<`|left shift|
|`>>`|right shift|

Truth table (per bit):

|a|b|`a&b`|`a\|b`|`a^b`|
|---|---|---|---|---|
|0|0|0|0|0|
|0|1|0|1|1|
|1|0|0|1|1|
|1|1|1|1|0|

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    uint8_t a = 0b1100;    // 12
    uint8_t b = 0b1010;    // 10

    printf("a & b  = 0x%02X\n", a & b);   // 0x08
    printf("a | b  = 0x%02X\n", a | b);   // 0x0E
    printf("a ^ b  = 0x%02X\n", a ^ b);   // 0x06
    printf("~a     = 0x%02X\n", (uint8_t)~a);  // 0xF3
    return 0;
}
```

### Shifts

- `x << n` → multiply by 2ⁿ.
- `x >> n` → divide by 2ⁿ.
- Zeros come in on left shift.
- Right shift of **unsigned** → zeros in.
- Right shift of **negative signed** → implementation-defined.

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    uint32_t x = 1;
    printf("%u\n", x << 3);        // 8
    printf("%u\n", 40u >> 2);      // 10
    printf("0x%X\n", 1u << 31);    // 0x80000000
    return 0;
}
```

> [!warning] Shift traps Shift by negative or by ≥ width → UB. `1 << 31` on 32-bit `int` → UB (sign bit). Use `1U << 31`. Always use `U` suffix when shifting into bit 31.

> [!warning] Integer promotion with `~` `~` on `uint8_t` promotes to `int` first. Result is wide. Cast back or mask.

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    uint8_t x = 0x0F;
    printf("0x%X\n", ~x);               // 0xFFFFFFF0
    printf("0x%X\n", (uint8_t)~x);      // 0xF0
    printf("0x%X\n", ~x & 0xFF);        // 0xF0
    return 0;
}
```

---

## 6. Bit Manipulation (Embedded Core)

Five standard moves:

|Task|Code|
|---|---|
|Set bit n|`reg \|= (1U << n)`|
|Clear bit n|`reg &= ~(1U << n)`|
|Toggle bit n|`reg ^= (1U << n)`|
|Check bit n|`reg & (1U << n)`|
|Extract field|`(reg >> pos) & mask`|

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    uint8_t reg = 0x00;

    reg |= (1U << 3);          // set bit 3
    printf("set    : 0x%02X\n", reg);   // 0x08

    reg ^= (1U << 0);          // toggle bit 0
    printf("toggle : 0x%02X\n", reg);   // 0x09

    reg &= ~(1U << 3);         // clear bit 3
    printf("clear  : 0x%02X\n", reg);   // 0x01

    if (reg & (1U << 0))
        printf("bit 0 is set\n");
    return 0;
}
```

### Multi-bit field

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    uint8_t reg = 0xB6;                  // 1011 0110

    uint8_t upper = (reg >> 4) & 0x0F;   // 1011 = 0xB
    printf("upper nibble: 0x%X\n", upper);

    // write field: put 0x5 into bits [3:1]
    reg &= ~(0x7U << 1);                 // clear field
    reg |= (0x5U << 1);                  // set value
    printf("reg: 0x%02X\n", reg);
    return 0;
}
```

Pattern for any field:

1. Clear with mask.
2. Shift value to position.
3. OR it in.

### Handy macros

```c
#include <stdio.h>
#include <stdint.h>

#define BIT(n)          (1U << (n))
#define SET_BIT(r, n)   ((r) |=  BIT(n))
#define CLR_BIT(r, n)   ((r) &= ~BIT(n))
#define TOG_BIT(r, n)   ((r) ^=  BIT(n))
#define IS_SET(r, n)    (((r) >> (n)) & 1U)

int main(void) {
    uint8_t r = 0;
    SET_BIT(r, 5);
    SET_BIT(r, 1);
    printf("0x%02X\n", r);            // 0x22
    CLR_BIT(r, 5);
    printf("0x%02X\n", r);            // 0x02
    printf("%u\n", IS_SET(r, 1));     // 1
    return 0;
}
```

> [!note] Embedded Registers are written like this daily. Use `1U`, not `1`. Avoid signed shifts. Read-modify-write is not atomic. Careful with ISRs.

---

## 7. Bit Tricks

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    // power of two check
    uint32_t n = 64;
    printf("pow2: %d\n", n && !(n & (n - 1)));    // 1

    // swap without temp
    int a = 5, b = 9;
    a ^= b; b ^= a; a ^= b;
    printf("a=%d b=%d\n", a, b);                  // 9 5

    // count set bits
    uint32_t v = 0xB7;
    int count = 0;
    while (v) { v &= (v - 1); count++; }
    printf("bits: %d\n", count);                  // 6

    // even / odd
    printf("odd: %d\n", 7 & 1);                   // 1

    // multiply / divide by 2
    printf("%d %d\n", 5 << 1, 20 >> 2);           // 10 5
    return 0;
}
```

- `n & (n - 1)` clears lowest set bit.
- XOR swap fails if both are the same variable.
- Compilers already optimize `*2` and `/2`. Write clear code.

---

## 8. Assignment Operators

|Op|Same as|
|---|---|
|`=`|assign|
|`+=`|`a = a + b`|
|`-=`|`a = a - b`|
|`*=`|`a = a * b`|
|`/=`|`a = a / b`|
|`%=`|`a = a % b`|
|`&=`|`a = a & b`|
|`\|=`|`a = a \| b`|
|`^=`|`a = a ^ b`|
|`<<=`|`a = a << b`|
|`>>=`|`a = a >> b`|

```c
#include <stdio.h>

int main(void) {
    int x = 10;
    x += 5;    printf("%d\n", x);   // 15
    x -= 3;    printf("%d\n", x);   // 12
    x *= 2;    printf("%d\n", x);   // 24
    x /= 4;    printf("%d\n", x);   // 6
    x %= 4;    printf("%d\n", x);   // 2
    x <<= 3;   printf("%d\n", x);   // 16

    int a, b, c;
    a = b = c = 7;                  // right to left
    printf("%d %d %d\n", a, b, c);
    return 0;
}
```

- Assignment is an **expression**. It returns the value.
- Evaluates right to left.

---

## 9. Increment and Decrement

|Form|Meaning|
|---|---|
|`++x`|increment, then use|
|`x++`|use, then increment|
|`--x`|decrement, then use|
|`x--`|use, then decrement|

```c
#include <stdio.h>

int main(void) {
    int i = 5;
    int a = i++;     // a = 5, i = 6
    printf("%d %d\n", a, i);

    int j = 5;
    int b = ++j;     // b = 6, j = 6
    printf("%d %d\n", b, j);
    return 0;
}
```

> [!warning] Do not modify twice Same variable changed twice in one expression → UB.

```c
// i = i++ + ++i;          UB
// printf("%d %d", i++, i++);   UB
// a[i] = i++;             UB
```

Rule: one side effect per variable per statement.

---

## 10. Conditional (Ternary) Operator

- Syntax: `cond ? value_if_true : value_if_false`
- Only operator with 3 operands.
- Returns a value.

```c
#include <stdio.h>

int main(void) {
    int a = 8, b = 12;
    int max = (a > b) ? a : b;
    printf("max = %d\n", max);

    int n = 7;
    printf("%s\n", (n % 2 == 0) ? "even" : "odd");
    return 0;
}
```

- Good for short choices.
- Avoid nesting. Hard to read.

---

## 11. Comma Operator

- Evaluates left, then right.
- Value is the right one.
- Common in `for` loops.

```c
#include <stdio.h>

int main(void) {
    int a = (1, 2, 3);
    printf("%d\n", a);               // 3

	// for following scene if comma op in condition only second condition evals
    for (int i = 0, j = 10; i < j, i < 2; i++, j--)
        printf("%d %d\n", i, j);
    return 0;
}
```

> [!note] Comma in `f(a, b)` is **not** the comma operator. It is an argument separator.

---

## 12. `sizeof` and Cast

```c
#include <stdio.h>

int main(void) {
    int x = 10;
    printf("%zu\n", sizeof x);            // no parens ok for variables
    printf("%zu\n", sizeof(int));         // parens needed for types
    printf("%.2f\n", (float)x / 4);       // cast: 2.50
    return 0;
}
```

- `sizeof` is compile-time.
- It does **not** evaluate its operand.

```c
#include <stdio.h>

int main(void) {
    int i = 5;
    size_t s = sizeof(i++);
    printf("%d\n", i);        // still 5
    return 0;
}
```

---

## 13. Address and Dereference (preview)

|Op|Meaning|
|---|---|
|`&x`|address of x|
|`*p`|value at address p|
|`p->m`|member via pointer|
|`s.m`|member of struct|
|`a[i]`|element i|

```c
#include <stdio.h>

int main(void) {
    int x = 42;
    int *p = &x;
    *p = 99;
    printf("%d\n", x);     // 99
    return 0;
}
```

Full coverage in the Pointers chapter.

---

## 14. Precedence and Associativity

- **Precedence** → which operator first.
- **Associativity** → direction when same level.

High to low (simplified):

|Level|Operators|Assoc.|
|---|---|---|
|1|`()` `[]` `->` `.` `x++` `x--`|L → R|
|2|`!` `~` `++x` `--x` `+x` `-x` `*p` `&x` `sizeof` `(cast)`|R → L|
|3|`*` `/` `%`|L → R|
|4|`+` `-`|L → R|
|5|`<<` `>>`|L → R|
|6|`<` `<=` `>` `>=`|L → R|
|7|`==` `!=`|L → R|
|8|`&`|L → R|
|9|`^`|L → R|
|10|`\|`|L → R|
|11|`&&`|L → R|
|12|`\|`|L → R|
|13|`?:`|R → L|
|14|`=` `+=` `-=` ...|R → L|
|15|`,`|L → R|

```c
#include <stdio.h>

int main(void) {
    printf("%d\n", 2 + 3 * 4);       // 14
    printf("%d\n", (2 + 3) * 4);     // 20
    printf("%d\n", 10 - 4 - 2);      // 4  (L to R)
    printf("%d\n", 1 + 2 << 1);      // 6  (+ before <<)
    return 0;
}
```

### Classic traps

```c
#include <stdio.h>

int main(void) {
    int flags = 0x04;

    // WRONG: == binds tighter than &
    if (flags & 0x04 == 0x04)
        printf("wrong parse\n");     // parsed as flags & (0x04 == 0x04)

    // RIGHT
    if ((flags & 0x04) == 0x04)
        printf("correct\n");
    return 0;
}
```

- Bitwise ops are **lower** than `==`.
- Always parenthesize bit tests.
- When unsure → add parentheses.

---

## 15. Order of Evaluation

- Precedence ≠ order of evaluation.
- In `f() + g()`, which runs first? **Unspecified.**
- Only `&&`, `||`, `?:`, `,` guarantee order.

```c
// int r = f() + g();       order unknown
// foo(a(), b());           order unknown
```

Do not depend on order when functions have side effects.

---

## 16. Type Rules in Expressions

- `char` / `short` → promoted to `int` first.
- Then **usual arithmetic conversions**:
    - `long double` > `double` > `float`
    - then by rank, unsigned wins on tie.

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    uint8_t a = 200, b = 100;
    int sum = a + b;                 // promoted to int, no overflow
    printf("%d\n", sum);             // 300

    uint8_t c = a + b;               // truncated on store
    printf("%u\n", c);               // 44

    int i = -1;
    unsigned int u = 1;
    printf("%d\n", i < u);           // 0  (i becomes huge)
    return 0;
}
```

---

## 17. Common Mistakes

- `=` instead of `==`.
- `&` instead of `&&` (and `|` vs `||`).
- `a & b == c` without parentheses.
- Integer division when decimals expected.
- Modifying a variable twice in one expression.
- `1 << 31` on signed int.
- Shifting by too many bits.
- Chained comparison `a < b < c`.
- Assuming evaluation order of function calls.
- Forgetting `~` promotes to `int`.
- Using `%` on floats.
- Semicolon after `if (...);`.

---

## 18. Embedded Cheat Sheet

- Use `1U`, `0xFFU` in bit math.
- Parenthesize every bit test.
- Use macros for set / clear / toggle.
- Mask fields: clear → shift → OR.
- Cast `~` result back to register width.
- Avoid signed types in bitwise work.
- Avoid side effects inside `&&` / `||` conditions.
- Read-modify-write is not atomic.
- Prefer clear code. Compiler optimizes shifts.

---

## 19. Practice

1. Print `17 / 5`, `17 % 5`, `-17 / 5`, `-17 % 5`. Explain.
2. Compute average of two ints with decimals.
3. Show short-circuit with a function that prints.
4. Set bits 2 and 6 of a `uint8_t`. Print in hex.
5. Clear bit 6, toggle bit 0. Print.
6. Extract bits [7:4] of `0xA5`.
7. Write a field: set bits [5:3] to `0b101` without touching others.
8. Check if a number is a power of two.
9. Count set bits in a `uint32_t`.
10. Swap two variables with XOR.
11. Fix: `if (x & 0x01 == 0)`.
12. Predict output: `int i = 3; printf("%d", i++ * 2);` then `i`.
13. Use the ternary to find the max of three numbers.
14. Reverse the bits of a `uint8_t`.
15. Print `~(uint8_t)0x0F` correctly.

---

## Quick Revision

- `/` on ints drops decimals. `%` follows left sign.
- `=` assigns. `==` compares.
- `&&` `||` short-circuit.
- `&` `|` `^` `~` work per bit.
- Set `|=`, clear `&= ~`, toggle `^=`, test `&`.
- Use `U` suffix for shifts.
- Bit ops rank below `==`. Use parentheses.
- Never modify a variable twice in one expression.
- Only `&&` `||` `?:` `,` guarantee order.

| previous: [[C-01 Variables & Data Types]] | next: [[C-03 Control Flow]] |
| ----------------------------------------- | --------------------------- |

