#ifndef CODEWORD_TO_ID_H
#define CODEWORD_TO_ID_H

#include "gf16.h"

int8_t codeword_to_id(gf16_poly codeword, int8_t nDiv3, int8_t k, int64_t* id);

#endif //CODEWORD_TO_ID_H
