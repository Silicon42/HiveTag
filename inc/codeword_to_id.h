#ifndef CODEWORD_TO_ID_H
#define CODEWORD_TO_ID_H

#include "gf16.h"

struct oriented_id
{
	int64_t id;
	int8_t orientation;	// 0: base orientation, 1: +120 degrees, 2: +240 degrees
};

struct oriented_id codeword_to_id(gf16_poly codeword, int8_t nDiv3, int8_t k);

#endif //CODEWORD_TO_ID_H
