> [!info] 
> title: Functions 
> tags: [C, basics, functions] 
> chapter: 4

| previous: [[C-03 Control Flow]] | next: [[C-05 Arrays & Strings]] |
| ------------------------------- | ------------------------------- |
## 1. What is a Function

- Named block of code.
- Takes input. Does work. Gives output.
- Write once. Call many times.

Why use:

- Reuse.
- Smaller, readable code.
- Easy to test.
- Hides details.

```c
#include <stdio.h>

int add(int a, int b) {
    return a + b;
}

int main(void) {
    printf("%d\n", add(3, 4));    // 7
    return 0;
}
```

---

## 2. Anatomy

```c
// return_type name(parameter_list) {
//     body
// }
```

|Part|Example|
|---|---|
|Return type|`int`|
|Name|`add`|
|Parameters|`int a, int b`|
|Body|`{ return a + b; }`|

- **Parameter** → variable in the definition.
- **Argument** → value passed in the call.

```c
#include <stdio.h>

int square(int n) {           // n is a parameter
    return n * n;
}

int main(void) {
    printf("%d\n", square(5));  // 5 is an argument
    return 0;
}
```

---

## 3. Declaration, Prototype, Definition

- **Declaration / prototype** → name, return type, parameter types. No body.
- **Definition** → has the body.
- Compiler must see the prototype **before** the call.

```c
#include <stdio.h>

int max(int a, int b);          // prototype

int main(void) {
    printf("%d\n", max(8, 3));  // call
    return 0;
}

int max(int a, int b) {         // definition
    return (a > b) ? a : b;
}
```

- Parameter names optional in prototype: `int max(int, int);`
- Without a prototype, old C assumed `int`. Removed in C99. Modern compilers warn or error.
- Prototype mismatch with definition → bugs or link errors.

---

## 4. `void`

- `void` return → returns nothing.
- `void` parameter list → takes nothing.

```c
#include <stdio.h>

void greet(void) {
    printf("hello\n");
}

int main(void) {
    greet();
    return 0;
}
```

> [!warning] `f()` vs `f(void)` `void f()` → unspecified arguments (before C23). `void f(void)` → no arguments. Always write `(void)`.

---

## 5. `return`

- Ends function immediately.
- Sends value back.
- Value converts to return type.
- Non-void function must return on every path.

```c
#include <stdio.h>

int sign(int n) {
    if (n > 0) return 1;
    if (n < 0) return -1;
    return 0;
}

int main(void) {
    printf("%d %d %d\n", sign(9), sign(-4), sign(0));
    return 0;
}
```

> [!warning] Missing `return` in a non-void function → using the result is UB. Turn on `-Wall -Wextra`.

---

## 6. Pass by Value

- C always copies arguments.
- Function works on a **copy**.
- Caller's variable unchanged.

```c
#include <stdio.h>

void bump(int x) {
    x = x + 10;
    printf("inside: %d\n", x);   // 15
}

int main(void) {
    int a = 5;
    bump(a);
    printf("outside: %d\n", a);  // 5
    return 0;
}
```

---

## 7. Pass by Pointer

- Pass the **address**.
- Function changes the original through `*p`.
- This is how C does "pass by reference".

```c
#include <stdio.h>

void bump(int *p) {
    *p = *p + 10;
}

void swap(int *a, int *b) {
    int t = *a;
    *a = *b;
    *b = t;
}

int main(void) {
    int a = 5, b = 9;
    bump(&a);
    printf("a = %d\n", a);            // 15

    swap(&a, &b);
    printf("a=%d b=%d\n", a, b);      // 9 15
    return 0;
}
```

### Pointer itself is also a copy

- To change the caller's **pointer**, pass pointer to pointer.

```c
#include <stdio.h>

static int g1 = 100, g2 = 200;

void point_to_g2(int **pp) {
    *pp = &g2;
}

int main(void) {
    int *p = &g1;
    printf("%d\n", *p);       // 100
    point_to_g2(&p);
    printf("%d\n", *p);       // 200
    return 0;
}
```

---

## 8. Returning Multiple Values

C returns one value. Options:

**1. Output parameters**

```c
#include <stdio.h>

void min_max(const int *a, int n, int *min, int *max) {
    *min = *max = a[0];
    for (int i = 1; i < n; i++) {
        if (a[i] < *min) *min = a[i];
        if (a[i] > *max) *max = a[i];
    }
}

int main(void) {
    int data[] = {4, 9, -2, 7};
    int lo, hi;
    min_max(data, 4, &lo, &hi);
    printf("%d %d\n", lo, hi);       // -2 9
    return 0;
}
```

**2. Return a struct**

```c
#include <stdio.h>

typedef struct { int quot; int rem; } DivResult;

DivResult divide(int a, int b) {
    DivResult r = { a / b, a % b };
    return r;
}

int main(void) {
    DivResult r = divide(17, 5);
    printf("%d %d\n", r.quot, r.rem);   // 3 2
    return 0;
}
```

**3. Status code + output parameter** (most common in embedded)

```c
#include <stdio.h>

typedef enum { OK = 0, ERR_NULL = -1, ERR_ZERO = -2 } Status;

Status safe_div(int a, int b, int *out) {
    if (out == NULL) return ERR_NULL;
    if (b == 0)      return ERR_ZERO;
    *out = a / b;
    return OK;
}

int main(void) {
    int r;
    if (safe_div(10, 2, &r) == OK)
        printf("r = %d\n", r);
    printf("%d\n", safe_div(1, 0, &r));    // -2
    return 0;
}
```

---

## 9. Arrays as Parameters

- Array decays to a pointer to first element.
- Function does **not** know the size.
- Always pass the length.
- `sizeof` inside gives pointer size.

```c
#include <stdio.h>

void show(int a[], int n) {            // same as int *a
    printf("sizeof in func: %zu\n", sizeof a);   // pointer size
    for (int i = 0; i < n; i++) printf("%d ", a[i]);
    printf("\n");
}

int main(void) {
    int arr[5] = {1, 2, 3, 4, 5};
    printf("sizeof in main: %zu\n", sizeof arr);  // 20
    show(arr, 5);
    return 0;
}
```

- Function can modify the caller's array.
- Use `const` if it should only read.

```c
#include <stdio.h>
#include <stddef.h>

int sum(const int *a, size_t n) {
    int s = 0;
    for (size_t i = 0; i < n; i++) s += a[i];
    // a[0] = 0;      // error: read-only
    return s;
}

int main(void) {
    int v[] = {1, 2, 3, 4};
    printf("%d\n", sum(v, 4));    // 10
    return 0;
}
```

Details in the Arrays and Pointers chapters.

---

## 10. `const` Parameters

|Declaration|Meaning|
|---|---|
|`const int *p`|data read-only, pointer can move|
|`int *const p`|pointer fixed, data changeable|
|`const int *const p`|both fixed|

- Use `const T *` for input pointers.
- Documents intent. Compiler enforces.

```c
#include <stdio.h>
#include <stddef.h>

size_t my_strlen(const char *s) {
    size_t n = 0;
    while (s[n] != '\0') n++;
    return n;
}

int main(void) {
    printf("%zu\n", my_strlen("embedded"));   // 8
    return 0;
}
```

---

## 11. Scope and Lifetime

- Parameters and locals live on the **stack**.
- Created at call. Destroyed at return.
- Each call gets its own copy.
- Locals are not initialized.

### Never return address of a local

```c
// int *bad(void) {
//     int x = 10;
//     return &x;        // x is gone after return. Dangling pointer.
// }
```

Fixes:

```c
#include <stdio.h>

// 1. caller supplies storage
void fill(int *out) { *out = 10; }

// 2. static storage (one shared copy)
int *get_static(void) {
    static int x = 10;
    return &x;
}

int main(void) {
    int v;
    fill(&v);
    printf("%d %d\n", v, *get_static());
    return 0;
}
```

- Third option: `malloc`. Caller must `free`. Avoid in many embedded systems.

---

## 12. `static` Local Variable

- Keeps value between calls.
- Initialized once.
- Stored in data/bss, not on stack.
- Scope still local.

```c
#include <stdio.h>

int next_id(void) {
    static int id = 0;
    return ++id;
}

int main(void) {
    printf("%d\n", next_id());   // 1
    printf("%d\n", next_id());   // 2
    printf("%d\n", next_id());   // 3
    return 0;
}
```

> [!warning] Static locals make a function non-reentrant. Not safe from multiple tasks or ISR + main without protection.

---

## 13. `static` and `extern` Functions

- Default: function has **external linkage**. Visible in other `.c` files.
- `static` function → visible only in its file.
- Use `static` for helpers. Avoids name clashes.

```c
#include <stdio.h>

static int helper(int x) {       // private to this file
    return x * 2;
}

int public_api(int x) {          // visible to other files
    return helper(x) + 1;
}

int main(void) {
    printf("%d\n", public_api(5));    // 11
    return 0;
}
```

---

## 14. Headers and Multiple Files

> [!note] Multi-file. Not for Code Runner.

`math_utils.h`

```text
#ifndef MATH_UTILS_H
#define MATH_UTILS_H

int add(int a, int b);
int mul(int a, int b);

#endif
```

`math_utils.c`

```text
#include "math_utils.h"

int add(int a, int b) { return a + b; }
int mul(int a, int b) { return a * b; }
```

`main.c`

```text
#include <stdio.h>
#include "math_utils.h"

int main(void) {
    printf("%d\n", add(2, 3));
    return 0;
}
```

Build:

```text
gcc main.c math_utils.c -o app
```

- `.h` → declarations. `.c` → definitions.
- Include guard stops double inclusion.
- `<...>` → system headers. `"..."` → your headers.
- Include the header in its own `.c` file. Compiler checks match.
- Do not define functions or variables in headers.

---

## 15. Recursion

- Function calls itself.
- Needs a **base case**.
- Each call gets a new stack frame.

```c
#include <stdio.h>

unsigned long fact(unsigned n) {
    if (n <= 1) return 1;          // base case
    return n * fact(n - 1);        // recursive case
}

int main(void) {
    printf("%lu\n", fact(5));      // 120
    return 0;
}
```

More:

```c
#include <stdio.h>

int sum_digits(int n) {
    if (n == 0) return 0;
    return (n % 10) + sum_digits(n / 10);
}

int fib(int n) {
    if (n < 2) return n;
    return fib(n - 1) + fib(n - 2);    // slow: repeated work
}

int main(void) {
    printf("%d\n", sum_digits(1234));  // 10
    printf("%d\n", fib(10));           // 55
    return 0;
}
```

Loop version of `fib`:

```c
#include <stdio.h>

int fib_iter(int n) {
    int a = 0, b = 1;
    for (int i = 0; i < n; i++) {
        int t = a + b;
        a = b;
        b = t;
    }
    return a;
}

int main(void) {
    printf("%d\n", fib_iter(10));    // 55
    return 0;
}
```

> [!warning] Embedded Stack is small (often 1–8 KB). Deep recursion → stack overflow → hard fault or silent corruption. Depth must be known and bounded. MISRA C forbids recursion. Prefer loops.

---

## 16. Call Stack

- Each call pushes a **stack frame**.
- Frame holds: return address, parameters, locals, saved registers.
- Return pops the frame.
- Stack usually grows toward lower addresses (ARM, x86).

```c
#include <stdio.h>

void c(void) { int z; printf("c: %p\n", (void *)&z); }
void b(void) { int y; printf("b: %p\n", (void *)&y); c(); }
void a(void) { int x; printf("a: %p\n", (void *)&x); b(); }

int main(void) {
    a();       // addresses usually decrease
    return 0;
}
```

Embedded stack rules:

- Avoid big local arrays. Use `static` or global buffers.
- Pass big structs by pointer.
- Know your worst-case stack depth.
- Fill stack with a pattern. Check high-water mark.

---

## 17. Function Pointers

- Variable that holds a function's address.
- Used for callbacks, tables, drivers.

Syntax:

```c
// return_type (*name)(parameter_types)
```

```c
#include <stdio.h>

int add(int a, int b) { return a + b; }
int sub(int a, int b) { return a - b; }

int main(void) {
    int (*op)(int, int) = add;
    printf("%d\n", op(6, 2));     // 8

    op = sub;
    printf("%d\n", op(6, 2));     // 4
    return 0;
}
```

### `typedef` makes it readable

```c
#include <stdio.h>

typedef int (*BinOp)(int, int);

int add(int a, int b) { return a + b; }
int mul(int a, int b) { return a * b; }

int apply(BinOp f, int x, int y) {
    return f(x, y);
}

int main(void) {
    printf("%d\n", apply(add, 3, 4));   // 7
    printf("%d\n", apply(mul, 3, 4));   // 12
    return 0;
}
```

### Callback

```c
#include <stdio.h>

typedef void (*Handler)(int);

void on_event(int code) { printf("event %d\n", code); }

void trigger(Handler cb, int code) {
    if (cb != NULL) cb(code);       // check before calling
}

int main(void) {
    trigger(on_event, 42);
    return 0;
}
```

### Dispatch table

- Array of function pointers.
- Replaces long `switch`. Common for command parsers and vector tables.

```c
#include <stdio.h>

void cmd_start(void) { printf("start\n"); }
void cmd_stop(void)  { printf("stop\n"); }
void cmd_reset(void) { printf("reset\n"); }

typedef void (*Cmd)(void);

int main(void) {
    Cmd table[] = { cmd_start, cmd_stop, cmd_reset };
    int n = sizeof table / sizeof table[0];

    for (int i = 0; i < n; i++)
        table[i]();
    return 0;
}
```

> [!note] Embedded The interrupt vector table is an array of function pointers. Always check for `NULL` before calling a callback.

---

## 18. `inline`

- Hint: paste the body at call site.
- Saves call overhead. Costs code size.
- Compiler may ignore it.
- In a header use `static inline`.

```c
#include <stdio.h>

static inline int square(int x) {
    return x * x;
}

int main(void) {
    printf("%d\n", square(7));    // 49
    return 0;
}
```

### Function vs macro

```c
#include <stdio.h>

#define SQUARE_BAD(x) x * x
#define SQUARE(x)     ((x) * (x))

static inline int square(int x) { return x * x; }

int main(void) {
    printf("%d\n", SQUARE_BAD(2 + 3));   // 2+3*2+3 = 11
    printf("%d\n", SQUARE(2 + 3));       // 25

    int i = 3;
    printf("%d\n", SQUARE(i++));         // UB: i++ twice
    i = 3;
    printf("%d\n", square(i++));         // 9, safe
    return 0;
}
```

- Macros: no type check. Arguments may evaluate twice.
- Prefer `static inline` functions.

---

## 19. Variadic Functions

- Variable number of arguments.
- `printf` is one.
- Needs `<stdarg.h>`.
- No type safety. You trust the caller.

```c
#include <stdio.h>
#include <stdarg.h>

int sum(int count, ...) {
    va_list ap;
    va_start(ap, count);

    int total = 0;
    for (int i = 0; i < count; i++)
        total += va_arg(ap, int);

    va_end(ap);
    return total;
}

int main(void) {
    printf("%d\n", sum(3, 10, 20, 30));   // 60
    printf("%d\n", sum(2, 5, 6));         // 11
    return 0;
}
```

- Small types promote: `char`, `short` → `int`, `float` → `double`.
- Wrong type in `va_arg` → UB.
- Rare in embedded. Often a custom `log()` wrapper.

---

## 20. `main`

Two valid forms:

```c
// int main(void)
// int main(int argc, char *argv[])
```

- `argc` → argument count.
- `argv` → argument strings.
- Return `0` → success.
- Embedded: `main` usually never returns.

```c
#include <stdio.h>

int main(int argc, char *argv[]) {
    printf("argc = %d\n", argc);
    for (int i = 0; i < argc; i++)
        printf("argv[%d] = %s\n", i, argv[i]);
    return 0;
}
```

---

## 21. Argument Evaluation Order

- Order of evaluating arguments is **unspecified**.
- Do not use side effects across arguments.

```c
// printf("%d %d\n", i++, i++);     UB
// f(get_a(), get_b());              order unknown
```

---

## 22. Embedded Notes

### ISR (interrupt service routine)

- No arguments. No return value.
- Keep short. Set a flag. Leave work to main loop.
- Do not call `printf` or `malloc`.
- Name must match the vector table (vendor startup file).

```c
// volatile uint8_t rx_flag = 0;
//
// void USART1_IRQHandler(void) {      // name from startup file
//     rx_flag = 1;
// }
```

### Reentrancy

- Reentrant → safe if interrupted and called again.
- Uses only its parameters and local variables.
- No static locals. No unprotected globals.

```c
#include <stdio.h>

static int counter = 0;

int not_reentrant(void) {
    counter++;                 // shared state
    return counter;
}

int reentrant(int c) {
    return c + 1;              // no shared state
}

int main(void) {
    printf("%d %d\n", not_reentrant(), reentrant(5));
    return 0;
}
```

### Other

- Keep functions small. One job each.
- Few parameters. Group with a struct if many.
- Use fixed-width types in signatures.
- Return status codes. Check them.
- Mark private functions `static`.
- Place critical routines in RAM with a section attribute (later chapter).

---

## 23. Common Mistakes

- Calling before declaring.
- Prototype and definition mismatch.
- Missing `return`.
- `f()` instead of `f(void)`.
- Expecting pass by value to change the caller.
- Returning address of a local.
- Using `sizeof` on an array parameter.
- Not passing array length.
- Forgetting to check `NULL` pointer.
- Recursion without a base case.
- Deep recursion on a small stack.
- Macro args evaluated twice.
- Calling a `NULL` function pointer.
- Using static locals in reentrant code.
- Defining functions in headers without `static inline`.

---

## 24. Embedded Cheat Sheet

- Always write prototypes. Put them in headers.
- `(void)` for no parameters.
- `const T *` for read-only inputs.
- Pass length with arrays.
- Pass big structs by pointer.
- Return a status. Output via pointer.
- Check pointers for `NULL`.
- `static` for file-private helpers.
- `static inline` over function-like macros.
- Avoid recursion.
- Avoid big stack locals.
- Keep ISRs tiny. Flag shared with `volatile`.
- Function pointer tables for dispatch.
- Check callback not `NULL` before calling.

---

## 25. Practice

1. Write `max3(a, b, c)`.
2. Write `is_prime(n)` returning `1` or `0`.
3. Write `swap(int *, int *)`. Test it.
4. Write `min_max` using output pointers.
5. Write a function that returns a struct with quotient and remainder.
6. Write `sum_array(const int *, size_t)`.
7. Show that `sizeof` on an array parameter gives the pointer size.
8. Write `next_id()` with a `static` local.
9. Write `power(base, exp)` with a loop. Then recursively.
10. Write recursive `gcd(a, b)`.
11. Write `reverse_string(char *)` in place.
12. Write `count_set_bits(uint32_t)`.
13. Write a `calc(BinOp, x, y)` function. Pass `add`, `sub`, `mul`.
14. Build a command table of 3 function pointers. Call by index with bounds check.
15. Write a variadic `average(int count, ...)`.
16. Compare `SQUARE(i++)` macro vs `static inline` version.
17. Write `Status read_sensor(int *out)` returning an enum.
18. Split code into `util.h`, `util.c`, `main.c`. Build with gcc.
19. Write a function that tries to return a local pointer. Fix it three ways.
20. Print addresses of locals in 3 nested calls. Observe direction.

---

## Quick Revision

- Prototype first. Definition has the body.
- Parameters are copies. Use pointers to change the caller's data.
- Arrays decay to pointers. Pass the length.
- `f(void)` means no args.
- Never return the address of a local.
- `static` local keeps value. `static` function is file-private.
- Recursion needs a base case. Avoid in embedded.
- Function pointers → callbacks, tables, vectors.
- Prefer `static inline` over macros.
- ISR: no args, no return, short, flag via `volatile`.
- Reentrant = no shared state.

| previous: [[C-04 Functions]] | next: [[C-05 Arrays & Strings]] |
| ---------------------------- | ------------------------------- |

