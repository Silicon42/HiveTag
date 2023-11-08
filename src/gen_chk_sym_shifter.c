// This is only meant to calculate a Lookup Table of gf16_poly's whose scaling effectively
//  shifts Reed-Solomon check symbols back across a region of padding to keep their cyclic
//  properties of a factor of the given field while shortening the code length.

// In this case the field is GF(16) and the cycle period that I'm using is 3 from the
//  original cycle length of 15. In this way codes can be created with n = 3, 6, 9, and 12
//  with k < n * 2/3 that have cyclic properties (period 3), are shorter than full length,
//  can be easily decoded with a standard Reed-Solomon decoding algorithm, and have high
//  resiliency to errors and erasures.

// All cases of n = 15 are trivial since they use the full Reed-Solomon number of symbols.

// k >= n * 2/3 is trivial in that it only involves evenly spacing the remaing symbols out
//  with padding and then encoding and vice versa for decoding, eg if we represent data
//  symbols with 'd', check symbols with 'c', and padding symbols with '0', then for n = 3,
//  k = 2, dd encodes as ddc by passing 0000d0000d0000 to a standard Reed-Solomon encoder
//  and receiving back from it 0000d0000d0000c. Another example would be for n = 6, k = 5,
//  ddddd encodes as dddddc by passing 000dd000dd000d to the encoder which returns
//  000dd000dd000dc

// NOTE: The strings of padding must be identical, however from my testing they don't
//  effect the encoding and so may as well be 0 and this program therefore assumes this.

// NOTE2: k = n * 1/3 results in repeat codes however for completeness sake the the
//  corresponding values will still be calculated.

#include <stdio.h>
#include "rs_gf16.h"

#define THIRD_MASK 0xFFFFF
#define TWO_THIRD_MASK 0xFFFFFFFFFF
#define THIRD_BITS 20

int main()
{
	gf16_poly raw = 0x100000000;
	gf16_poly normal_enc = 0;
	gf16_poly fixed_enc = 0;
	gf16_poly to_move_mask = GF16_MAX;

	for(int n_over_3 = 1; n_over_3 < 5; ++n_over_3)
	{
		int chk_syms = n_over_3 + 1;
		normal_enc = rs16_encode(raw, chk_syms);
		to_move_mask <<= GF16_SYM_SZ;	//where to shift the symbol from
		int to_move_bits = THIRD_BITS - GF16_SYM_SZ * chk_syms;	//how many bits to shift the check symbol by

		//brute force attempt to find what value makes the term to be moved 0
		for(int64_t i = 1; i < 16; ++i)
		{
			fixed_enc = rs16_encode(raw | (i << to_move_bits), chk_syms);
			if(!(fixed_enc & to_move_mask))
				break;
		}
		gf16_poly diff = normal_enc ^ fixed_enc;
		gf16_elem scale_factor = gf16_inverse((diff >> (n_over_3 * GF16_SYM_SZ)) & GF16_MAX);
		diff = gf16_poly_scale(diff, scale_factor);
		printf("To shift 1 check symbol for n = %i, scale 0x%llX by term %i\n", n_over_3 * 3, diff, n_over_3);
		raw >>= GF16_SYM_SZ;
	}
}