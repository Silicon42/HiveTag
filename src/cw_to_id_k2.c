// specialized version of codeword to ID conversion for k=2

#include "cw_to_id_k2.h"
#include "ht_math_defs.h"

struct oriented_id_k2 cw_to_id_k2(gf16_poly codeword, int8_t nDiv3)
{
	// handle data symbol stored on the rotationally independent portion
	struct oriented_id_k2 tag = {0,0};
	uint32_t third2 = codeword >> 2*THIRD_SIZE;
	uint32_t indep_component = (third2 ^ (codeword >> THIRD_SIZE) ^ codeword) & THIRD_MASK;
	tag.id = indep_component & GF16_MAX;
	third2 ^= indep_component;
	if(!third2)	// check if non-orientable
	{
		tag.id += 0x51;	// add offset
		return tag;
	}

	// handle data symbol stored on the rotationally dependent portion
	gf16_elem dep_component = third2 >> (nDiv3 - 1)*GF16_SYM_SZ;
	gf16_elem dep_data = gf16_log[dep_component];
	tag.orientation = dep_data / 5;
	tag.id |= (dep_data % 5) << GF16_SYM_SZ;

	return tag;
}