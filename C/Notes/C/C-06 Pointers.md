> [!info]
> title: Pointers 
> tags: [C, pointers, memory] 
> chapter: 6

| previous: [[C-05 Arrays & Strings]] | next: [[C-07 Structs, Unions, Enums]] |
| ----------------------------------- | ------------------------------------- |
## 1. What is a Pointer

- Variable that stores an **address**.
- Every variable lives at an address.
- Pointer "points to" that variable.
- Type of pointer says how to read the memory there.

```c
#include <stdio.h>

int main(void) {
    int x = 10;
    int *p = &x;

    printf("x      = %d\n", x);
    printf("&x     = %p\n", (void *)&x);
    printf("p      = %p\n", (void *)p);     // same as &x
    printf("*p     = %d\n", *p);            // 10
    printf("&p     = %p\n", (void *)&p);    // pointer has its own address
    return 0;
}
```

Two key operators:

|Op|Name|Meaning|
|---|---|---|
|`&x`|address-of|address of x|
|`*p`|dereference|value at address p|

- `*` in a declaration → "this is a pointer".
- `*` in an expression → "go to that address".

---

## 2. Declaration and Initialization

```c
#include <stdio.h>

int main(void) {
    int  n = 5;
    int *p = &n;           // pointer to int
    int *q = NULL;         // points to nothing
    double d = 2.5;
    double *pd = &d;

    printf("%d %.1f\n", *p, *pd);
    printf("%d\n", q == NULL);     // 1
    return 0;
}
```

> [!warning] Star belongs to the variable `int *a, b;` → `a` is pointer, `b` is plain `int`. Declare one per line.

```c
#include <stdio.h>

int main(void) {
    int *a = NULL, b = 0;
    printf("%zu %zu\n", sizeof a, sizeof b);   // 8 4 (64-bit)
    return 0;
}
```

---

## 3. Reading and Writing Through a Pointer

```c
#include <stdio.h>

int main(void) {
    int x = 10;
    int *p = &x;

    *p = 20;               // writes x
    printf("%d\n", x);     // 20

    *p += 5;
    printf("%d\n", x);     // 25

    int y = *p * 2;
    printf("%d\n", y);     // 50
    return 0;
}
```

Two pointers, same target:

```c
#include <stdio.h>

int main(void) {
    int x = 1;
    int *a = &x, *b = &x;
    *a = 7;
    printf("%d\n", *b);    // 7
    return 0;
}
```

---

## 4. Pointer Size and Types

- All data pointers have the **same size** on a platform.
    - 32-bit MCU → 4 bytes.
    - 64-bit PC → 8 bytes.
- Type decides:
    - how many bytes `*p` reads.
    - how far `p + 1` moves.

```c
#include <stdio.h>

int main(void) {
    printf("char*   : %zu\n", sizeof(char *));
    printf("int*    : %zu\n", sizeof(int *));
    printf("double* : %zu\n", sizeof(double *));
    printf("void*   : %zu\n", sizeof(void *));
    return 0;
}
```

Same bytes, different view:

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    uint32_t v = 0x11223344;
    uint8_t  *b = (uint8_t *)&v;      // view as bytes
    uint16_t *h = (uint16_t *)&v;     // view as halfwords

    printf("%02X %02X %02X %02X\n", b[0], b[1], b[2], b[3]);   // 44 33 22 11 (LE)
    printf("%04X\n", h[0]);                                    // 3344 (LE)
    return 0;
}
```

- Byte view of any object via `char *` / `uint8_t *` is allowed.
- Other casts → see strict aliasing, section 15.

---

## 5. NULL, Wild, and Dangling Pointers

|Kind|Meaning|
|---|---|
|**NULL**|points to nothing. Safe to test.|
|**Wild**|uninitialized. Random address.|
|**Dangling**|target no longer exists.|

```c
#include <stdio.h>
#include <stddef.h>

int main(void) {
    int *p = NULL;
    if (p == NULL) printf("p is null\n");
    if (!p)        printf("also null\n");

    // *p = 5;       // dereference NULL → crash / UB
    return 0;
}
```

Causes of **dangling**:

- Returning address of a local.
- Using memory after `free`.
- Pointer into a buffer that moved (`realloc`).

```c
// int *bad(void) { int x = 1; return &x; }     // x dies on return
//
// int *p = malloc(4);
// free(p);
// *p = 1;                                        // use after free
```

Rules:

- Initialize every pointer. Use `NULL` if no target yet.
- Check for `NULL` before dereferencing.
- After `free(p)`, set `p = NULL`.

> [!note] Embedded Address `0` can be valid on MCUs (vector table, Flash start). Dereferencing `NULL` may **not** crash. It may read silently. Check pointers yourself.

---

## 6. Pointer Arithmetic

- `p + n` moves by `n` **elements**, not bytes.
- Address changes by `n * sizeof(*p)`.

|Allowed|Result|
|---|---|
|`p + n`, `p - n`|pointer|
|`p++`, `p--`|pointer|
|`p - q` (same array)|`ptrdiff_t` count of elements|
|`p < q`, `p == q`|compare (same array)|
|`p + q`, `p * 2`|**not allowed**|

```c
#include <stdio.h>

int main(void) {
    int a[4] = {10, 20, 30, 40};
    int *p = a;

    printf("%d\n", *p);          // 10
    printf("%d\n", *(p + 2));    // 30
    p++;
    printf("%d\n", *p);          // 20

    int *end = a + 4;            // one past the last: allowed to form
    printf("%td\n", end - a);    // 4 elements

    printf("%td bytes\n", (char *)(p + 1) - (char *)p);   // 4
    return 0;
}
```

### Walking with a pointer

```c
#include <stdio.h>

int main(void) {
    int a[] = {1, 2, 3, 4, 5};
    int *end = a + sizeof a / sizeof a[0];

    for (int *p = a; p < end; p++)
        printf("%d ", *p);
    printf("\n");
    return 0;
}
```

### Rules

- Stay inside the array or **one past** the end.
- Dereferencing one-past-end → UB.
- Comparing pointers from different arrays → UB.
- `void *` arithmetic is a GNU extension. Cast to `char *` first.

---

## 7. `*p++` and Friends

- Postfix `++` binds tighter than `*`.

|Expression|Meaning|
|---|---|
|`*p++`|read `*p`, then move pointer|
|`(*p)++`|increment the **value**|
|`*++p`|move pointer, then read|
|`++*p`|increment the value, then read|

```c
#include <stdio.h>

int main(void) {
    int a[] = {1, 2, 3};
    int *p = a;

    int v = *p++;          // v = 1, p → a[1]
    printf("%d %d\n", v, *p);    // 1 2

    (*p)++;                // a[1] becomes 3
    printf("%d\n", a[1]);        // 3

    v = *++p;              // p → a[2], v = 3
    printf("%d\n", v);
    return 0;
}
```

Common use: string copy.

```c
#include <stdio.h>

void copy(char *d, const char *s) {
    while ((*d++ = *s++) != '\0') { }
}

int main(void) {
    char buf[16];
    copy(buf, "pointer");
    printf("%s\n", buf);
    return 0;
}
```

---

## 8. Pointers and Arrays

- Array name → pointer to first element (in most expressions).
- `a[i]` ≡ `*(a + i)`.
- Full table in [[C-05 Arrays & Strings]].

```c
#include <stdio.h>

int main(void) {
    int a[3] = {5, 6, 7};
    int *p = a;

    printf("%d %d %d\n", a[1], *(a + 1), p[1]);   // 6 6 6
    printf("%zu %zu\n", sizeof a, sizeof p);      // 12 8
    return 0;
}
```

### Pointer to array vs array of pointers

```c
#include <stdio.h>

int main(void) {
    int m[2][3] = {{1, 2, 3}, {4, 5, 6}};

    int (*row)[3] = m;                 // pointer to array of 3 ints
    printf("%d\n", row[1][2]);         // 6
    printf("%td bytes\n", (char *)(row + 1) - (char *)row);   // 12

    int x = 1, y = 2, z = 3;
    int *ptrs[3] = {&x, &y, &z};       // array of 3 pointers
    printf("%d\n", *ptrs[2]);          // 3
    return 0;
}
```

---

## 9. Pointers and Functions

Recap from [[C-04 Functions]]:

- Pass an address → function can change the caller's data.
- Use `const T *` for read-only inputs.
- Always check `NULL` for pointer arguments.

```c
#include <stdio.h>

int div_mod(int a, int b, int *q, int *r) {
    if (b == 0 || q == NULL || r == NULL) return -1;
    *q = a / b;
    *r = a % b;
    return 0;
}

int main(void) {
    int q, r;
    if (div_mod(17, 5, &q, &r) == 0)
        printf("%d %d\n", q, r);      // 3 2
    return 0;
}
```

### Returning a pointer

- Return pointer to: static data, caller's buffer, heap, or a global.
- **Never** to a local.

```c
#include <stdio.h>

const char *state_name(int s) {
    static const char *names[] = {"IDLE", "RUN", "ERROR"};
    if (s < 0 || s > 2) return "UNKNOWN";
    return names[s];
}

int main(void) {
    printf("%s %s\n", state_name(1), state_name(9));   // RUN UNKNOWN
    return 0;
}
```

---

## 10. `const` and Pointers

Read right to left.

|Declaration|Reads as|Change data|Move pointer|
|---|---|---|---|
|`int *p`|pointer to int|yes|yes|
|`const int *p`|pointer to const int|no|yes|
|`int *const p`|const pointer to int|yes|no|
|`const int *const p`|const pointer to const int|no|no|

```c
#include <stdio.h>

int main(void) {
    int a = 1, b = 2;

    const int *p1 = &a;
    // *p1 = 5;          // error
    p1 = &b;             // ok

    int *const p2 = &a;
    *p2 = 5;             // ok
    // p2 = &b;          // error

    printf("%d %d\n", *p1, *p2);   // 2 5
    return 0;
}
```

> [!note] Embedded Registers: `volatile uint32_t *const` (fixed address, changing value). Flash tables: `const` data, pointer to `const`.

---

## 11. Pointer to Pointer

- `int **pp` holds the address of an `int *`.
- Use to:
    - change a caller's pointer,
    - hold arrays of strings (`char **`),
    - dynamic 2D arrays.

```c
#include <stdio.h>

int main(void) {
    int x = 10;
    int *p = &x;
    int **pp = &p;

    printf("%d %d %d\n", x, *p, **pp);   // 10 10 10
    **pp = 99;
    printf("%d\n", x);                    // 99
    return 0;
}
```

Walking `char **` (like `argv`):

```c
#include <stdio.h>

int main(void) {
    const char *words[] = {"alpha", "beta", "gamma", NULL};

    for (const char **w = words; *w != NULL; w++)
        printf("%s\n", *w);
    return 0;
}
```

---

## 12. `void *` — Generic Pointer

- Points to any data type.
- Cannot dereference directly.
- Cannot do arithmetic (standard C).
- Converts to and from other pointer types implicitly in C.
- Used by `malloc`, `memcpy`, `qsort`.

```c
#include <stdio.h>
#include <stddef.h>

void swap_any(void *a, void *b, size_t n) {
    unsigned char *x = a, *y = b;
    while (n--) {
        unsigned char t = *x;
        *x++ = *y;
        *y++ = t;
    }
}

int main(void) {
    int a = 1, b = 2;
    double x = 1.5, y = 9.5;

    swap_any(&a, &b, sizeof a);
    swap_any(&x, &y, sizeof x);
    printf("%d %d %.1f %.1f\n", a, b, x, y);   // 2 1 9.5 1.5
    return 0;
}
```

### `qsort` with a comparator

- Comparator gets `const void *`. Cast inside.

```c
#include <stdio.h>
#include <stdlib.h>

int cmp_int(const void *a, const void *b) {
    int x = *(const int *)a;
    int y = *(const int *)b;
    return (x > y) - (x < y);        // avoids subtraction overflow
}

int main(void) {
    int a[] = {5, 2, 9, 1, 7};
    qsort(a, 5, sizeof a[0], cmp_int);
    for (int i = 0; i < 5; i++) printf("%d ", a[i]);   // 1 2 5 7 9
    printf("\n");
    return 0;
}
```

---

## 13. Function Pointers (recap)

- Detail in [[C-04 Functions]].
- Same idea: pointer holds an address. The address is code.

```c
#include <stdio.h>

int twice(int x) { return 2 * x; }
int neg(int x)   { return -x; }

int main(void) {
    int (*f)(int) = twice;
    printf("%d\n", f(5));         // 10
    f = neg;
    printf("%d\n", (*f)(5));      // -5 (both call forms work)
    return 0;
}
```

---

## 14. Dynamic Memory (intro)

- `malloc(n)` → n bytes from heap. Uninitialized.
- `calloc(count, size)` → zeroed.
- `realloc(p, n)` → resize.
- `free(p)` → release.
- Header: `<stdlib.h>`.

```c
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    int n = 5;
    int *a = malloc(n * sizeof *a);     // sizeof *a stays correct if type changes
    if (a == NULL) return 1;            // always check

    for (int i = 0; i < n; i++) a[i] = i * i;
    for (int i = 0; i < n; i++) printf("%d ", a[i]);
    printf("\n");

    free(a);
    a = NULL;
    return 0;
}
```

### Safe `realloc`

- On failure it returns `NULL` and the old block stays valid.
- Never do `p = realloc(p, ...)`. You lose the block on failure.

```c
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    int *a = calloc(3, sizeof *a);
    if (!a) return 1;

    int *tmp = realloc(a, 6 * sizeof *a);
    if (!tmp) { free(a); return 1; }
    a = tmp;

    for (int i = 3; i < 6; i++) a[i] = i;
    for (int i = 0; i < 6; i++) printf("%d ", a[i]);   // 0 0 0 3 4 5
    printf("\n");

    free(a);
    return 0;
}
```

### Bugs

- **Leak** → no `free`.
- **Double free** → `free` twice.
- **Use after free**.
- **Overflow** → write past the block.
- Wrong size: `malloc(n)` instead of `malloc(n * sizeof *p)`.
- Not checking for `NULL`.

> [!warning] Embedded Many firmware projects ban `malloc`. Reasons: fragmentation, no failure handling, non-deterministic timing. Use static buffers, memory pools, or fixed-size arrays.

---

## 15. Alignment and Strict Aliasing

### Alignment

- Many CPUs need `uint32_t` at an address divisible by 4.
- Cortex-M0 faults on unaligned access.
- Cortex-M3+ may allow it, slower.

### Strict aliasing

- Accessing an object through a pointer of a **different type** is UB.
- Exception: `char` / `unsigned char` / `uint8_t` pointers.

```c
#include <stdio.h>
#include <stdint.h>
#include <string.h>

int main(void) {
    uint8_t buf[8] = {0x78, 0x56, 0x34, 0x12, 0, 0, 0, 0};

    // BAD: misaligned + aliasing
    // uint32_t *bad = (uint32_t *)(buf + 1);
    // printf("%X\n", *bad);

    // GOOD: copy bytes
    uint32_t v;
    memcpy(&v, buf, sizeof v);
    printf("0x%08X\n", v);          // 0x12345678 (little-endian host)
    return 0;
}
```

- `memcpy` is the portable way to reinterpret bytes.
- Compilers optimize it away.

---

## 16. Embedded: Memory-Mapped Registers

- Peripherals live at fixed addresses.
- Access them with pointers.
- Always `volatile`. Compiler must not skip or reorder accesses.

Raw form (hardware only):

```c
// #define GPIOA_ODR   (*(volatile uint32_t *)0x40020014UL)
//
// GPIOA_ODR |=  (1U << 5);     // set pin 5
// GPIOA_ODR &= ~(1U << 5);     // clear pin 5
```

Reading the macro:

1. `0x40020014UL` → number.
2. `(volatile uint32_t *)` → treat as pointer to volatile 32-bit.
3. `*( ... )` → the register itself.

### Register block as a struct

- Struct fields match register offsets.
- One base pointer. Access with `->`.
- This is what CMSIS and vendor headers do.

```c
#include <stdio.h>
#include <stdint.h>

typedef struct {
    volatile uint32_t MODER;    // offset 0x00
    volatile uint32_t IDR;      // offset 0x04
    volatile uint32_t ODR;      // offset 0x08
} GPIO_TypeDef;

// real hardware:  #define GPIOA ((GPIO_TypeDef *)0x40020000UL)
static GPIO_TypeDef fake_gpio;                  // simulation in RAM
#define GPIOA (&fake_gpio)

int main(void) {
    GPIOA->MODER |= (1U << 10);     // pin 5 as output
    GPIOA->ODR   |= (1U << 5);      // pin 5 high

    printf("MODER = 0x%08X\n", GPIOA->MODER);
    printf("ODR   = 0x%08X\n", GPIOA->ODR);
    return 0;
}
```

### Why `volatile`

```c
// uint32_t *status = (uint32_t *)0x40011000UL;     // no volatile
// while ((*status & 1U) == 0) { }                  // compiler may read once → infinite loop
//
// volatile uint32_t *status = (volatile uint32_t *)0x40011000UL;
// while ((*status & 1U) == 0) { }                  // reads every pass
```

### `volatile` placement

|Declaration|Meaning|
|---|---|
|`volatile uint32_t *p`|pointer to volatile data (register)|
|`uint32_t *volatile p`|volatile pointer (rare)|
|`volatile uint32_t *const p`|fixed address, volatile data|
|`const volatile uint32_t *p`|read-only register (status)|

### Other embedded pointer uses

- `uintptr_t` → integer that can hold an address.
- Pointer into DMA buffers. Keep buffers aligned.
- Vector table → array of function pointers.
- Pointer into Flash tables → `const` data.
- Linker symbols → addresses of sections (`_estack`, `__bss_start__`).

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    int x = 0;
    uintptr_t addr = (uintptr_t)&x;
    printf("aligned to 4: %d\n", (addr % 4) == 0);   // 1
    return 0;
}
```

---

## 17. Linked List (Interview Favourite)

- Node holds data + pointer to next node.
- Last node's `next` is `NULL`.
- Here nodes are on the stack. Real code may use a pool.

```c
#include <stdio.h>

typedef struct Node {
    int val;
    struct Node *next;
} Node;

void print_list(const Node *n) {
    for (; n != NULL; n = n->next)
        printf("%d ", n->val);
    printf("\n");
}

Node *reverse(Node *head) {
    Node *prev = NULL;
    while (head != NULL) {
        Node *nx = head->next;    // save
        head->next = prev;        // flip
        prev = head;              // advance prev
        head = nx;                // advance head
    }
    return prev;
}

int main(void) {
    Node c = {3, NULL};
    Node b = {2, &c};
    Node a = {1, &b};

    print_list(&a);               // 1 2 3
    Node *h = reverse(&a);
    print_list(h);                // 3 2 1
    return 0;
}
```

- `->` is `(*p).member`.
- Full struct coverage in the next chapter.

---

## 18. Reading Complex Declarations

Rule: start at the name. Go right. Then left. Parentheses first.

|Declaration|Meaning|
|---|---|
|`int *p`|pointer to int|
|`int *p[3]`|array of 3 pointers to int|
|`int (*p)[3]`|pointer to array of 3 ints|
|`int *f(void)`|function returning pointer to int|
|`int (*f)(void)`|pointer to function returning int|
|`int (*f[4])(int)`|array of 4 pointers to functions (int → int)|
|`const int *p`|pointer to const int|
|`int *const p`|const pointer to int|
|`char **p`|pointer to pointer to char|

`typedef` helps:

```c
#include <stdio.h>

typedef void (*Handler)(void);

void a(void) { printf("a\n"); }
void b(void) { printf("b\n"); }

int main(void) {
    Handler table[2] = {a, b};
    table[1]();
    return 0;
}
```

---

## 19. Common Mistakes

- Uninitialized pointer.
- Dereferencing `NULL`.
- Returning address of a local.
- Use after `free`. Double `free`.
- `malloc` without `NULL` check.
- Leak: forgetting `free`.
- `int *a, b;` expecting two pointers.
- `sizeof p` expecting array size.
- Pointer arithmetic outside the array.
- Comparing pointers from different arrays.
- Assigning `int *` to `char *` without a cast, then misreading data.
- Dereferencing `void *`.
- Misaligned access via cast pointer.
- Missing `volatile` on registers / ISR data.
- `p = realloc(p, n)` losing the old block.
- Confusing `*p++` and `(*p)++`.
- Modifying through a `const` pointer by casting it away.
- Passing a pointer to a local that goes out of scope (stored by callee).

---

## 20. Embedded Cheat Sheet

- Initialize all pointers. `NULL` if unused.
- Check pointer args for `NULL`.
- `const T *` for inputs. Output via `T *`.
- Registers → `volatile` + fixed address.
- Register block → struct overlay + base pointer.
- Never return the address of a local.
- Avoid `malloc`. Use static pools.
- Use `uintptr_t` for address math.
- Use `memcpy` for reinterpreting bytes.
- Watch alignment on Cortex-M0.
- `uint8_t *` is the safe byte view.
- Function pointers for callbacks and vector tables.
- Keep pointer depth low. Avoid `***`.
- Set pointer to `NULL` after `free`.

---

## 21. Practice

1. Print a variable's value, address, and the pointer's own address.
2. Swap two ints with pointers.
3. Write `min_max(const int *, int n, int *min, int *max)`.
4. Sum an array using only pointer walking. No `[]`.
5. Reverse an array using two pointers.
6. Write `strlen`, `strcpy`, `strcmp` with pointers.
7. Print the bytes of a `float`. Explain.
8. Show little-endian by reading a `uint32_t` as `uint8_t *`.
9. Use `p++`, `*p++`, `(*p)++` on an array. Predict, then run.
10. Print the byte distance of `row + 1` for `int m[3][4]`.
11. Write a function that changes a caller's pointer using `**`.
12. Use `void *` to write `swap_any`.
13. Sort an array of strings with `qsort`.
14. Allocate an array with `malloc`. Fill. Free. Set to `NULL`.
15. Grow it with a safe `realloc`.
16. Create a dangling pointer on purpose. Run with `-fsanitize=address`.
17. Make a leak. Detect it with `valgrind`.
18. Build a linked list of 5 nodes. Print. Reverse. Print.
19. Find the middle node of a list with slow/fast pointers.
20. Detect a cycle in a list (Floyd).
21. Simulate a GPIO register block in RAM with a struct.
22. Write macros `REG_SET(reg, bit)` and `REG_CLR(reg, bit)` for `volatile uint32_t`.
23. Use `memcpy` to read a `uint32_t` from an unaligned byte buffer.
24. Write a dispatch table of 4 function pointers. Bounds-check the index.
25. Read these aloud: `int *(*f)(int)`, `char (*p)[8]`, `void (*t[3])(void)`.

---

## Quick Revision

- Pointer = variable holding an address.
- `&` gets the address. `*` follows it.
- `p + n` moves by `n * sizeof(*p)` bytes.
- `a[i]` ≡ `*(a + i)`.
- `*p++` → read, then move. `(*p)++` → bump the value.
- `NULL` = nothing. Check before use. Initialize always.
- Never return the address of a local. Never use after `free`.
- `const` placement: left of `*` → data. Right of `*` → pointer.
- `void *` → generic. Cast before use.
- `malloc` → check, use, `free`, set `NULL`. Avoid in firmware.
- Reinterpret bytes with `memcpy`. Watch alignment.
- Registers: `volatile` + fixed address; struct overlay with `->`.
- Read declarations from the name, right then left.

| previous: [[C-05 Arrays & Strings]] | next: [[C-07 Structs, Unions, Enums]] |
| ----------------------------------- | ------------------------------------- |

