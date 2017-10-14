#ifndef __COMMON_H__
#define __COMMON_H__

#ifdef NULL
#undef NULL
#endif
#define NULL    ((void *)0)

#define ZEROPAD 1               // Pad with zero
#define SIGN    2               // Unsigned/signed long
#define PLUS    4               // Show plus
#define SPACE   8               // Space if plus
#define LEFT    16              // Left justified
#define SPECIAL 64              // 0x
#define SMALL	32
#define LARGE   64              // Use 'ABCDEF' instead of 'abcdef'
#define EOF (-1)
#define TAB_SIZE 4

#define SWAP(a, b)      do { (a) ^= (b); (b) ^= (a); (a) ^= (b); } while (0)
#define ISDIGIT(c)      ((c) >= '0' && (c) <= '9')
#define __do_div(n, base) ({ \
int __res; \
__res = ((unsigned long) n) % (unsigned) base; \
n = ((unsigned long) n) / (unsigned) base; \
__res; })

#define public
#define private static

/* width of stack == width of int */
#define STACKITEM   int

/* round up width of objects pushed on stack. The expression before the
& ensures that we get 0 for objects of size 0. */
#define VA_SIZE(TYPE)                   \
    ((sizeof(TYPE) + sizeof(STACKITEM) - 1) \
        & ~(sizeof(STACKITEM) - 1))

/* &(LASTARG) points to the LEFTMOST argument of the function call
(before the ...) */
#define va_start(AP, LASTARG)   \
    (AP=((va_list)&(LASTARG) + VA_SIZE(LASTARG)))

/* nothing for va_end */
#define va_end(AP)

#define va_arg(AP, TYPE)    \
    (AP += VA_SIZE(TYPE), *((TYPE *)(AP - VA_SIZE(TYPE))))

typedef unsigned int addr_t;
/* standard size_t type */
typedef unsigned size_t;

typedef unsigned long long uint64_t;
typedef long long int64_t;
typedef unsigned int uint32_t;
typedef int int32_t;
typedef unsigned short uint16_t;
typedef short int16_t;
typedef unsigned char uint8_t;
typedef char int8_t;

typedef	uint64_t u64;
typedef	uint32_t u32;
typedef	uint16_t u16;
typedef	uint8_t u8;

typedef unsigned char *va_list;


typedef enum {
    false = 0,
    true
} bool;

public int atoi(const char *str);
public char *itoa(int value, char *str, int base);
public int skip_atoi(const char **s);

public int logdisp(const char *format, ...);

#endif
