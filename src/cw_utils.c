#include "cw_utils.h"
#include "ht_math_defs.h"

// strips padding symbols to convert to contracted form
gf16_poly contract_cw(gf16_poly codeword, int8_t nDiv3)
{
	int8_t shift = nDiv3 * GF16_SYM_SZ;
	gf16_poly contracted = codeword & THIRD_MASK;
	//TODO: check if theres a more efficient way to compute the mask and shifts
	contracted |= ((codeword >> THIRD_SIZE) & THIRD_MASK) << shift;
	contracted |= ((codeword >> 2*THIRD_SIZE)) << 2*shift;
	return contracted;
}

// re-adds padding symbols to convert to full form
gf16_poly expand_cw(gf16_poly contracted, int8_t nDiv3)
{
	gf16_poly third_mask = (1LL << nDiv3*GF16_SYM_SZ) - 1;
	int8_t shift = nDiv3 * GF16_SYM_SZ;
	gf16_poly codeword = contracted & third_mask;
	codeword |= ((contracted >> shift) & third_mask) << THIRD_SIZE;
	codeword |= ((contracted >> 2*shift) & third_mask) << 2*THIRD_SIZE;
	return codeword;
}

