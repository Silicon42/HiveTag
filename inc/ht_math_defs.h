#ifndef HT_MATH_DEFS_H
#define HT_MATH_DEFS_H

#define THIRD_MASK 0xFFFFF
#define THIRD_SYMS 5
#define THIRD_SIZE (THIRD_SYMS*4)
#define THIRD_TRIPLER 0x10000100001

#include <stdint.h>

// possible alternative w/o lookup:
// 0x55555555 & ((1L << factors*GF16_SYM_SZ) - 1)
// TODO: check if there is any benefit to this ^
extern const int32_t id_offsets[9];

#endif //HT_MATH_DEFS_H