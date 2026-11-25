> [!info]
> title: Control Flow 
> tags: [C, basics, control-flow] 
> chapter 3

| previous: [[C-02 Operators]] | next: [[C-04 Functions]] |
| ---------------------------- | ------------------------ |
## 1. Overview

- Code runs top to bottom by default.
- Control flow changes that order.

Groups:

- Decision → `if`, `else`, `switch`, `?:`
- Loop → `while`, `do-while`, `for`
- Jump → `break`, `continue`, `goto`, `return`

---

## 2. Statements and Blocks

- Statement ends with `;`.
- Block `{ }` groups statements.
- Block creates a new scope.
- Always use braces. Even for one line.

```c
#include <stdio.h>

int main(void) {
    int x = 5;
    {
        int y = 10;          // y lives only here
        printf("%d %d\n", x, y);
    }
    // printf("%d", y);      // error: y not visible
    return 0;
}
```

---

## 3. Truth in C

- `0` → false.
- Any nonzero → true.
- Relational and logical ops give `0` or `1`.
- Pointers: `NULL` is false.

```c
#include <stdio.h>

int main(void) {
    if (5)      printf("5 is true\n");
    if (-1)     printf("-1 is true\n");
    if (0)      printf("never\n");
    if (!0)     printf("!0 is true\n");
    return 0;
}
```

---

## 4. `if`, `else if`, `else`

Syntax:

```c
// if (condition) { ... }
// else if (condition) { ... }
// else { ... }
```

```c
#include <stdio.h>

int main(void) {
    int marks = 72;

    if (marks >= 90) {
        printf("A\n");
    } else if (marks >= 75) {
        printf("B\n");
    } else if (marks >= 60) {
        printf("C\n");
    } else {
        printf("Fail\n");
    }
    return 0;
}
```

- Checked top to bottom.
- First true branch runs. Rest skipped.
- Order matters. Put narrow checks first.

### Common traps

```c
#include <stdio.h>

int main(void) {
    int x = 3;

    if (x == 5);               // stray semicolon
        printf("always runs\n");

    if (x = 5)                 // assignment, not compare
        printf("always true\n");
    return 0;
}
```

- `if (cond);` → empty body. Next line runs always.
- `=` vs `==`. Compiler warning usually. Keep `-Wall`.

### Dangling else

- `else` binds to the nearest `if`.
- Indentation does not matter to compiler.

```c
#include <stdio.h>

int main(void) {
    int a = 1, b = 0;

    if (a)
        if (b)
            printf("both\n");
        else
            printf("belongs to inner if\n");   // this runs
    return 0;
}
```

Fix: use braces.

### Guard clauses

- Check bad cases first. Return early.
- Less nesting. Easier to read.

```c
#include <stdio.h>

int divide(int a, int b, int *out) {
    if (out == NULL) return -1;
    if (b == 0)      return -2;
    *out = a / b;
    return 0;
}

int main(void) {
    int r;
    printf("%d\n", divide(10, 0, &r));   // -2
    printf("%d\n", divide(10, 2, &r));   // 0
    printf("r = %d\n", r);               // 5
    return 0;
}
```

---

## 5. Ternary vs `if`

- Ternary for short value choice.
- `if` for actions and longer logic.

```c
#include <stdio.h>

int main(void) {
    int n = 8;
    const char *s = (n % 2 == 0) ? "even" : "odd";
    printf("%s\n", s);
    return 0;
}
```

---

## 6. `switch`

- Picks a branch by integer value.
- Works with `int`, `char`, `enum`.
- Case labels must be **constants**.
- No floats. No strings. No ranges (standard C).

```c
#include <stdio.h>

int main(void) {
    int day = 3;

    switch (day) {
        case 1:
            printf("Mon\n");
            break;
        case 2:
            printf("Tue\n");
            break;
        case 3:
            printf("Wed\n");
            break;
        default:
            printf("Other\n");
            break;
    }
    return 0;
}
```

### Fall-through

- No `break` → next case also runs.
- Useful to group cases.
- Mark intentional fall-through with a comment.

```c
#include <stdio.h>

int main(void) {
    char c = 'e';

    switch (c) {
        case 'a':
        case 'e':
        case 'i':
        case 'o':
        case 'u':
            printf("vowel\n");
            break;
        default:
            printf("consonant\n");
    }
    return 0;
}
```

Accidental fall-through:

```c
#include <stdio.h>

int main(void) {
    int x = 1;
    switch (x) {
        case 1: printf("one\n");     // missing break
        case 2: printf("two\n");     // also runs
        case 3: printf("three\n");   // also runs
                break;
    }
    return 0;
}
```

> [!warning] Missing `break` Classic bug. Compile with `-Wimplicit-fallthrough`. In C23 / GCC: `[[fallthrough]];` or `__attribute__((fallthrough));`.

### Declarations inside `case`

- Label cannot be followed by a declaration (before C23).
- Wrap in braces.

```c
#include <stdio.h>

int main(void) {
    int op = 1;

    switch (op) {
        case 1: {
            int temp = 42;
            printf("%d\n", temp);
            break;
        }
        default:
            break;
    }
    return 0;
}
```

### Rules

- `default` is optional. Can be anywhere. Keep it last.
- Duplicate case values → error.
- `break` exits the `switch`, not an outer loop.
- Always add `default` for enums. Catches bad values.

### `if-else` or `switch`?

|Use `switch`|Use `if-else`|
|---|---|
|One variable, many fixed values|Ranges|
|enum / state / command code|Complex conditions|
|Compiler may build a jump table|Floats, strings, pointers|

---

## 7. `while` Loop

- Checks condition **first**.
- May run zero times.

```c
#include <stdio.h>

int main(void) {
    int i = 1;
    int sum = 0;

    while (i <= 5) {
        sum += i;
        i++;
    }
    printf("sum = %d\n", sum);    // 15
    return 0;
}
```

Parts:

1. Init before loop.
2. Condition in `while`.
3. Update inside body.

> [!warning] Forget the update → infinite loop. `while (cond);` → empty body.

---

## 8. `do-while` Loop

- Body runs **first**. Condition checked after.
- Runs at least once.
- Note the `;` after `while(...)`.

```c
#include <stdio.h>

int main(void) {
    int n = 0;

    do {
        printf("n = %d\n", n);
        n++;
    } while (n < 3);

    int k = 10;
    do {
        printf("runs once, k = %d\n", k);
    } while (k < 5);
    return 0;
}
```

Use for: retry, menu, input validation.

### `do { } while (0)` macro idiom

- Makes multi-statement macro act like one statement.

```c
#include <stdio.h>

#define LOG_TWICE(msg) do { printf("%s\n", msg); printf("%s\n", msg); } while (0)

int main(void) {
    int on = 1;
    if (on)
        LOG_TWICE("hello");
    else
        printf("off\n");
    return 0;
}
```

- Without the wrapper, `if/else` would break.

---

## 9. `for` Loop

Syntax:

```c
// for (init; condition; update) { body }
```

Order:

1. `init` once.
2. `condition` checked.
3. `body` runs.
4. `update` runs.
5. Go to step 2.

```c
#include <stdio.h>

int main(void) {
    for (int i = 0; i < 5; i++) {
        printf("%d ", i);
    }
    printf("\n");
    return 0;
}
```

- `int i` inside `for` → C99+. Scope is the loop only.
- Any part can be empty.

### Variations

```c
#include <stdio.h>

int main(void) {
    // count down
    for (int i = 5; i > 0; i--) printf("%d ", i);
    printf("\n");

    // step by 2
    for (int i = 0; i <= 10; i += 2) printf("%d ", i);
    printf("\n");

    // two variables
    for (int i = 0, j = 5; i < j; i++, j--)
        printf("(%d,%d) ", i, j);
    printf("\n");

    // empty parts
    int k = 0;
    for (; k < 3; ) { printf("%d ", k); k++; }
    printf("\n");
    return 0;
}
```

### Infinite loops

```c
// for (;;) { ... }
// while (1) { ... }
```

Both are common. Embedded main loop uses one.

### Off-by-one

- Array size `n` → valid index `0` to `n-1`.
- Use `i < n`, not `i <= n`.

```c
#include <stdio.h>

int main(void) {
    int arr[4] = {10, 20, 30, 40};
    int n = sizeof arr / sizeof arr[0];

    for (int i = 0; i < n; i++)       // correct
        printf("%d ", arr[i]);
    printf("\n");
    // for (int i = 0; i <= n; i++)   // reads arr[4]. UB.
    return 0;
}
```

### Unsigned countdown trap

- `unsigned` is never negative.
- `i >= 0` is always true.
- Loop never ends.

```c
#include <stdio.h>
#include <stddef.h>

int main(void) {
    size_t n = 4;

    // BAD: for (size_t i = n - 1; i >= 0; i--)   infinite

    // GOOD: test then decrement
    for (size_t i = n; i-- > 0; )
        printf("%zu ", i);               // 3 2 1 0
    printf("\n");
    return 0;
}
```

### Float counters

- Avoid. Rounding errors.
- Loop on an integer. Compute the float inside.

```c
#include <stdio.h>

int main(void) {
    for (int i = 0; i <= 10; i++) {
        float x = i * 0.1f;
        printf("%.1f ", x);
    }
    printf("\n");
    return 0;
}
```

---

## 10. `while` vs `do-while` vs `for`

|Loop|Use when|
|---|---|
|`for`|count known|
|`while`|condition-driven, may skip|
|`do-while`|must run once|

---

## 11. `break` and `continue`

- `break` → exit nearest loop or `switch`.
- `continue` → skip to next iteration.
    - In `for`, update still runs.

```c
#include <stdio.h>

int main(void) {
    for (int i = 1; i <= 10; i++) {
        if (i % 2 == 0) continue;     // skip evens
        if (i > 7) break;             // stop
        printf("%d ", i);             // 1 3 5 7
    }
    printf("\n");
    return 0;
}
```

> [!warning] `continue` in `while` → make sure update is not skipped. Else infinite loop.

### Breaking out of nested loops

- `break` leaves only the inner loop.
- Use a flag, a function `return`, or `goto`.

```c
#include <stdio.h>

int main(void) {
    int found = 0;

    for (int i = 0; i < 5 && !found; i++) {
        for (int j = 0; j < 5; j++) {
            if (i * j == 6) {
                printf("found %d x %d\n", i, j);
                found = 1;
                break;
            }
        }
    }
    return 0;
}
```

---

## 12. Nested Loops

- Inner loop runs fully for each outer pass.
- Total runs = outer × inner.

```c
#include <stdio.h>

int main(void) {
    for (int i = 1; i <= 4; i++) {
        for (int j = 1; j <= i; j++)
            printf("*");
        printf("\n");
    }
    return 0;
}
```

Multiplication table:

```c
#include <stdio.h>

int main(void) {
    for (int i = 1; i <= 3; i++) {
        for (int j = 1; j <= 5; j++)
            printf("%3d", i * j);
        printf("\n");
    }
    return 0;
}
```

---

## 13. `goto`

- Jumps to a label in the same function.
- Avoid for normal flow.
- **One good use: error cleanup.** Common in Linux kernel and drivers.

```c
#include <stdio.h>
#include <stdlib.h>

int setup(void) {
    int status = -1;

    char *a = malloc(16);
    if (a == NULL) goto out;

    char *b = malloc(32);
    if (b == NULL) goto free_a;

    printf("setup ok\n");
    status = 0;

    free(b);
free_a:
    free(a);
out:
    return status;
}

int main(void) {
    return setup();
}
```

- Labels release resources in reverse order.
- Never jump into a block that skips initialization.

---

## 14. `return`

- Ends the function now.
- Can appear in many places.
- `main` returns `0` for success.

```c
#include <stdio.h>

int first_negative(const int *a, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] < 0) return i;     // early exit
    }
    return -1;
}

int main(void) {
    int data[] = {4, 7, -2, 9};
    printf("%d\n", first_negative(data, 4));    // 2
    return 0;
}
```

---

## 15. Embedded Patterns

### Super loop

- No OS. `main` never returns.
- Init once. Then loop forever.

```c
#include <stdio.h>

static int tick = 0;

int main(void) {
    // init_clock(); init_gpio(); init_uart();

    for (;;) {
        tick++;
        if (tick % 3 == 0) printf("task A @ %d\n", tick);
        if (tick % 5 == 0) printf("task B @ %d\n", tick);

        if (tick >= 10) break;     // only for this demo
    }
    return 0;
}
```

### Polling with timeout

- Never wait forever on hardware.
- Count attempts. Fail safely.

```c
#include <stdio.h>
#include <stdbool.h>

static int calls = 0;
static bool device_ready(void) { return ++calls >= 4; }   // fake hardware

int wait_ready(int max_tries) {
    while (max_tries--) {
        if (device_ready()) return 0;      // ok
    }
    return -1;                             // timeout
}

int main(void) {
    printf("%d\n", wait_ready(10));        // 0
    return 0;
}
```

### Waiting on a flag set by ISR

- Flag must be `volatile`.
- Else compiler may cache it. Loop never exits.

```c
// volatile uint8_t data_ready = 0;       // set in ISR
// while (!data_ready) { }                // wait
// data_ready = 0;
```

### State machine with `switch`

- State stored in an `enum`.
- `switch` picks the action.
- Very common in firmware.

```c
#include <stdio.h>

typedef enum { RED, GREEN, YELLOW } State;

int main(void) {
    State s = RED;

    for (int step = 0; step < 6; step++) {
        switch (s) {
            case RED:
                printf("RED\n");
                s = GREEN;
                break;
            case GREEN:
                printf("GREEN\n");
                s = YELLOW;
                break;
            case YELLOW:
                printf("YELLOW\n");
                s = RED;
                break;
            default:
                s = RED;           // recover from bad state
                break;
        }
    }
    return 0;
}
```

### Busy-wait delay

- Burns CPU. Avoid. Use timers.
- Compiler may remove an empty loop unless `volatile`.

```c
// for (volatile uint32_t i = 0; i < 100000; i++) { }
```

---

## 16. Common Mistakes

- `;` right after `if`, `for`, `while`.
- `=` instead of `==` in conditions.
- Missing `break` in `switch`.
- No `default` in `switch`.
- Declaring variable right after `case` without braces.
- Off-by-one: `<=` vs `<`.
- Unsigned countdown `i >= 0`.
- Infinite loop: forgot update.
- `continue` skipping the update in `while`.
- `break` expecting to leave outer loop.
- Float as loop counter.
- Dangling else.
- Polling a flag without `volatile`.
- No timeout on hardware wait.

---

## 17. Embedded Cheat Sheet

- Always use braces.
- Use `switch` for states and commands.
- Add `default` to every `switch`.
- Mark intentional fall-through.
- Use `for (;;)` for the main loop.
- Add timeout to every polling loop.
- Shared flags with ISR → `volatile`.
- Prefer timers over busy-wait.
- Use `goto` only for cleanup.
- Keep loop bounds as `i < n`.
- Use guard clauses. Return early.

---

## 18. Practice

1. Print grade from marks with `if-else if`.
2. Check if a year is a leap year.
3. Find the largest of three numbers.
4. Print weekday name from 1–7 with `switch`.
5. Vowel or consonant with grouped cases.
6. Sum of 1 to N with `while`.
7. Sum of digits of a number.
8. Reverse the digits of a number.
9. Print the multiplication table of N.
10. Print a right-angle star pattern.
11. FizzBuzz for 1–30.
12. Check if a number is prime.
13. Print the Fibonacci series, first N terms.
14. Find the first negative number in an array. Return its index.
15. Countdown from array length using `size_t` safely.
16. Break out of a nested loop with a flag.
17. Build a traffic-light state machine with an `enum`.
18. Write a polling function with a timeout.
19. Use `goto` cleanup with two `malloc` calls.
20. Use `do-while` to retry until a fake sensor returns success.

---

## Quick Revision

- `0` false. Nonzero true.
- Always use braces.
- `else` pairs with nearest `if`.
- `switch` works on integers. Needs `break`. Add `default`.
- `while` checks first. `do-while` runs once.
- `for` → init, check, body, update.
- `break` leaves one level. `continue` skips to next pass.
- Unsigned countdown needs `i-- > 0`.
- `goto` only for cleanup.
- Embedded: super loop, state machine, timeout, `volatile` flag.

| previous: [[C-03 Control Flow]] | next: [[C-04 Functions]] |
| ------------------------------- | ------------------------ |
