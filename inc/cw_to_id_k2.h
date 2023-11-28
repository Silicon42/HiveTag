#ifndef CW_TO_ID_K2
#define CW_TO_ID_K2

#include "gf16.h"

struct oriented_id_k2
{
	int8_t id;
	int8_t orientation;	// 0: base orientation, 1: +120 degrees, 2: +240 degrees
};

struct oriented_id_k2 cw_to_id_k2(gf16_poly codeword, int8_t nDiv3);

#endif //CW_TO_ID_K2