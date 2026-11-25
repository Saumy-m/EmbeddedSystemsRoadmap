> [!info]
> title: File I/O & Standard Library 
> tags: [C, stdio, stdlib, file-io, newlib] 
> chapter: 10


| previous: [[C-09 Memory Layout & Storage Classes]] | next: [[C-11 Build Tools, Debugging & Testing]] |
| -------------------------------------------------- | ----------------------------------------------- |

> [!note] Plan for this chapter Part A: full standard file I/O and the useful library headers. Runs on a PC. Part B: what changes on a microcontroller (no OS, no files, retargeting `printf`).

---

# Part A: Hosted C

## 1. Standard Library Map

|Header|Gives you|
|---|---|
|`<stdio.h>`|streams, `printf`, files|
|`<stdlib.h>`|`malloc`, `qsort`, `strtol`, `rand`, `exit`|
|`<string.h>`|`str*`, `mem*`|
|`<ctype.h>`|`isdigit`, `toupper`|
|`<math.h>`|`sqrt`, `pow`, `sin`, `floor`|
|`<time.h>`|`time`, `clock`, `strftime`|
|`<limits.h>`, `<float.h>`|type limits|
|`<stdint.h>`, `<inttypes.h>`|fixed-width types, printf macros|
|`<stdbool.h>`, `<stddef.h>`|`bool`, `size_t`, `NULL`, `offsetof`|
|`<stdarg.h>`|variadic functions|
|`<errno.h>`|`errno` error codes|
|`<assert.h>`|`assert`, `static_assert`|
|`<setjmp.h>`|non-local jumps|
|`<signal.h>`|signals|
|`<stdatomic.h>`|atomic operations (C11)|

Covered earlier: string and ctype ([[C-05 Arrays & Strings]]), `stdarg` ([[C-04 Functions]]), `assert` ([[C-08 Preprocessor & Macros]]), `malloc` ([[C-06 Pointers]]).

---

## 2. Streams

- A **stream** = a source or sink of bytes, used through a `FILE *`.
- Three open at program start:

| Stream   | Default  | Use                 |
| -------- | -------- | ------------------- |
| `stdin`  | keyboard | input               |
| `stdout` | screen   | normal output       |
| `stderr` | screen   | errors (unbuffered) |

```c
#include <stdio.h>

int main(void) {
    printf("to stdout\n");                      // same as fprintf(stdout, ...)
    fprintf(stderr, "to stderr\n");
    fputs("fputs adds no newline\n", stdout);
    puts("puts adds a newline");
    putchar('X'); putchar('\n');
    return 0;
}
```

### Buffering

|Mode|Behavior|Default for|
|---|---|---|
|fully buffered|flush when buffer full|files|
|line buffered|flush on `\n`|`stdout` to a terminal|
|unbuffered|write at once|`stderr`|

```c
#include <stdio.h>

int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);     // unbuffered. Call before any output.
    printf("no newline, shows at once");
    printf("\n");

    fflush(stdout);                        // force a flush of buffered output
    return 0;
}
```

- A crash can lose buffered text. Flush before risky calls.
- `fflush(stdin)` is undefined behavior. Never.

---

## 3. Opening and Closing Files

```c
// FILE *fopen(const char *path, const char *mode);
// int   fclose(FILE *f);
```

|Mode|Meaning|If file exists|If missing|
|---|---|---|---|
|`"r"`|read|ok|`NULL`|
|`"w"`|write|**erased**|created|
|`"a"`|append|keep, write at end|created|
|`"r+"`|read + write|ok|`NULL`|
|`"w+"`|read + write|**erased**|created|
|`"a+"`|read + append|keep|created|

- Add `b` for binary: `"rb"`, `"wb"`. No effect on Linux. Matters on Windows.
- C11 adds `"x"`: `"wx"` fails if the file exists.
- **Always** check for `NULL`.
- **Always** `fclose`. It flushes buffers and frees the handle.

```c
#include <stdio.h>

int main(void) {
    FILE *f = fopen("demo.txt", "w");
    if (f == NULL) {
        perror("fopen");
        return 1;
    }

    fprintf(f, "line %d\n", 1);
    fputs("second line\n", f);
    fclose(f);

    f = fopen("demo.txt", "r");
    if (f == NULL) { perror("fopen"); return 1; }

    char line[64];
    while (fgets(line, sizeof line, f) != NULL)
        printf("got: %s", line);

    fclose(f);
    remove("demo.txt");                  // delete the file
    return 0;
}
```

> [!note] Code Runner Files go to the current working directory of the run. The examples delete what they create.

---

## 4. Reading and Writing Text

|Function|Does|
|---|---|
|`fprintf(f, fmt, ...)`|formatted write|
|`fputs(s, f)`|write string|
|`fputc(c, f)` / `putc`|write one char|
|`fgets(buf, n, f)`|read a line (safe)|
|`fgetc(f)` / `getc`|read one char|
|`fscanf(f, fmt, ...)`|formatted read|

### Read char by char

- `fgetc` returns `int`, not `char`.
- It returns `EOF` (negative) at end or on error.
- Store in `int`. A `char` can miss `EOF`.

```c
#include <stdio.h>
#include <ctype.h>

int main(void) {
    FILE *f = fopen("count.txt", "w");
    if (!f) return 1;
    fputs("hello world\nfoo bar baz\n", f);
    fclose(f);

    f = fopen("count.txt", "r");
    if (!f) return 1;

    int c, chars = 0, lines = 0, words = 0, in_word = 0;
    while ((c = fgetc(f)) != EOF) {          // c is int
        chars++;
        if (c == '\n') lines++;
        if (isspace(c)) in_word = 0;
        else if (!in_word) { in_word = 1; words++; }
    }
    fclose(f);

    printf("chars=%d lines=%d words=%d\n", chars, lines, words);   // 24 2 5
    remove("count.txt");
    return 0;
}
```

### The `feof` trap

- `feof` becomes true only **after** a read hits the end.
- `while (!feof(f))` runs one extra time.

```c
// BAD:
// while (!feof(f)) {
//     fgets(line, sizeof line, f);     // last call fails, line is stale
//     printf("%s", line);
// }
//
// GOOD:
// while (fgets(line, sizeof line, f) != NULL) { ... }
```

After the loop, find out why it stopped:

```c
// if (ferror(f)) perror("read error");
// else            /* reached end of file */;
```

### `fgets` details

- Reads up to `n - 1` chars. Adds `'\0'`.
- Keeps the `'\n'`. Strip it:

```c
// line[strcspn(line, "\n")] = '\0';
```

- If a line is longer than the buffer, the rest stays for the next call.

### Parsing a line

```c
#include <stdio.h>

int main(void) {
    const char *line = "temp,25.5,ok";
    char name[16], status[8];
    float v;

    int n = sscanf(line, "%15[^,],%f,%7s", name, &v, status);
    if (n == 3)
        printf("%s %.1f %s\n", name, v, status);    // temp 25.5 ok
    else
        printf("bad line (%d fields)\n", n);
    return 0;
}
```

- `sscanf` / `fscanf` / `scanf` return the **number of fields matched**. Check it.
- Always give a **width** to `%s` and `%[...]`: `%15s`.
- `%[^,]` → read until a comma.
- `scanf("%d")` leaves the newline in the buffer. Prefer `fgets` + `sscanf`.

---

## 5. Binary Files

```c
// size_t fread (void *buf, size_t size, size_t count, FILE *f);
// size_t fwrite(const void *buf, size_t size, size_t count, FILE *f);
```

- Both return the number of **items** done. Compare with `count`.

```c
#include <stdio.h>

typedef struct {
    int   id;
    float temp;
} Rec;

int main(void) {
    Rec out[3] = {{1, 20.5f}, {2, 21.0f}, {3, 22.5f}};

    FILE *f = fopen("recs.bin", "wb");
    if (!f) return 1;
    size_t w = fwrite(out, sizeof out[0], 3, f);
    fclose(f);
    printf("wrote %zu records\n", w);

    f = fopen("recs.bin", "rb");
    if (!f) return 1;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);                     // file size in bytes
    rewind(f);
    printf("file size %ld bytes\n", size);    // 24

    Rec in[3];
    size_t r = fread(in, sizeof in[0], 3, f);
    printf("read %zu records\n", r);
    for (size_t i = 0; i < r; i++)
        printf("id=%d temp=%.1f\n", in[i].id, in[i].temp);

    // random access: jump to record 2
    fseek(f, (long)(1 * sizeof(Rec)), SEEK_SET);
    Rec second;
    if (fread(&second, sizeof second, 1, f) == 1)
        printf("second: id=%d\n", second.id);

    fclose(f);
    remove("recs.bin");
    return 0;
}
```

### Positioning

|Function|Does|
|---|---|
|`fseek(f, off, whence)`|move. `SEEK_SET`, `SEEK_CUR`, `SEEK_END`|
|`ftell(f)`|current position|
|`rewind(f)`|back to start, clears error|
|`fgetpos` / `fsetpos`|save and restore position (large files)|

> [!warning] Binary struct dumps `fwrite` of a struct writes **padding** and uses the **host's byte order**. Files are not portable across compilers, CPUs, or versions. For real formats: write each field explicitly in a fixed byte order (see [[C-07 Structs, Unions, Enums]]).

---

## 6. Errors: `errno`, `perror`, `strerror`

- Many library calls set `errno` on failure.
- Read it **right after** the failing call.
- Only meaningful when the call reported failure.

```c
#include <stdio.h>
#include <errno.h>
#include <string.h>

int main(void) {
    FILE *f = fopen("/no/such/file.txt", "r");
    if (f == NULL) {
        printf("errno = %d (%s)\n", errno, strerror(errno));
        perror("open");                       // prints to stderr: "open: ..."
        if (errno == ENOENT) printf("file does not exist\n");
        return 1;
    }
    fclose(f);
    return 0;
}
```

Check every result:

|Call|Failure sign|
|---|---|
|`fopen`|`NULL`|
|`fclose`|`EOF`|
|`fread` / `fwrite`|count less than requested|
|`fgets`|`NULL`|
|`fgetc`|`EOF`|
|`malloc`|`NULL`|
|`strtol`|check `errno == ERANGE` and `end`|
|`snprintf`|return `>= size` → truncated|

### Cleanup pattern with `goto`

```c
#include <stdio.h>

int copy_file(const char *src, const char *dst) {
    int status = -1;
    FILE *in = NULL, *out = NULL;

    in = fopen(src, "rb");
    if (!in) goto done;

    out = fopen(dst, "wb");
    if (!out) goto done;

    char buf[256];
    size_t n;
    while ((n = fread(buf, 1, sizeof buf, in)) > 0) {
        if (fwrite(buf, 1, n, out) != n) goto done;
    }
    if (ferror(in)) goto done;
    status = 0;

done:
    if (out) fclose(out);
    if (in)  fclose(in);
    return status;
}

int main(void) {
    FILE *f = fopen("a.txt", "w");
    if (!f) return 1;
    fputs("abc\n", f);
    fclose(f);

    printf("copy: %d\n", copy_file("a.txt", "b.txt"));    // 0

    f = fopen("b.txt", "r");
    char line[16];
    if (f && fgets(line, sizeof line, f)) printf("b.txt: %s", line);
    if (f) fclose(f);

    remove("a.txt");
    remove("b.txt");
    return 0;
}
```

Other file calls:

- `remove(path)` delete.
- `rename(old, new)` rename or move. Often atomic.
- `tmpfile()` temp file, auto-deleted on close.
- `freopen` reassign a stream.

---

## 7. Formatted I/O Notes

### Fixed-width types

- `%d` is wrong for `uint32_t` on some targets.
- Use `<inttypes.h>` macros.

```c
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

int main(void) {
    uint32_t a = 4000000000u;
    int64_t  b = -5000000000LL;
    uint8_t  c = 200;

    printf("%" PRIu32 " %" PRId64 " %" PRIu8 "\n", a, b, c);
    printf("0x%08" PRIX32 "\n", (uint32_t)0xBEEF);       // 0x0000BEEF

    uint32_t parsed;
    sscanf("1234", "%" SCNu32, &parsed);
    printf("%" PRIu32 "\n", parsed);
    return 0;
}
```

### Width from an argument

```c
#include <stdio.h>

int main(void) {
    int w = 8;
    printf("[%*d]\n", w, 42);          // [      42]
    printf("[%-*d]\n", w, 42);         // [42      ]
    printf("[%.*s]\n", 3, "abcdef");   // [abc]
    return 0;
}
```

### Format string bug

- Never pass user text as the format.

```c
// printf(user_text);            // %x %n in user_text read or write memory
// printf("%s", user_text);      // safe
```

- Avoid `%n`.
- Match specifier and type. `%d` with a `double` is UB.

### `printf` return value

- Returns characters written, or negative on error.
- `snprintf` returns the length it **wanted** (see [[C-05 Arrays & Strings]]).

---

## 8. `stdlib.h` Extras

### Integer helpers

```c
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

int main(void) {
    printf("%d\n", abs(-7));                    // 7
    printf("%ld\n", labs(-100000L));            // 100000
    // abs(INT_MIN) is UB: the result does not fit

    div_t d = div(17, 5);
    printf("%d r %d\n", d.quot, d.rem);         // 3 r 2
    (void)INT_MAX;
    return 0;
}
```

### `bsearch`

```c
#include <stdio.h>
#include <stdlib.h>

static int cmp_int(const void *a, const void *b) {
    int x = *(const int *)a, y = *(const int *)b;
    return (x > y) - (x < y);
}

int main(void) {
    int a[] = {2, 5, 8, 12, 23, 40};
    int key = 23;

    int *p = bsearch(&key, a, 6, sizeof a[0], cmp_int);   // array must be sorted
    if (p) printf("found at index %td\n", p - a);         // 4
    return 0;
}
```

### `exit`, `atexit`, `abort`

```c
#include <stdio.h>
#include <stdlib.h>

static void bye(void) { printf("cleanup\n"); }

int main(void) {
    atexit(bye);                // runs when the program ends normally
    printf("main done\n");
    return 0;                   // same as exit(0)
}
```

|Call|Does|
|---|---|
|`return` from `main`|like `exit(status)`|
|`exit(n)`|runs `atexit` handlers, flushes streams|
|`_Exit(n)`|ends at once. No handlers, no flush|
|`abort()`|abnormal end (signal)|

### Random numbers

- `rand()` is low quality, and its sequence depends on the platform.
- `rand() % n` is slightly biased.
- Fine for demos. Not for security.
- Embedded: a tiny PRNG you control.

```c
#include <stdio.h>
#include <stdint.h>

static uint32_t rng_state = 2463534242u;      // must be nonzero

static uint32_t xorshift32(void) {
    uint32_t x = rng_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    rng_state = x;
    return x;
}

int main(void) {
    for (int i = 0; i < 5; i++)
        printf("%u ", xorshift32() % 6 + 1);   // dice
    printf("\n");
    return 0;
}
```

- Same seed → same sequence. Good for repeatable tests.
- Seed from a noisy source (ADC noise, timer, UID) if you need variation.
- Security needs a real hardware RNG or a crypto library.

### Other

- `getenv("NAME")` read an environment variable.
- `system("cmd")` run a shell command. Avoid. Security and portability issues.

---

## 9. `math.h`

```c
#include <stdio.h>
#include <math.h>

int main(void) {
    double x = 2.0;
    printf("%.4f\n", sqrt(x));         // 1.4142
    printf("%.1f\n", pow(x, 10));      // 1024.0
    printf("%.1f %.1f\n", floor(-2.5), ceil(-2.5));    // -3.0 -2.0
    printf("%.1f\n", round(2.5));      // 3.0
    printf("%.1f\n", fmod(7.5, 2.0));  // 1.5
    printf("%.1f\n", fabs(-3.2));      // 3.2

    double bad = sqrt(-1.0);           // NaN
    printf("%d\n", isnan(bad) != 0);   // 1
    return 0;
}
```

- On Linux, link with `-lm`. Without it: `undefined reference to 'sqrt'`.
- Code Runner: add `-lm` to the C command in its settings (`code-runner.executorMap` → `"c"`), or this snippet will not link.
- Float versions: `sqrtf`, `sinf`, `powf`. Use them on `float` data.
- `==` on floats: compare with a tolerance (see [[C-01 Variables & Data Types]]).
- Check domain: `sqrt(-1)`, `log(0)`, divide by zero → NaN / inf.

> [!note] Embedded No FPU → `sin`, `pow`, `log` are slow and big. Options: lookup tables, fixed-point, CORDIC, integer square root. Cortex-M4F / M7 have a single-precision FPU. A `double` still runs in software. Use `float` and `f` suffixes: `1.5f`, `sinf()`.

---

## 10. `time.h`

```c
#include <stdio.h>
#include <time.h>

int main(void) {
    // convert a fixed timestamp to calendar time
    time_t t = 86400;                        // 1 day after 1970-01-01 00:00 UTC
    struct tm *g = gmtime(&t);

    char buf[32];
    strftime(buf, sizeof buf, "%Y-%m-%d %H:%M:%S", g);
    printf("%s\n", buf);                     // 1970-01-02 00:00:00

    // measure CPU time of a loop
    clock_t start = clock();
    volatile unsigned long sum = 0;
    for (unsigned long i = 0; i < 1000000UL; i++) sum += i;
    clock_t end = clock();

    double sec = (double)(end - start) / CLOCKS_PER_SEC;
    printf("elapsed >= 0: %d\n", sec >= 0.0);
    return 0;
}
```

|Function|Does|
|---|---|
|`time(NULL)`|seconds since the Epoch|
|`clock()`|CPU time used (ticks)|
|`difftime(a, b)`|`a - b` in seconds|
|`gmtime` / `localtime`|`time_t` → `struct tm`|
|`mktime`|`struct tm` → `time_t`|
|`strftime`|format a `struct tm`|

- `gmtime` / `localtime` return a pointer to a **static** struct. Not reentrant. Use `gmtime_r` (POSIX) in threads.
- `struct tm`: `tm_year` counts from 1900. `tm_mon` is 0–11.

> [!note] Embedded No OS clock. `time()` may not work. Keep your own tick counter (SysTick) and, if needed, an RTC. Store time as a plain `uint32_t` of seconds. Convert only for display.

---

## 11. `setjmp` / `longjmp`

- Jump back to an earlier point across function calls.
- Used for error recovery in C.
- Skips normal returns. No destructors in C, but cleanup is on you.

```c
#include <stdio.h>
#include <setjmp.h>

static jmp_buf env;

static void risky(int x) {
    if (x < 0) longjmp(env, 1);        // jump back to setjmp
    printf("ok %d\n", x);
}

int main(void) {
    if (setjmp(env) == 0) {            // first time: returns 0
        risky(5);
        risky(-1);
        printf("not reached\n");
    } else {                           // after longjmp: returns 1
        printf("recovered\n");
    }
    return 0;
}
```

Rules:

- Locals changed after `setjmp` and read after `longjmp` should be `volatile`.
- Do not `longjmp` into a function that has returned.
- Do not use it as normal control flow.
- Many safety standards (MISRA) forbid it.
- Firmware often uses error codes or a reset instead.

---

## 12. `signal` (PC) vs Interrupts (MCU)

- `signal` / `raise` simulate asynchronous events on a PC (used in [[C-09 Memory Layout & Storage Classes]]).
- Handlers may only safely:
    - set a `volatile sig_atomic_t` flag,
    - call async-signal-safe functions.
- No `printf`, no `malloc` inside a handler. Same rule as an ISR.

---

## 13. `stdatomic.h` (C11)

- Atomic types: operations cannot be split by an interrupt or another thread.
- Better than `volatile` for shared counters.

```c
#include <stdio.h>
#include <stdatomic.h>

int main(void) {
    atomic_int counter = 0;

    atomic_fetch_add(&counter, 1);
    atomic_fetch_add(&counter, 5);

    int old = atomic_exchange(&counter, 100);
    printf("old=%d now=%d\n", old, atomic_load(&counter));   // old=6 now=100

    int expected = 100;
    int ok = atomic_compare_exchange_strong(&counter, &expected, 200);
    printf("cas=%d value=%d\n", ok, atomic_load(&counter));  // cas=1 value=200
    return 0;
}
```

|Operation|Does|
|---|---|
|`atomic_load` / `atomic_store`|read / write|
|`atomic_fetch_add` / `_sub` / `_or` / `_and`|read-modify-write|
|`atomic_exchange`|swap|
|`atomic_compare_exchange_strong`|set only if equal (lock-free updates)|

> [!note] Embedded Cortex-M3 and up have `LDREX` / `STREX`. Cortex-M0 does not: the compiler may use a library call or fail to link. Check `ATOMIC_INT_LOCK_FREE`. Alternatives: briefly disable interrupts, or use an RTOS primitive.

---

# Part B: Embedded Considerations

## 14. Hosted vs Freestanding

||Hosted (PC)|Freestanding (bare metal)|
|---|---|---|
|OS|yes|no|
|Files|yes|none (unless you add a filesystem)|
|`printf` to screen|works|you must provide the output path|
|`main`|starts the program|called by startup code|
|Library|full|partial. You pick what to link.|

- Always available in freestanding mode: `<float.h>`, `<limits.h>`, `<stdarg.h>`, `<stdbool.h>`, `<stddef.h>`, `<stdint.h>` (plus `<iso646.h>`, `<stdalign.h>`, `<stdnoreturn.h>`).
- The rest (`stdio`, `stdlib`, `string`, `math`...) come from a library such as **newlib** or **picolibc** that you link in.
- Flags: `-ffreestanding`, `-nostdlib`, `-nostartfiles` when you write everything yourself.

---

## 15. Newlib, Newlib-nano, and Syscalls

Typical `arm-none-eabi-gcc` setup:

|Option|Effect|
|---|---|
|`--specs=nano.specs`|newlib-nano: smaller `printf`, `malloc`|
|`--specs=nosys.specs`|stubs for system calls. Links without a real OS.|
|`-u _printf_float`|enable `%f` in nano `printf` (costs several KB)|
|`-u _scanf_float`|enable `%f` in nano `scanf`|
|`-Wl,--gc-sections` + `-ffunction-sections -fdata-sections`|drop unused code|

Newlib talks to the OS through small stubs. You implement them for your board:

|Stub|Used by|
|---|---|
|`_write`|`printf`, `puts`, `fwrite`|
|`_read`|`scanf`, `fgets(stdin)`|
|`_sbrk`|`malloc`|
|`_close`, `_lseek`, `_fstat`, `_isatty`|file layer|
|`_exit`, `_kill`, `_getpid`|`exit`, `abort`|

Cost of `printf`:

- Full `printf` can pull **tens of KB** of Flash.
- Newlib-nano `printf` without float is much smaller.
- Float printing is the big extra.
- `printf` also needs a lot of **stack** (hundreds of bytes).

---

## 16. Retargeting `printf` to a UART

Make `printf` write to your UART by implementing `_write`.

> [!note] Hardware code. Not for Code Runner.

```text
#include <unistd.h>
#include <errno.h>

extern void uart_putc(char c);      /* your driver */

int _write(int fd, const char *buf, int len) {
    if (fd != STDOUT_FILENO && fd != STDERR_FILENO) {
        errno = EBADF;
        return -1;
    }
    for (int i = 0; i < len; i++) {
        if (buf[i] == '\n') uart_putc('\r');     /* terminals want CRLF */
        uart_putc(buf[i]);
    }
    return len;
}
```

Notes:

- `printf` output is **line buffered or fully buffered**. If you see nothing on the UART, add `\n` or call `fflush(stdout)` or `setvbuf(stdout, NULL, _IONBF, 0)`.
- `printf` is **not** safe in an ISR.
- With an RTOS, protect it with a mutex or send output to a logging task.
- Other output paths: **semihosting** (via debugger, very slow), **ITM / SWO** trace, **SEGGER RTT**.

---

## 17. A Light-weight `printf` of Your Own

- Format into a small buffer. Send the bytes.
- No `FILE`, no stdio buffering, fixed stack use.

```c
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>

static void uart_write(const char *s, size_t n) {   // stand-in for the UART driver
    fwrite(s, 1, n, stdout);
}

int kprintf(const char *fmt, ...) {
    char buf[64];
    va_list ap;

    va_start(ap, fmt);
    int n = vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);

    if (n < 0) return n;
    size_t len = ((size_t)n < sizeof buf) ? (size_t)n : sizeof buf - 1;   // truncated?
    uart_write(buf, len);
    return n;
}

int main(void) {
    kprintf("temp=%d C, mode=%s\n", 25, "auto");
    return 0;
}
```

### Number to text without `printf`

- A few lines replace `printf("%u")`.

```c
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

size_t u32_to_dec(uint32_t v, char *out) {
    char tmp[10];
    size_t n = 0;

    do {
        tmp[n++] = (char)('0' + v % 10);
        v /= 10;
    } while (v != 0);

    for (size_t i = 0; i < n; i++)
        out[i] = tmp[n - 1 - i];
    out[n] = '\0';
    return n;
}

int main(void) {
    char s[11];
    u32_to_dec(0, s);           printf("%s\n", s);    // 0
    u32_to_dec(4096, s);        printf("%s\n", s);    // 4096
    u32_to_dec(4294967295u, s); printf("%s\n", s);    // 4294967295
    return 0;
}
```

### Print a float without `%f`

- Scale to an integer. Print with an integer `printf`.

```c
#include <stdio.h>
#include <stdint.h>

void print_fixed2(float f) {
    int32_t centi = (int32_t)(f * 100.0f + (f < 0 ? -0.5f : 0.5f));   // round
    char sign = '+';
    if (centi < 0) { sign = '-'; centi = -centi; }
    printf("%c%d.%02d\n", sign, (int)(centi / 100), (int)(centi % 100));
}

int main(void) {
    print_fixed2(25.37f);      // +25.37
    print_fixed2(-3.5f);       // -3.50
    print_fixed2(0.05f);       // +0.05
    return 0;
}
```

- Saves the whole float formatting path.
- Range is limited by `int32_t`.

---

## 18. Files on Microcontrollers

- Bare metal has no file API. Add a filesystem library:

|Library|Medium|Notes|
|---|---|---|
|**FatFs**|SD card, USB stick|FAT32, widely used|
|**littlefs**|SPI / internal Flash|power-loss safe, wear leveling|
|**SPIFFS**|small SPI Flash|older, ESP-style|
|**LittleFS / FAT via VFS**|RTOS BSPs|gives `fopen` style calls|

FatFs style:

```text
FIL f;
UINT bw;

if (f_open(&f, "log.txt", FA_WRITE | FA_OPEN_APPEND) == FR_OK) {
    f_write(&f, "boot\n", 5, &bw);
    f_close(&f);
}
```

Same ideas as stdio: open, check, write, close, check result.

Flash and storage rules:

- Flash has **limited erase cycles**. Do not write every second to one spot.
- Erase before write. Erase size is a sector (KBs).
- Power can fail mid-write. Design for it.
- Safe update: write to a **temp file / new slot**, check a **CRC**, then switch (rename or flip a pointer).
- Keep a version and CRC in every stored block.
- Do slow storage writes outside ISRs and time-critical code.
- Fixed-size records in a ring of sectors beat rewriting one file.

---

## 19. Reentrancy and the Library

Many library functions use hidden static state. Dangerous with ISRs or RTOS tasks.

|Function|Problem|Safer|
|---|---|---|
|`strtok`|static pointer|`strtok_r`|
|`rand` / `srand`|static seed|own PRNG|
|`localtime` / `gmtime`|static result|`_r` variants, or own code|
|`strerror`|static buffer|`strerror_r`|
|`malloc` / `free`|shared heap|lock, or avoid|
|`printf` family|shared stdout buffer|mutex, or per-task buffer|
|`errno`|one global|thread-local `errno` in RTOS-aware libc|

Newlib has a reentrancy struct (`_reent`). RTOS ports switch it per task. Some RTOSes need `configUSE_NEWLIB_REENTRANT`.

---

## 20. Common Mistakes

- Not checking `fopen` result.
- Forgetting `fclose`. Data lost in buffers.
- `while (!feof(f))`.
- `char c = fgetc(f)` then comparing with `EOF`.
- `gets`. `scanf("%s")` without a width.
- Using `printf(user_input)`.
- Ignoring `fread`, `fwrite`, `sscanf` return values.
- Binary struct dump used as a file format.
- Reading `errno` without a failed call.
- `"w"` mode erasing a file by mistake.
- `rand() % n` for important randomness.
- `abs(INT_MIN)`.
- `sqrt` with no `-lm`. `float` data through `double` functions.
- Using `time()` or `clock()` as if an RTC exists.
- `longjmp` into a dead function.
- `printf` in an ISR.
- `printf("%f")` linking huge code or printing nothing (nano without `-u _printf_float`).
- No `\n` and no flush: UART shows nothing.
- `malloc` inside a task without a lock.
- Calling non-reentrant library functions from ISR and main.
- Writing Flash too often or without power-fail design.

---

## 21. Embedded Cheat Sheet

- Know what your libc provides. Link `nano.specs` to save space.
- Implement `_write` for output, `_sbrk` for heap.
- Prefer your own `log` / `kprintf` with a bounded buffer.
- Avoid float `printf`. Print scaled integers.
- Use `snprintf` plus a size check. Never `sprintf`.
- Use `strtol`, not `atoi`. Check the end pointer.
- Own PRNG. Seed from real noise.
- Time = tick counter. RTC only if you need calendar time.
- No `printf`, `malloc` or file calls in an ISR.
- `atomic_*` or short critical sections for shared counters.
- Avoid `setjmp`, `system`, `getenv`, `exit`.
- Filesystem: FatFs for removable media, littlefs for on-chip Flash.
- Write with CRC, versions, and atomic switch-over.
- Check Flash and stack cost of each library function you pull in (`size`, map file).

---

## 22. Practice

1. Write lines to a file. Read them back with `fgets`. Delete the file.
2. Count lines, words, and characters in a text file.
3. Copy a file with `fread` / `fwrite`. Use a `goto` cleanup.
4. Append a timestamp line to a log file with `"a"` mode.
5. Parse `"key=value"` lines with `sscanf`. Reject bad lines.
6. Read a CSV of numbers. Compute min, max, mean.
7. Write 5 structs with `fwrite`. Read them back. Print the file size.
8. Jump to record 3 with `fseek`. Read just that record.
9. Write a record with explicit field-by-field big-endian bytes. Compare with `fwrite` of the struct.
10. Show the `feof` bug. Fix it.
11. Cause `ENOENT` on purpose. Print `errno`, `strerror`, `perror`.
12. Use `setvbuf` to make `stdout` unbuffered. Compare with the default when output is piped.
13. Print `uint32_t`, `uint8_t`, `int64_t` with `inttypes.h`.
14. Use `bsearch` on a sorted array. Handle "not found".
15. Use `atexit` with two handlers. Predict the order (last registered runs first).
16. Write `xorshift32`. Test the distribution of 6000 dice rolls.
17. Use `strftime` on `gmtime(&t)` for a few fixed `t` values.
18. Measure a function with `clock()`. Explain why this is CPU time, not wall time.
19. Build error recovery with `setjmp` / `longjmp`. Then rewrite with return codes.
20. Use `atomic_fetch_add` from two threads (pthreads) and compare with a plain `int`.
21. Write `kprintf` with `vsnprintf` and a 64-byte buffer. Test truncation.
22. Write `u32_to_dec` and `i32_to_dec` without `printf`.
23. Print a float as fixed-point with integers only.
24. Write a `_write` retarget in comments for your MCU's UART. List the other stubs `nosys.specs` provides.
25. Build the same program with and without `--specs=nano.specs`. Compare `size`.

---

## Quick Revision

- `FILE *` stream. `stdin`, `stdout`, `stderr` exist at start.
- `fopen` → check → use → `fclose`. Modes: `r w a r+ w+ a+` and `b`.
- `"w"` erases. `"a"` appends.
- `fgets` for lines. `fgetc` returns `int`. Loop on the return value, not `feof`.
- `sscanf` / `fscanf` → check the field count. Give widths.
- `fread` / `fwrite` count items. Struct dumps are not portable.
- `fseek`, `ftell`, `rewind` for position.
- `errno` + `perror` / `strerror` right after a failure.
- `inttypes.h` macros for fixed-width printing.
- `exit` runs `atexit` handlers. `_Exit` does not.
- `rand` is weak. Use your own PRNG on MCUs.
- `math.h` needs `-lm`. Prefer `float` functions. Avoid on FPU-less chips.
- `time.h` needs an OS clock. On MCUs keep your own tick counter.
- `setjmp` / `longjmp` for rare recovery. `stdatomic.h` for lock-free shared data.
- Bare metal: link newlib-nano, implement `_write` / `_sbrk`, mind `printf` cost.
- Own `kprintf` and integer-only formatting save space and stack.
- Filesystems: FatFs, littlefs. Protect against power loss with CRC and atomic switch.
- Library functions with hidden state are not reentrant.

| previous: [[C-09 Memory Layout & Storage Classes]] | next: [[C-11 Build Tools, Debugging & Testing]] |
| -------------------------------------------------- | ----------------------------------------------- |

