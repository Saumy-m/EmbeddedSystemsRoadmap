> [!info] 
> title: Structs, Unions, Enums 
> tags: [C, struct, union, enum, typedef] 
> chapter: 7

| previous: [[C-06 Pointers]] | next: [[C-08 Preprocessor & Macros]] |
| --------------------------- | ------------------------------------ |
## 1. Why

- Group related data under one name.
- `struct` → many fields, all present.
- `union` → many views, one storage.
- `enum` → named integer constants.
- `typedef` → short names for types.

---

## 2. Struct Basics

- Collection of members. Types can differ.
- Members sit in memory in declaration order.
- Access with `.` (dot).

```c
#include <stdio.h>

struct Point {
    int x;
    int y;
};

int main(void) {
    struct Point p;
    p.x = 3;
    p.y = 4;
    printf("(%d, %d)\n", p.x, p.y);
    return 0;
}
```

### Initialization

```c
#include <stdio.h>

struct Sensor {
    int   id;
    float value;
    char  name[8];
};

int main(void) {
    struct Sensor a = {1, 25.5f, "temp"};              // positional
    struct Sensor b = {.name = "hum", .id = 2};        // designated (C99)
    struct Sensor c = {0};                             // all zero

    printf("%d %.1f %s\n", a.id, a.value, a.name);
    printf("%d %.1f %s\n", b.id, b.value, b.name);     // value = 0
    printf("%d\n", c.id);
    return 0;
}
```

- Missing members → zero.
- Designated init → order free. Easier to read. Safe when fields change.
- Prefer designated init for config structs.

---

## 3. `typedef`

- Gives a type a new name.
- No more writing `struct` each time.

```c
#include <stdio.h>

typedef struct {
    int x;
    int y;
} Point;

int main(void) {
    Point p = {1, 2};
    printf("%d %d\n", p.x, p.y);
    return 0;
}
```

Other uses:

```c
#include <stdio.h>
#include <stdint.h>

typedef uint8_t  byte_t;
typedef uint32_t reg_t;
typedef void (*Callback)(int);

void hello(int n) { printf("hello %d\n", n); }

int main(void) {
    byte_t b = 200;
    reg_t  r = 0xFF;
    Callback cb = hello;
    cb((int)b + (int)r);
    return 0;
}
```

Tips:

- `typedef` gives a name. It does not create a new type.
- Do **not** hide pointers: `typedef int *IntPtr` confuses `const`.
- Embedded style: `Name_t` or `PascalCase`. Be consistent.

### Struct tag + typedef (needed for self-reference)

```c
typedef struct Node {
    int val;
    struct Node *next;       // tag needed here
} Node;
```

---

## 4. Pointers to Structs

- Use `->` through a pointer.
- `p->x` is the same as `(*p).x`.

```c
#include <stdio.h>

typedef struct { int x, y; } Point;

void move(Point *p, int dx, int dy) {
    p->x += dx;
    p->y += dy;
}

int main(void) {
    Point a = {1, 2};
    Point *pa = &a;

    move(pa, 10, 20);
    printf("%d %d\n", a.x, pa->y);          // 11 22
    printf("%d\n", (*pa).x);                // 11
    return 0;
}
```

---

## 5. Structs and Functions

- Pass by value → whole struct is copied.
- Pass by pointer → only an address.
- Large struct → pass `const T *`.

```c
#include <stdio.h>

typedef struct {
    int  id;
    char name[16];
    int  scores[4];
} Student;

void print_value(Student s) {                // copies ~36 bytes
    printf("%s\n", s.name);
}

void print_ptr(const Student *s) {           // copies only an address
    printf("%s %d\n", s->name, s->scores[0]);
}

int main(void) {
    Student s = {7, "Asha", {90, 80, 70, 60}};
    print_value(s);
    print_ptr(&s);
    return 0;
}
```

Returning a struct is fine for small ones:

```c
#include <stdio.h>

typedef struct { int x, y; } Point;

Point add(Point a, Point b) {
    return (Point){a.x + b.x, a.y + b.y};     // compound literal (C99)
}

int main(void) {
    Point p = add((Point){1, 2}, (Point){10, 20});
    printf("%d %d\n", p.x, p.y);              // 11 22
    return 0;
}
```

---

## 6. Copy and Compare

- `b = a` copies **all** members. Arrays inside are copied too.
- Pointers inside are copied as addresses. Shallow.
- `a == b` is **not allowed**.
- `memcmp` on structs is risky. Padding bytes are unspecified.

```c
#include <stdio.h>
#include <string.h>

typedef struct { int id; char tag[4]; } Item;

int main(void) {
    Item a = {1, "ab"};
    Item b = a;                      // full copy
    b.tag[0] = 'X';

    printf("%s %s\n", a.tag, b.tag);   // ab Xb

    // compare field by field
    int same = (a.id == b.id) && (strcmp(a.tag, b.tag) == 0);
    printf("%d\n", same);              // 0
    return 0;
}
```

Shallow copy trap:

```c
#include <stdio.h>

typedef struct { int *data; } Box;

int main(void) {
    int v = 10;
    Box a = {&v};
    Box b = a;            // both point to the same int
    *b.data = 99;
    printf("%d\n", *a.data);   // 99
    return 0;
}
```

---

## 7. Arrays of Structs and Nested Structs

```c
#include <stdio.h>

typedef struct { int x, y; } Point;
typedef struct { Point tl, br; } Rect;     // nested

typedef struct { const char *name; int age; } Person;

int main(void) {
    Rect r = {{0, 0}, {10, 5}};
    printf("w=%d h=%d\n", r.br.x - r.tl.x, r.br.y - r.tl.y);

    Person team[] = {
        {"Ana", 30},
        {"Raj", 25},
        {"Mei", 28},
    };
    int n = sizeof team / sizeof team[0];

    for (int i = 0; i < n; i++)
        printf("%s %d\n", team[i].name, team[i].age);
    return 0;
}
```

---

## 8. Memory Layout: Padding and Alignment

- CPU likes data at addresses divisible by its size.
- Compiler inserts **padding** bytes.
- Rule:
    1. Each member aligned to its own size.
    2. Struct size is a multiple of its largest alignment.
- Order members large → small to save space.

```c
#include <stdio.h>
#include <stddef.h>

struct A { char a; int b; char c; };     // padded
struct B { int b; char a; char c; };     // better order

int main(void) {
    printf("A: %zu\n", sizeof(struct A));    // 12
    printf("B: %zu\n", sizeof(struct B));    // 8

    printf("offset a=%zu b=%zu c=%zu\n",
           offsetof(struct A, a),
           offsetof(struct A, b),
           offsetof(struct A, c));            // 0 4 8
    return 0;
}
```

Picture of `struct A` (4-byte int):

```text
byte: 0  1 2 3  4 5 6 7  8  9 10 11
      a  . . .  b b b b  c  . .  .
         pad               pad
```

- `offsetof` → byte offset of a member. Header `<stddef.h>`.
- `sizeof` of a struct may be larger than the sum of its members.

### Packed structs

- Remove padding. Compiler extension.
- GCC/Clang: `__attribute__((packed))`.
- Access may be **unaligned** → slow, or a fault on Cortex-M0.
- Never take the address of a packed member and pass it around.

```c
#include <stdio.h>
#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint8_t  type;
    uint32_t value;
    uint8_t  flags;
} Packed;

int main(void) {
    printf("%zu\n", sizeof(Packed));    // 6
    return 0;
}
```

### Check sizes at compile time

- `_Static_assert` (C11) stops the build if a layout changes.

```c
#include <stdint.h>

typedef struct {
    uint8_t  id;
    uint8_t  len;
    uint16_t crc;
} Header;

_Static_assert(sizeof(Header) == 4, "Header must be 4 bytes");

int main(void) { return 0; }
```

---

## 9. Struct Members: Special Cases

### Struct with function pointers (driver "ops" table)

- C way of doing interfaces.

```c
#include <stdio.h>

typedef struct {
    const char *name;
    int  (*init)(void);
    int  (*read)(void);
} SensorOps;

static int temp_init(void) { printf("temp init\n"); return 0; }
static int temp_read(void) { return 25; }
static int hum_init(void)  { printf("hum init\n");  return 0; }
static int hum_read(void)  { return 60; }

static const SensorOps sensors[] = {
    {"temp", temp_init, temp_read},
    {"hum",  hum_init,  hum_read},
};

int main(void) {
    for (int i = 0; i < 2; i++) {
        sensors[i].init();
        printf("%s = %d\n", sensors[i].name, sensors[i].read());
    }
    return 0;
}
```

### Flexible array member (C99)

- Last member, no size.
- Allocate struct + payload in one block.

```c
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct {
    size_t  len;
    uint8_t data[];          // flexible array member
} Packet;

int main(void) {
    const uint8_t src[] = {1, 2, 3, 4};

    Packet *p = malloc(sizeof *p + sizeof src);
    if (p == NULL) return 1;

    p->len = sizeof src;
    memcpy(p->data, src, p->len);

    for (size_t i = 0; i < p->len; i++) printf("%u ", p->data[i]);
    printf("\n");

    free(p);
    return 0;
}
```

- `sizeof(Packet)` does not include `data`.
- Needs the heap. Many firmware projects use a fixed `data[MAX]` instead.

### Anonymous struct / union members (C11)

```c
#include <stdio.h>

typedef struct {
    int kind;
    union {
        int   i;
        float f;
    };                         // access as s.i or s.f
} Value;

int main(void) {
    Value v = {.kind = 0, .i = 42};
    printf("%d\n", v.i);
    return 0;
}
```

---

## 10. Bit-Fields

- Pack small fields into bits.
- Syntax: `type name : width;`
- Use `unsigned int` (or `uint32_t`).

```c
#include <stdio.h>

struct Status {
    unsigned int ready : 1;
    unsigned int error : 1;
    unsigned int mode  : 3;
    unsigned int       : 3;      // unnamed padding
    unsigned int count : 8;
};

int main(void) {
    struct Status s = {0};
    s.ready = 1;
    s.mode  = 5;
    s.count = 200;

    printf("%u %u %u\n", s.ready, s.mode, s.count);
    printf("size: %zu\n", sizeof s);                // usually 4

    s.mode = 9;           // 9 needs 4 bits. Truncated to 1.
    printf("%u\n", s.mode);
    return 0;
}
```

> [!warning] Bit-field layout is implementation-defined Bit order inside a word depends on compiler and endianness. Cannot take their address (`&s.ready`). Not safe for wire formats or portable register maps. Prefer shifts and masks for protocols. Fine for private flags.

---

## 11. Unions

- All members share the same memory.
- Size = largest member.
- Only one member is valid at a time (by your logic).
- Reading a different member than the last written = reinterpreting bytes.

```c
#include <stdio.h>
#include <stdint.h>

typedef union {
    uint32_t u;
    float    f;
    uint8_t  b[4];
} Pun;

int main(void) {
    Pun x;
    x.f = 1.0f;

    printf("size = %zu\n", sizeof x);                       // 4
    printf("bits = 0x%08X\n", x.u);                         // 0x3F800000
    printf("bytes: %02X %02X %02X %02X\n",
           x.b[0], x.b[1], x.b[2], x.b[3]);                 // 00 00 80 3F (LE)
    return 0;
}
```

- Type punning through a union is allowed in C99/C11.
- Not allowed that way in C++. Use `memcpy` there.

### Tagged union (variant)

- Store a **tag** saying which member is valid.
- Standard pattern for messages and events.

```c
#include <stdio.h>

typedef enum { EV_BUTTON, EV_TEMP, EV_MSG } EventType;

typedef struct {
    EventType type;
    union {
        int   button_id;
        float temp_c;
        char  msg[8];
    } data;
} Event;

void handle(const Event *e) {
    switch (e->type) {
        case EV_BUTTON: printf("button %d\n", e->data.button_id); break;
        case EV_TEMP:   printf("temp %.1f\n", e->data.temp_c);    break;
        case EV_MSG:    printf("msg %s\n", e->data.msg);          break;
        default:        break;
    }
}

int main(void) {
    Event a = {.type = EV_BUTTON, .data.button_id = 3};
    Event b = {.type = EV_TEMP,   .data.temp_c = 36.6f};
    handle(&a);
    handle(&b);
    return 0;
}
```

### Register view: whole word + fields

```c
#include <stdio.h>
#include <stdint.h>

typedef union {
    uint32_t raw;
    struct {
        uint32_t en   : 1;
        uint32_t mode : 2;
        uint32_t      : 5;
        uint32_t div  : 8;
        uint32_t      : 16;
    } bits;
} Ctrl;

int main(void) {
    Ctrl c = {0};
    c.bits.mode = 2;
    c.bits.div  = 10;
    c.bits.en   = 1;
    printf("raw = 0x%08X\n", c.raw);       // 0x00000A05 on GCC little-endian
    return 0;
}
```

- Handy for reading. Layout is compiler-specific.
- Vendor headers (CMSIS) use masks and shifts instead for portability.

---

## 12. Enums

- Named integer constants.
- Default values start at 0 and go up by 1.
- Can set any value.

```c
#include <stdio.h>

enum Color { RED, GREEN = 5, BLUE };     // 0, 5, 6

int main(void) {
    enum Color c = BLUE;
    printf("%d %d %d\n", RED, GREEN, c);     // 0 5 6
    printf("size: %zu\n", sizeof c);         // usually 4
    return 0;
}
```

- Constants have type `int`.
- Enum variable stores any value the compiler lets it. Not checked.
- No scope: names go into the surrounding scope. Prefix them.

```c
// enum A { OK };
// enum B { OK };      // error: OK already used
```

### With `typedef` and prefix

```c
#include <stdio.h>

typedef enum {
    STATE_IDLE,
    STATE_RUN,
    STATE_ERROR,
    STATE_COUNT          // number of states
} State;

int main(void) {
    State s = STATE_RUN;
    printf("%d of %d\n", s, STATE_COUNT);
    return 0;
}
```

### Enum → string table

- Designated initializers keep it in sync.

```c
#include <stdio.h>

typedef enum { STATE_IDLE, STATE_RUN, STATE_ERROR, STATE_COUNT } State;

static const char *const state_str[STATE_COUNT] = {
    [STATE_IDLE]  = "IDLE",
    [STATE_RUN]   = "RUN",
    [STATE_ERROR] = "ERROR",
};

const char *state_name(State s) {
    return (s >= 0 && s < STATE_COUNT) ? state_str[s] : "?";
}

int main(void) {
    printf("%s %s\n", state_name(STATE_RUN), state_name((State)9));   // RUN ?
    return 0;
}
```

### Bit flags

```c
#include <stdio.h>

enum {
    FLAG_RX   = 1U << 0,
    FLAG_TX   = 1U << 1,
    FLAG_ERR  = 1U << 2,
    FLAG_BUSY = 1U << 3,
};

int main(void) {
    unsigned flags = 0;

    flags |= FLAG_RX | FLAG_BUSY;
    flags &= ~FLAG_BUSY;
    flags ^= FLAG_ERR;

    if (flags & FLAG_RX)  printf("rx\n");
    if (flags & FLAG_ERR) printf("err\n");
    printf("0x%X\n", flags);               // 0x5
    return 0;
}
```

- Store flags in `unsigned`, not the enum type.

### Enum vs `#define`

||`enum`|`#define`|
|---|---|---|
|Type|`int`|none (text)|
|Debugger sees name|yes|no|
|Scope|normal|file onward|
|Good for|related constants, states|single values, strings, types|

> [!note] C23 Enums can have a fixed underlying type: `enum State : uint8_t { ... };` Before that, size is compiler-chosen. GCC option `-fshort-enums` shrinks them. For packets, store as `uint8_t` and cast.

---

## 13. State Machine: Enum + Table

```c
#include <stdio.h>

typedef enum { S_IDLE, S_RUN, S_DONE, S_COUNT } State;
typedef enum { E_START, E_FINISH, E_RESET, E_COUNT } Event;

static const State next[S_COUNT][E_COUNT] = {
    /*              START   FINISH  RESET  */
    /* IDLE */   { S_RUN,   S_IDLE, S_IDLE },
    /* RUN  */   { S_RUN,   S_DONE, S_IDLE },
    /* DONE */   { S_DONE,  S_DONE, S_IDLE },
};

static const char *const names[S_COUNT] = {"IDLE", "RUN", "DONE"};

int main(void) {
    State s = S_IDLE;
    Event script[] = {E_START, E_FINISH, E_START, E_RESET};

    for (int i = 0; i < 4; i++) {
        s = next[s][script[i]];
        printf("-> %s\n", names[s]);
    }
    return 0;
}
```

- Table instead of nested `switch`.
- Add a state or event → one row or column.
- `const` table lives in Flash.

---

## 14. Embedded Patterns

### Register block overlay

- Recap from [[C-06 Pointers]].

```c
// typedef struct {
//     volatile uint32_t CR;
//     volatile uint32_t SR;
//     volatile uint32_t DR;
// } UART_TypeDef;
//
// #define UART1 ((UART_TypeDef *)0x40011000UL)
// UART1->CR |= 1U;
```

### Packet with explicit byte order

- Do **not** cast a byte buffer to a struct.
- Padding, alignment, and endianness break it.
- Write fields byte by byte.

```c
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint8_t  cmd;
    uint16_t addr;
    uint16_t value;
} Msg;

size_t msg_encode(const Msg *m, uint8_t *out) {
    size_t i = 0;
    out[i++] = 0xAA;                       // start byte
    out[i++] = m->cmd;
    out[i++] = (uint8_t)(m->addr  >> 8);   // big-endian on the wire
    out[i++] = (uint8_t)(m->addr  & 0xFF);
    out[i++] = (uint8_t)(m->value >> 8);
    out[i++] = (uint8_t)(m->value & 0xFF);
    return i;
}

int main(void) {
    Msg m = {.cmd = 0x02, .addr = 0x1234, .value = 0xBEEF};
    uint8_t buf[8];
    size_t n = msg_encode(&m, buf);

    for (size_t i = 0; i < n; i++) printf("%02X ", buf[i]);
    printf("\n");                          // AA 02 12 34 BE EF
    return 0;
}
```

### Constant config in Flash

```c
#include <stdio.h>
#include <stdint.h>

typedef struct {
    uint32_t baud;
    uint8_t  data_bits;
    uint8_t  stop_bits;
    uint8_t  parity;
} UartConfig;

static const UartConfig uart_cfg = {
    .baud = 115200,
    .data_bits = 8,
    .stop_bits = 1,
    .parity = 0,
};

int main(void) {
    printf("%u %u\n", uart_cfg.baud, uart_cfg.data_bits);
    return 0;
}
```

### Opaque type (information hiding)

> [!note] Multi-file. Not for Code Runner.

`uart.h`

```text
typedef struct Uart Uart;          // fields hidden

Uart *uart_open(int port);
int   uart_write(Uart *u, const void *buf, size_t n);
```

`uart.c`

```text
struct Uart { int port; int tx_count; /* ... */ };
```

- Users cannot touch internals.
- Change fields without breaking callers.

### Other tips

- Group related state into one struct. Pass its pointer.
- Do not pass large structs by value in ISRs or deep call chains.
- Use designated init so adding a field does not break old code.
- Put `volatile` on members shared with ISRs.
- Zero-init config structs: `= {0}`.
- Order fields large → small. Check with `_Static_assert`.
- Do not `memcmp` structs with padding. Compare fields.
- Zero the whole struct with `memset` before sending raw bytes. Clears padding.

---

## 15. Common Mistakes

- Forgetting the `;` after `struct { ... }`.
- Using `.` on a pointer. Using `->` on a value.
- Comparing structs with `==`.
- `memcmp` on structs with padding.
- Assuming `sizeof(struct)` = sum of members.
- Casting a byte buffer to a struct pointer.
- Packed struct member address passed to a function.
- Bit-fields in wire protocols.
- Bit-field value too big for its width.
- Reading a different union member than the written one without meaning to.
- No tag in a tagged union.
- Enum names clashing across enums.
- Using enum value out of range. Not checked.
- Using enum as a bit-flag variable type.
- Shallow copy of a struct holding pointers.
- Forgetting `struct` tag for self-referencing node.
- Returning a pointer to a local struct.
- Big struct passed by value repeatedly.

---

## 16. Embedded Cheat Sheet

- `typedef struct` for everything. Add a tag if self-referencing.
- Designated initializers for config.
- `const` structs → Flash.
- `->` through pointers. Pass `const T *` for inputs.
- Order members large → small.
- `_Static_assert(sizeof(T) == N, ...)` for layouts.
- Never cast raw buffers to structs. Serialize fields.
- Bit-fields only for private flags. Masks for hardware and wire formats.
- Union for byte views and variants. Always carry a tag.
- Enums for states, errors, commands. Prefix the names. Add `_COUNT`.
- Flags → `1U << n`, stored in `unsigned` / `uint32_t`.
- Table-driven state machines over deep `switch`.
- `volatile` on fields touched by ISRs or hardware.
- Opaque types for drivers.

---

## 17. Practice

1. Define `Student {id, name, marks}`. Print three students.
2. Write `Point` helpers: `add`, `dist2`, `print`. Pass by pointer.
3. Sort an array of structs by a field using `qsort`.
4. Print `sizeof` and `offsetof` of a struct with `char`, `int`, `double`. Reorder to shrink it.
5. Add `_Static_assert` for your struct size.
6. Make a packed struct. Print its size. Explain the unaligned risk.
7. Show a shallow-copy bug with a pointer member.
8. Compare two structs correctly. Write `equal(a, b)`.
9. Define bit-field flags. Set, clear, and print them.
10. Overflow a bit-field on purpose. Observe truncation.
11. Print the bytes of a `float` using a union.
12. Write a tagged union `Event`. Handle it in a `switch`.
13. Write a `Ctrl` register union. Print `raw` after setting fields.
14. Make an `enum` of error codes. Write `err_str(code)` with a string table.
15. Define bit-flag enums. Test set, clear, toggle, check.
16. Build a traffic-light table-driven state machine.
17. Write a sensor driver "ops" struct with function pointers. Register 2 sensors.
18. Encode and decode a message with explicit big-endian bytes.
19. Simulate a UART register block in RAM. Write `uart_send_byte` that waits on a status bit.
20. Build a linked list of `Person` structs. Insert, find, remove.
21. Create a flexible-array `Packet`. Allocate, fill, free.
22. Keep a `static const` config struct. Print it.
23. Write an opaque `Counter` type in `counter.h` / `counter.c`.
24. Explain why `memcmp(&a, &b, sizeof a)` can lie.

---

## Quick Revision

- `struct` → all members exist. `.` on value, `->` on pointer.
- `typedef` names a type. Add a tag for self-reference.
- Assignment copies a struct. `==` is not allowed. Compare fields.
- Padding is inserted for alignment. `sizeof` may exceed the sum. Order large → small.
- `offsetof` for member offsets. `_Static_assert` to lock layouts.
- Packed removes padding. Risk: unaligned access.
- Bit-fields: compact, layout is implementation-defined. Not for wire formats.
- `union` → shared storage. Size = largest member. Use a tag.
- `enum` → named ints. Prefix names. `_COUNT` for the last. Not range-checked.
- Flags: `1U << n` with `|`, `&`, `~`, `^`.
- Packets: serialize bytes explicitly. Do not cast.
- Tables of structs and function pointers replace large `switch` blocks.

| previous: [[C-06 Pointers]] | next: [[C-08 Preprocessor & Macros]] |
| --------------------------- | ------------------------------------ |

