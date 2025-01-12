/*
 * This file is part of muforth: https://muforth.nimblemachines.com/
 *
 * Copyright (c) 2002-2022 David Frech. (Read the LICENSE for details.)
 */

#include <sys/types.h>
#include <stdint.h>
#include <string.h>

#include "env.h"

/* Support for putting 32-bit values into cell (64-bit) sized containers -
 * for support of 64-bit muforth on 32-bit platforms.
 */
#ifdef MU_ADDR_32

#ifdef MU_LITTLE_ENDIAN
/* LE 32 */
#define CELL_T(type)    struct { type value; uintptr_t padding; }
#define CELL(value)     { value, 0 }

#else
/* BE 32 */
#define CELL_T(type)    struct { uintptr_t padding; type value; }
#define CELL(value)     { 0, value }

#endif

#define _(cell)         (cell).value
#define _STAR(pcell)    (pcell)->value

#else  /* 64-bit */

/* These are no-ops for 64-bit. */
#define CELL_T(type)    type
#define CELL(value)     (value)
#define _(cell)         (cell)
#define _STAR(pcell)    *(pcell)

#endif

/* Every kind of cell - address, data, stack - is 64 bits! */
typedef  int64_t   cell;
typedef uint64_t  ucell;

/* address type - for casting */
typedef uintptr_t  addr;   /* intptr_t and uintptr_t are integer types that
                             are the same size as a native pointer. Whether
                             this type is unsigned or not will affect how
                             32-bit addresses are treated when pushed onto
                             a 64-bit stack: signed will sign-extend,
                             unsigned will zero-extend. */

/* pointer and cell types */
typedef void (*code)(void);         /* POINTER to word's machine code */
typedef CELL_T(code)  code_cell;    /* CELL wrapper of code */
typedef code_cell    *xt;           /* POINTER to code; aka "execution token" */
typedef CELL_T(xt)    xt_cell;      /* CELL wrapper of xt */

/* Forth VM execution registers */
extern cell     *SP;    /* parameter stack pointer */
extern cell     *RP;    /* return stack pointer */
extern xt_cell  *IP;    /* instruction pointer */
extern xt        W;     /* on entry, points to the current Forth word */

/* dictionary size */
/* Cells are 64 bits. Let's allocate 1M cells - 8MB of heap. */
#define DICT_CELLS     (1024 * 1024)

/* data and return stacks */
/* NOTE: Even on 32-bit platforms the R stack is 64 bits wide! This makes
 * things _much_ simpler, at the expense of a bit more storage. */
extern cell dstack[];
extern cell rstack[];

#define STACK_SIZE  4096
#define STACK_SAFETY  32

/* Stack bottoms */
#define SP0   &dstack[STACK_SIZE - STACK_SAFETY]
#define RP0   &rstack[STACK_SIZE - STACK_SAFETY]

/* Data stack */
#define TOP   ST0
#define ST0   SP[0]
#define ST1   SP[1]
#define ST2   SP[2]
#define ST3   SP[3]

#define DROP(n)     (SP += (n))
#define PUSH(v)     (*--SP = (cell)(v))
#define PUSH_ADDR(v)    PUSH((addr)(v))
#define POP         (*SP++)

/* Return stack */
#define RTOP        (*RP)
#define RDROP(n)    (RP += (n))
#define RPUSH(n)    (*--RP = (cell)(n))
#define RPOP        (*RP++)

#define ALIGN_SIZE  sizeof(cell)
#define ALIGNED(x)  (((intptr_t)(x) + ALIGN_SIZE - 1) & -ALIGN_SIZE)

/*
 * struct string is a "normal" string: pointer to the first character, and
 * length.
 */
struct string
{
    char *data;
    size_t length;
};

extern int parsed_lineno;       /* captured with first character of token */
extern struct string parsed;    /* last token parsed */

/* declare common functions */

/* public.h is automagically generated, and can match every public function
 * taking no arguments. Other functions need to be put here explicitly.
 */
#include "public.h"

/* error.c */
void die(const char *zmsg);
void abort_zmsg(const char *zmsg);
void assert(int cond, const char *zmsg);

/* file.c */
char *string_copy(char *dest, char *src);
char *concat_paths(char *dest, size_t destsize, char *p1, char *p2);

/* Utility macros */
#define MIN(a,b)    (((a) < (b)) ? (a) : (b))
