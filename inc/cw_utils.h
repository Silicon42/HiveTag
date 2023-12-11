#ifndef CW_UTILS_H
#define CW_UTILS_H

#include "gf16.h"

// strips padding symbols to convert to contracted form
gf16_poly contract_cw(gf16_poly codeword, int8_t nDiv3);

// re-adds padding symbols to convert to full form
gf16_poly expand_cw(gf16_poly codeword, int8_t nDiv3);

// rotates the codeword 0, 120 or 240 degrees ccw based on pos
//gf16_poly rotate_cw(gf16_poly codeword, int8_t nDiv3, int8_t pos);

#endif //CW_UTILS_H