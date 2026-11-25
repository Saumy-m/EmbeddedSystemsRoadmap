> [!info]
> title: Arrays and Strings 
> tags: [C, basics, arrays, strings] 
> chapter: 5

| previous: [[C-04 Functions]] | next: [[C-06 Pointers]] |
| ---------------------------- | ----------------------- |
## 1. Array Basics

- Fixed-size collection of the **same type**.
- Elements sit next to each other in memory.
- Index starts at **0**.
- Size fixed at compile time.
- No bounds checking.

```c
#include <stdio.h>

int main(void) {
    int a[5];                 // 5 ints, garbage values
    a[0] = 10;
    a[1] = 20;
    a[2] = 30;
    a[3] = 40;
    a[4] = 50;

    for (int i = 0; i < 5; i++)
        printf("a[%d] = %d\n", i, a[i]);
    return 0;
}
```

---

## 2. Initialization

```c
#include <stdio.h>

int main(void) {
    int a[5] = {1, 2, 3, 4, 5};     // full
    int b[5] = {1, 2};              // rest become 0
    int c[5] = {0};                 // all zero
    int d[]  = {7, 8, 9};           // size = 3
    int e[6] = {[1] = 10, [4] = 40};// designated (C99)

    for (int i = 0; i < 5; i++) printf("%d ", b[i]);
    printf("\n");
    for (int i = 0; i < 6; i++) printf("%d ", e[i]);
    printf("\n");
    printf("d has %zu elements\n", sizeof d / sizeof d[0]);
    (void)a; (void)c;
    return 0;
}
```

- Too many initializers → error.
- Local array without initializer → garbage.
- Global / static array → zero by default.
- `{0}` zeroes the whole array.

---

## 3. Array Length

- `sizeof arr` → total bytes.
- `sizeof arr[0]` → one element.
- Divide to get count.
- Works **only** on a real array, not a pointer.

```c
#include <stdio.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

int main(void) {
    int data[] = {3, 1, 4, 1, 5, 9, 2, 6};
    printf("bytes: %zu\n", sizeof data);          // 32
    printf("count: %zu\n", ARRAY_SIZE(data));     // 8
    return 0;
}
```

> [!warning] `ARRAY_SIZE` on a function parameter gives a wrong answer. The parameter is a pointer.

---

## 4. Memory Layout and Indexing

- `a[i]` is the same as `*(a + i)`.
- Pointer arithmetic scales by element size.
- Name of the array → address of first element.

```c
#include <stdio.h>

int main(void) {
    int a[4] = {10, 20, 30, 40};
    int *p = a;

    printf("%d %d %d\n", a[2], *(a + 2), p[2]);   // 30 30 30
    printf("%d\n", 2[a]);                          // 30 (legal, silly)

    for (int i = 0; i < 4; i++)
        printf("&a[%d] = %p\n", i, (void *)&a[i]); // +4 bytes each
    return 0;
}
```

---

## 5. Out-of-Bounds

- Reading or writing past the end → **undefined behavior**.
- May crash. May silently corrupt other variables.
- Biggest source of embedded bugs.

```c
// int a[3] = {1, 2, 3};
// a[3] = 99;          // one past the end. UB.
// a[-1] = 5;          // before the start. UB.
// for (i = 0; i <= 3; i++)   // off-by-one
```

Defence:

- Loop with `i < n`.
- Pass length with the array.
- Check index before use.
- Compile with `-Wall -Wextra -fsanitize=address` on PC.

```c
#include <stdio.h>
#include <stddef.h>

int get(const int *a, size_t n, size_t idx, int *out) {
    if (idx >= n) return -1;      // bounds check
    *out = a[idx];
    return 0;
}

int main(void) {
    int a[3] = {1, 2, 3};
    int v;
    printf("%d\n", get(a, 3, 5, &v));   // -1
    printf("%d\n", get(a, 3, 1, &v));   // 0
    printf("v = %d\n", v);              // 2
    return 0;
}
```

---

## 6. Arrays and Pointers

||Array|Pointer|
|---|---|---|
|Storage|the elements|an address|
|`sizeof`|whole array|pointer size|
|Assign `a = b`|not allowed|allowed|
|Can move `a++`|no|yes|
|Decays when passed|yes|n/a|

```c
#include <stdio.h>

int main(void) {
    int a[5] = {1, 2, 3, 4, 5};
    int *p = a;

    printf("%zu\n", sizeof a);     // 20
    printf("%zu\n", sizeof p);     // 8 (64-bit)
    p++;                           // ok
    // a++;                        // error
    printf("%d\n", *p);            // 2
    return 0;
}
```

Passing to functions: see [[C-04 Functions]]. Always pass the length.

---

## 7. Copying and Comparing Arrays

- `b = a` is **not** allowed.
- Use a loop or `memcpy`.
- `a == b` compares addresses, not content.
- Use a loop or `memcmp`.

```c
#include <stdio.h>
#include <string.h>

int main(void) {
    int a[4] = {1, 2, 3, 4};
    int b[4];

    memcpy(b, a, sizeof a);               // copy bytes
    printf("%d\n", memcmp(a, b, sizeof a) == 0);   // 1 (equal)
    printf("%d\n", a == b);                         // 0 (different addresses)
    return 0;
}
```

---

## 8. Common Array Operations

### Reverse in place

```c
#include <stdio.h>

void reverse(int *a, int n) {
    for (int i = 0, j = n - 1; i < j; i++, j--) {
        int t = a[i];
        a[i] = a[j];
        a[j] = t;
    }
}

int main(void) {
    int a[] = {1, 2, 3, 4, 5};
    reverse(a, 5);
    for (int i = 0; i < 5; i++) printf("%d ", a[i]);   // 5 4 3 2 1
    printf("\n");
    return 0;
}
```

### Linear search

```c
#include <stdio.h>

int find(const int *a, int n, int key) {
    for (int i = 0; i < n; i++)
        if (a[i] == key) return i;
    return -1;
}

int main(void) {
    int a[] = {4, 8, 15, 16, 23, 42};
    printf("%d\n", find(a, 6, 16));   // 3
    printf("%d\n", find(a, 6, 99));   // -1
    return 0;
}
```

### Bubble sort

```c
#include <stdio.h>

void bubble(int *a, int n) {
    for (int i = 0; i < n - 1; i++) {
        int swapped = 0;
        for (int j = 0; j < n - 1 - i; j++) {
            if (a[j] > a[j + 1]) {
                int t = a[j]; a[j] = a[j + 1]; a[j + 1] = t;
                swapped = 1;
            }
        }
        if (!swapped) break;
    }
}

int main(void) {
    int a[] = {5, 2, 9, 1, 7};
    bubble(a, 5);
    for (int i = 0; i < 5; i++) printf("%d ", a[i]);   // 1 2 5 7 9
    printf("\n");
    return 0;
}
```

### Binary search (sorted array)

```c
#include <stdio.h>

int bsearch_idx(const int *a, int n, int key) {
    int lo = 0, hi = n - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;      // avoids overflow
        if (a[mid] == key) return mid;
        if (a[mid] < key)  lo = mid + 1;
        else               hi = mid - 1;
    }
    return -1;
}

int main(void) {
    int a[] = {1, 3, 5, 7, 9, 11};
    printf("%d\n", bsearch_idx(a, 6, 7));    // 3
    return 0;
}
```

---

## 9. Two-Dimensional Arrays

- Array of arrays.
- Stored **row-major**. Row after row.
- Declare as `type name[rows][cols]`.

```c
#include <stdio.h>

int main(void) {
    int m[2][3] = {
        {1, 2, 3},
        {4, 5, 6}
    };

    for (int r = 0; r < 2; r++) {
        for (int c = 0; c < 3; c++)
            printf("%d ", m[r][c]);
        printf("\n");
    }

    int *flat = &m[0][0];                 // one block in memory
    for (int i = 0; i < 6; i++) printf("%d ", flat[i]);   // 1..6
    printf("\n");
    return 0;
}
```

### Passing a 2D array

- All dimensions except the first are required.

```c
#include <stdio.h>

void print_matrix(int m[][3], int rows) {
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < 3; c++)
            printf("%3d", m[r][c]);
        printf("\n");
    }
}

int main(void) {
    int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
    print_matrix(m, 2);
    return 0;
}
```

### Transpose

```c
#include <stdio.h>

int main(void) {
    int a[2][3] = {{1, 2, 3}, {4, 5, 6}};
    int t[3][2];

    for (int r = 0; r < 2; r++)
        for (int c = 0; c < 3; c++)
            t[c][r] = a[r][c];

    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 2; c++) printf("%d ", t[r][c]);
        printf("\n");
    }
    return 0;
}
```

---

## 10. Variable Length Arrays (VLA)

- C99: size can be a runtime value.
- Optional since C11.
- Lives on the stack. Size not checked.
- **Avoid in embedded.** Stack overflow risk.

```c
// int n = read_size();
// int buf[n];          // VLA. Avoid.
```

Use a fixed max-size buffer instead.

---

## 11. Strings in C

- No string type.
- A string is a `char` array ending with `'\0'` (null terminator).
- `'\0'` is the number 0.
- Length does **not** count the terminator.

```c
#include <stdio.h>

int main(void) {
    char s[] = {'H', 'i', '\0'};
    printf("%s\n", s);
    printf("%d %d %d\n", s[0], s[1], s[2]);   // 72 105 0
    return 0;
}
```

---

## 12. Creating Strings

```c
#include <stdio.h>

int main(void) {
    char a[] = "hello";          // size 6 (5 + '\0'), modifiable copy
    char b[10] = "hi";           // rest filled with '\0'
    const char *p = "world";     // points to read-only literal
    char c[5] = "hello";         // NO room for '\0'. Not a string!

    printf("%zu %zu %zu\n", sizeof a, sizeof b, sizeof p);
    a[0] = 'J';
    printf("%s %s %s\n", a, b, p);
    (void)c;
    return 0;
}
```

|Form|Modifiable|Storage|
|---|---|---|
|`char s[] = "hi"`|yes|array copy on stack / data|
|`const char *p = "hi"`|**no**|literal in read-only memory|

> [!warning] `char *p = "hi"; p[0] = 'H';` → undefined behavior. Use `const char *` for literals. `char c[5] = "hello"` has no terminator. `printf("%s")` reads past it.

> [!note] Embedded String literals live in Flash. They do not use RAM. `char s[] = "hi"` copies to RAM at startup.

---

## 13. `sizeof` vs `strlen`

- `sizeof` → bytes in the array. Compile time.
- `strlen` → characters before `'\0'`. Runtime.

```c
#include <stdio.h>
#include <string.h>

int main(void) {
    char s[20] = "embedded";
    printf("sizeof: %zu\n", sizeof s);      // 20
    printf("strlen: %zu\n", strlen(s));     // 8
    return 0;
}
```

---

## 14. String Input and Output

```c
#include <stdio.h>

int main(void) {
    char name[] = "Preet";
    printf("%s\n", name);
    printf("[%10s]\n", name);      // right align
    printf("[%-10s]\n", name);     // left align
    printf("[%.3s]\n", name);      // first 3 chars
    puts(name);                    // adds newline
    putchar('A'); putchar('\n');
    return 0;
}
```

### Reading input

- Use `fgets`. Safe with a size.
- `scanf("%s")` has no limit. Unsafe.
- `gets` is removed. Never use.
- `fgets` keeps the `'\n'`. Strip it.

```c
#include <stdio.h>
#include <string.h>

int main(void) {
    char line[32];

    if (fgets(line, sizeof line, stdin) != NULL) {
        line[strcspn(line, "\n")] = '\0';     // strip newline
        printf("you typed: %s\n", line);
    }
    return 0;
}
```

> [!note] Code Runner Needs stdin enabled. If not, use hard-coded strings.

---

## 15. `string.h` Functions

|Function|Does|
|---|---|
|`strlen(s)`|length|
|`strcpy(d, s)`|copy|
|`strncpy(d, s, n)`|copy up to n|
|`strcat(d, s)`|append|
|`strncat(d, s, n)`|append up to n|
|`strcmp(a, b)`|compare|
|`strncmp(a, b, n)`|compare first n|
|`strchr(s, c)`|first occurrence of char|
|`strrchr(s, c)`|last occurrence of char|
|`strstr(s, sub)`|find substring|
|`memcpy(d, s, n)`|copy bytes|
|`memmove(d, s, n)`|copy bytes, overlap safe|
|`memset(p, v, n)`|fill bytes|
|`memcmp(a, b, n)`|compare bytes|

### Copy, concatenate, length

```c
#include <stdio.h>
#include <string.h>

int main(void) {
    char buf[32];

    strcpy(buf, "Hello");
    strcat(buf, ", ");
    strcat(buf, "World");
    printf("%s (%zu)\n", buf, strlen(buf));   // Hello, World (12)
    return 0;
}
```

### Compare

- Returns `<0`, `0`, `>0`.
- **Never** use `==` to compare strings. It compares addresses.

```c
#include <stdio.h>
#include <string.h>

int main(void) {
    char a[] = "abc";
    char b[] = "abc";

    printf("%d\n", a == b);                 // 0 (different arrays)
    printf("%d\n", strcmp(a, b) == 0);      // 1 (same content)
    printf("%d\n", strcmp("apple", "banana") < 0);   // 1
    return 0;
}
```

### Search

```c
#include <stdio.h>
#include <string.h>

int main(void) {
    const char *s = "temperature=25";

    const char *eq = strchr(s, '=');
    if (eq != NULL)
        printf("index %td, value %s\n", eq - s, eq + 1);   // 11, 25

    const char *p = strstr(s, "per");
    printf("%s\n", p ? p : "not found");    // perature=25
    return 0;
}
```

### `memset`, `memcpy`, `memmove`

```c
#include <stdio.h>
#include <string.h>
#include <stdint.h>

int main(void) {
    uint8_t buf[8];
    memset(buf, 0xFF, sizeof buf);
    for (int i = 0; i < 8; i++) printf("%02X ", buf[i]);
    printf("\n");

    char s[] = "abcdef";
    memmove(s + 2, s, 4);          // overlapping move
    printf("%s\n", s);             // ababcd
    return 0;
}
```

- `memcpy` with overlapping regions → UB.
- Use `memmove` when ranges may overlap.
- `memset` sets **bytes**. `memset(a, 1, sizeof a)` on `int` array gives `0x01010101`, not 1.

---

## 16. Safe Copying

- `strcpy` and `strcat` do not check size.
- Source longer than destination → **buffer overflow**.

```c
// char small[4];
// strcpy(small, "overflow");     // writes past the array. UB.
```

### `strncpy` caveat

- Does **not** add `'\0'` if source ≥ n.
- Pads with zeros if source is shorter. Wastes time.

```c
#include <stdio.h>
#include <string.h>

int main(void) {
    char d[6];
    strncpy(d, "embedded", sizeof d - 1);
    d[sizeof d - 1] = '\0';            // always terminate
    printf("%s\n", d);                 // embed
    return 0;
}
```

### `snprintf` is the best default

- Never writes more than the size.
- Always terminates.
- Returns length it **wanted** to write. Use it to detect truncation.

```c
#include <stdio.h>

int main(void) {
    char buf[8];
    int n = snprintf(buf, sizeof buf, "T=%d C", 2500);
    printf("%s (wanted %d)\n", buf, n);     // T=2500  (wanted 8)  -> truncated
    if (n >= (int)sizeof buf)
        printf("truncated\n");
    return 0;
}
```

---

## 17. `ctype.h`

|Function|Checks|
|---|---|
|`isdigit(c)`|0–9|
|`isalpha(c)`|letter|
|`isalnum(c)`|letter or digit|
|`isspace(c)`|space, tab, newline|
|`isupper(c)` / `islower(c)`|case|
|`toupper(c)` / `tolower(c)`|convert|

```c
#include <stdio.h>
#include <ctype.h>

int main(void) {
    char s[] = "Hello 123";
    int letters = 0, digits = 0;

    for (int i = 0; s[i] != '\0'; i++) {
        if (isalpha((unsigned char)s[i])) letters++;
        else if (isdigit((unsigned char)s[i])) digits++;
        s[i] = (char)toupper((unsigned char)s[i]);
    }
    printf("%s  letters=%d digits=%d\n", s, letters, digits);
    return 0;
}
```

- Cast to `unsigned char` before passing. Negative `char` is UB.

---

## 18. Numbers ↔ Strings

### String to number

- `atoi` → no error detection. Avoid.
- `strtol` → detects bad input.

```c
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    char *end;

    long v = strtol("123abc", &end, 10);
    printf("%ld rest=%s\n", v, end);            // 123 rest=abc

    long h = strtol("FF", NULL, 16);
    printf("%ld\n", h);                         // 255

    double d = strtod("3.14xyz", &end);
    printf("%.2f rest=%s\n", d, end);           // 3.14 rest=xyz
    return 0;
}
```

- Check `end` is not the start → something was parsed.
- Check `*end == '\0'` → whole string used.

### Number to string

- Use `snprintf`.

```c
#include <stdio.h>

int main(void) {
    char buf[16];
    snprintf(buf, sizeof buf, "%d", 4096);
    printf("[%s]\n", buf);

    snprintf(buf, sizeof buf, "0x%04X", 255);
    printf("[%s]\n", buf);                      // 0x00FF
    return 0;
}
```

---

## 19. Writing String Functions by Hand

```c
#include <stdio.h>
#include <ctype.h>

size_t my_strlen(const char *s) {
    size_t n = 0;
    while (s[n] != '\0') n++;
    return n;
}

void my_strcpy(char *d, const char *s) {
    while ((*d++ = *s++) != '\0') { }
}

void reverse(char *s) {
    size_t n = my_strlen(s);
    for (size_t i = 0; i < n / 2; i++) {
        char t = s[i];
        s[i] = s[n - 1 - i];
        s[n - 1 - i] = t;
    }
}

int is_palindrome(const char *s) {
    size_t i = 0, j = my_strlen(s);
    if (j == 0) return 1;
    j--;
    while (i < j) {
        if (tolower((unsigned char)s[i]) != tolower((unsigned char)s[j])) return 0;
        i++; j--;
    }
    return 1;
}

int main(void) {
    char a[16];
    my_strcpy(a, "stressed");
    reverse(a);
    printf("%s\n", a);                        // desserts
    printf("%d\n", is_palindrome("Level"));   // 1
    return 0;
}
```

---

## 20. Array of Strings

Two ways:

```c
#include <stdio.h>

int main(void) {
    // 1. array of pointers. Compact. Read-only text.
    const char *days[] = {"Mon", "Tue", "Wednesday"};

    // 2. 2D char array. Fixed width. Modifiable.
    char names[3][8] = {"ann", "bob", "cy"};

    for (int i = 0; i < 3; i++)
        printf("%s %s\n", days[i], names[i]);

    printf("%zu %zu\n", sizeof days, sizeof names);   // 24 24 (64-bit)
    names[0][0] = 'A';
    printf("%s\n", names[0]);                          // Ann
    return 0;
}
```

|Form|RAM use|Modify|
|---|---|---|
|`const char *a[]`|pointers only|no|
|`char a[N][W]`|N × W bytes|yes|

> [!note] Embedded `const char *table[]` → table in Flash if declared `static const` and pointer array is const too: `static const char *const names[] = {...};`

---

## 21. Tokenizing with `strtok`

- Splits a string at delimiters.
- **Modifies** the string. Replaces delimiters with `'\0'`.
- Uses hidden static state. **Not reentrant.**
- Cannot be used on literals.

```c
#include <stdio.h>
#include <string.h>

int main(void) {
    char s[] = "a,b,c";

    for (char *t = strtok(s, ","); t != NULL; t = strtok(NULL, ","))
        printf("%s\n", t);
    return 0;
}
```

---

## 22. Embedded Patterns

### `uint8_t` for bytes, `char` for text

- Binary data → `uint8_t`.
- Text → `char`.
- Do not mix. `char` can be signed.

### Fixed buffers

- Allocate at compile time.
- No `malloc` in most firmware.
- Size with `#define`. Check against it.

```c
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define RX_BUF_SIZE 16

static uint8_t rx_buf[RX_BUF_SIZE];

int main(void) {
    memset(rx_buf, 0, sizeof rx_buf);
    printf("buf size = %zu\n", sizeof rx_buf);
    return 0;
}
```

### Lookup table in Flash

- `const` global arrays go to Flash.
- Saves RAM. Faster than computing.

```c
#include <stdio.h>
#include <stdint.h>

static const uint16_t pow10_tab[] = {1, 10, 100, 1000, 10000};

int main(void) {
    for (int i = 0; i < 5; i++)
        printf("%u ", pow10_tab[i]);
    printf("\n");
    return 0;
}
```

### Ring (circular) buffer

- Fixed array. `head` writes. `tail` reads.
- Wraps around with `%`.
- Standard for UART RX/TX.

```c
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define BUF_SIZE 4

typedef struct {
    uint8_t data[BUF_SIZE];
    uint8_t head, tail, count;
} Ring;

bool ring_put(Ring *r, uint8_t b) {
    if (r->count == BUF_SIZE) return false;     // full
    r->data[r->head] = b;
    r->head = (r->head + 1) % BUF_SIZE;
    r->count++;
    return true;
}

bool ring_get(Ring *r, uint8_t *b) {
    if (r->count == 0) return false;            // empty
    *b = r->data[r->tail];
    r->tail = (r->tail + 1) % BUF_SIZE;
    r->count--;
    return true;
}

int main(void) {
    Ring r = {0};
    uint8_t v;

    for (uint8_t i = 1; i <= 5; i++)
        printf("put %u -> %d\n", i, ring_put(&r, i));   // 5th fails

    ring_get(&r, &v); printf("got %u\n", v);   // 1
    ring_get(&r, &v); printf("got %u\n", v);   // 2

    ring_put(&r, 6);
    while (ring_get(&r, &v)) printf("got %u\n", v);     // 3 4 6
    return 0;
}
```

> [!warning] Shared between ISR and main → `count` update is not atomic. Use separate head/tail ownership or disable interrupts briefly. Make shared fields `volatile`.

### Viewing bytes: hex dump and endianness

```c
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

void hexdump(const uint8_t *p, size_t n) {
    for (size_t i = 0; i < n; i++) {
        printf("%02X ", p[i]);
        if ((i + 1) % 8 == 0) printf("\n");
    }
    printf("\n");
}

int main(void) {
    hexdump((const uint8_t *)"Hello", 5);        // 48 65 6C 6C 6F

    uint32_t x = 0x12345678;
    hexdump((const uint8_t *)&x, sizeof x);      // 78 56 34 12 on little-endian
    return 0;
}
```

- Little-endian: low byte first (ARM Cortex-M, x86).
- Byte arrays + casts: watch alignment and endianness on protocols.

### Simple checksum

```c
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

uint8_t checksum(const uint8_t *p, size_t n) {
    uint8_t sum = 0;
    for (size_t i = 0; i < n; i++) sum += p[i];    // wraps at 256
    return (uint8_t)(~sum + 1);                    // two's complement
}

int main(void) {
    uint8_t msg[] = {0x01, 0x02, 0x03};
    printf("0x%02X\n", checksum(msg, sizeof msg));  // 0xFA
    return 0;
}
```

---

## 23. Common Mistakes

- Index off by one. `i <= n`.
- Forgetting `'\0'` space. Buffer needs `len + 1`.
- `char c[5] = "hello"`.
- `==` to compare strings.
- `sizeof` on an array parameter.
- Modifying a string literal.
- `strcpy` / `strcat` into a small buffer.
- `strncpy` without manual terminator.
- `scanf("%s")` without width. `gets`.
- `memset` on `int` with nonzero value.
- `memcpy` on overlapping memory.
- Assigning arrays with `=`.
- Returning a local array from a function.
- VLA on a small stack.
- `isalpha` with negative `char`.
- Using `atoi` on untrusted input.
- Mixing `char` and `uint8_t` for binary data.

---

## 24. Embedded Cheat Sheet

- Fixed-size static buffers. No `malloc`.
- Always pass length with the array.
- Bounds-check every index from outside input.
- `sizeof arr / sizeof arr[0]` only on real arrays.
- Reserve 1 byte for `'\0'`.
- Use `snprintf`, not `sprintf`.
- Terminate after `strncpy`.
- Use `strtol`, not `atoi`.
- `uint8_t` for raw bytes. `char` for text.
- `static const` tables → Flash.
- Ring buffer for UART and sensors.
- Avoid `printf` with floats on tiny MCUs. Big code size.
- Avoid VLAs. Avoid big stack arrays.
- Zero buffers with `memset`. Know the byte meaning.
- Watch endianness when casting byte arrays.

---

## 25. Practice

1. Fill an array with squares of 1–10. Print.
2. Find min, max, and average of an array.
3. Reverse an array in place.
4. Remove duplicates from a sorted array.
5. Rotate an array left by k.
6. Merge two sorted arrays.
7. Sort with bubble sort. Then use `qsort`.
8. Binary search on a sorted array.
9. Add two 3×3 matrices.
10. Multiply two matrices.
11. Print the transpose of a matrix.
12. Write your own `strlen`, `strcpy`, `strcmp`.
13. Reverse a string in place.
14. Check for a palindrome. Ignore case.
15. Count vowels, consonants, digits.
16. Convert a string to uppercase without `toupper`.
17. Remove all spaces from a string.
18. Find the first non-repeating character.
19. Check if two strings are anagrams.
20. Parse `"key=value"` with `strchr`.
21. Convert `"1234"` to an int by hand. Handle bad input with `strtol`.
22. Convert a number to a binary string.
23. Implement a ring buffer. Test full and empty.
24. Print a hex dump of any array.
25. Show little-endian by printing bytes of `0x12345678`.
26. Compute an 8-bit checksum. Verify the sum is zero.
27. Reproduce a buffer overflow in a test. Fix with `snprintf`.
28. Split a CSV line with `strtok`.

---

## Quick Revision

- Array = same type, contiguous, index from 0, fixed size.
- `a[i]` ≡ `*(a + i)`.
- No bounds check. Out-of-bounds is UB.
- Array decays to pointer when passed. Pass the length.
- No `=` or `==` on arrays. Use `memcpy` and `memcmp`.
- 2D arrays are row-major. Pass all but the first dimension.
- String = `char` array ending in `'\0'`.
- Buffer needs length + 1.
- `char s[]` modifiable. `const char *` literal is read-only.
- Compare with `strcmp`, not `==`.
- Prefer `snprintf`, `strtol`, `fgets`.
- `memmove` for overlap.
- Embedded: fixed buffers, `uint8_t` bytes, ring buffers, const tables in Flash, mind endianness.

| previous: [[C-04 Functions]] | next: [[C-06 Pointers]] |
| ---------------------------- | ----------------------- |

