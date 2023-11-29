// This is only meant to calculate gf16_poly shim values whose scaling effectively
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

// NOTE2: n = 3, k = 1 results in repeat codes however for completeness sake the the
//  corresponding values will still be calculated.

// NOTE3: the values here aren't actually used anywhere in the final product, they are only
//  used to help me understand which codes are valid in terms of the input space, this is
//  because the constraint that they be cyclically unique and capable of having the IDs be
//  calculated back from the data within isn't here at this step.

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
	gf16_poly to_move_mask = GF16_MAX << GF16_SYM_SZ;
	int chk_syms;
	int to_move_bits;
	gf16_poly diff;
	gf16_elem scale_factor;

	for(int nDiv3 = 1; nDiv3 < 5; ++nDiv3)
	{
		chk_syms = nDiv3 + 1;
		normal_enc = rs16_encode_systematic(raw, chk_syms);
		to_move_bits = THIRD_BITS - GF16_SYM_SZ * chk_syms;	//how many bits to shift the check symbol by

		//brute force attempt to find what value makes the term to be moved 0
		for(int64_t i = 1; i < 16; ++i)
		{
			fixed_enc = rs16_encode_systematic(raw | (i << to_move_bits), chk_syms);
			if(!(fixed_enc & to_move_mask))
				break;
		}
		diff = normal_enc ^ fixed_enc;
		scale_factor = gf16_inverse((diff >> (nDiv3 * GF16_SYM_SZ)) & GF16_MAX);
		diff = gf16_poly_scale(diff, scale_factor);
		printf("To shift 1 check symbol for n = %i, scale 0x%llX by term %i\n", nDiv3 * 3, diff, nDiv3);
		raw >>= GF16_SYM_SZ;
		to_move_mask <<= GF16_SYM_SZ;	//where to shift the symbol from

	}

	printf("\n");	//last case gets duplicated to help check correctness of calculation

	// up to 4 symbols can move at size n = 12 without increasing the difficulty because it's really
	//  only one vacating a position that should be padding, the other 3 are already in the next third
	to_move_mask = 0xF0000;
	raw = 0x10;
	to_move_bits = 0;
	for(chk_syms = 5; chk_syms <= 9; ++chk_syms)
	{
		if(chk_syms == 8)	// at 8 check symbols, the input value must skip over the padding gap
			raw <<= GF16_SYM_SZ;
		if(chk_syms == 9)	// at 9 check symbols, where the 9th will be relocated to must skip the padding gap
			to_move_bits += GF16_SYM_SZ;

		normal_enc = rs16_encode_systematic(raw, chk_syms);

		//brute force attempt to find what value makes the term to be moved 0
		for(int64_t i = 1; i < 16; ++i)
		{
			fixed_enc = rs16_encode_systematic(raw | (i << to_move_bits), chk_syms);
			if(!(fixed_enc & to_move_mask))
				break;
		}
		diff = normal_enc ^ fixed_enc;
		scale_factor = gf16_inverse((diff >> (THIRD_BITS-GF16_SYM_SZ)) & GF16_MAX);
		diff = gf16_poly_scale(diff, scale_factor);
		printf("To shift %i check symbols for n = 12, scale 0x%llX by term 4\n", chk_syms - 4, diff);
	}

	// now it gets hard because moving more than one symbol causes the effects of both to interfere with
	//  each other so it's no longer a simple one time scale and add fix but the combined operation of both

	printf("\nn = 6, k = 2:\n");
	chk_syms = 4;
	to_move_mask = 0xFF00;
	to_move_bits = GF16_SYM_SZ;
	for(int64_t j = 1; j < 256; ++j)
	{
		raw = j << (6 * GF16_SYM_SZ);
		normal_enc = rs16_encode_systematic(raw, chk_syms);

		// we want only the effects of moving one of the terms, and since we have to mill through
		//  the posibilites, it might as well be one that doesn't require scaling, that way we only
		//  get one of each type. If it's not either of those, try another one. Here it's well behaved
		//  and easily separable.
		if(!((normal_enc & to_move_mask) == 0x1000 || (normal_enc & to_move_mask) == 0x0100))
			continue;
		//brute force attempt to find what values make the terms to be moved 0
		for(int64_t i = 1; i < 256; ++i)
		{
			fixed_enc = rs16_encode_systematic(raw | (i << to_move_bits), chk_syms);
			if(!(fixed_enc & to_move_mask))
				break;
		}

		printf("0x%llX * Term %i\n", fixed_enc ^ normal_enc, (normal_enc & 0x0F00) ? 2 : 3 );
	}

	// following comment block is for n=6, k=1 but is too complicated for me to work out for something
	//  that results in a glorified repeat code
/*
	printf("\nn = 6, k = 1:\n");
	chk_syms = 5;
	to_move_mask = 0xFFF00;
	to_move_bits = 12;
	for(int64_t j = 1; j < 16; ++j)
	{
		raw = j << (6 * GF16_SYM_SZ);
		normal_enc = rs16_encode_systematic(raw, chk_syms);
		printf("normal: 0x%012llX	", normal_enc);

		// we want only the effects of moving one of the terms, and since we have to mill through
		//  the posibilites, it might as well be one that doesn't require scaling, that way we only
		//  get one of each type. If it's not either of those, try another one.
		//if(!((normal_enc & to_move_mask) == 0x10000 || (normal_enc & to_move_mask) == 0x01000 || (normal_enc & to_move_mask) == 0x00100))
		//	continue;
		//brute force attempt to find what values make the terms to be moved 0
		for(int64_t i = 1; i < 4096; ++i)
		{
			fixed_enc = rs16_encode_systematic(raw | ((i & 0xF00) << to_move_bits) | (i & 0x0FF), chk_syms);
			if(!(fixed_enc & to_move_mask))
				break;
		}

		//int x = 4;
		//if(normal_enc & 0x00F00)
		//	x = 2;
		//else if (normal_enc & 0x0F000)
		//	x = 3;
		//printf("fixed: 0x%012llX	diff: 0x%llX\n", fixed_enc, fixed_enc ^ normal_enc);
	}
*/

	printf("\nn = 9, k = 4:\n");
	chk_syms = 5;
	to_move_mask = 0xFF000;
	to_move_bits = 0;
	int once_A = 0;
	int once_B = 0;
	for(int64_t j = 1; j < 65536; ++j)
	{
		raw = ((j & 0xF) << (2 * GF16_SYM_SZ)) | ((j & 0xFFF0) << (4 * GF16_SYM_SZ));
		normal_enc = rs16_encode_systematic(raw, chk_syms);

		// we want only the effects of moving one of the terms, and since we have to mill through
		//  the posibilites, it might as well be one that doesn't require scaling. If it's not 
		//  either of those, try another one. Here it's well behaved and easily separable.
		if(!((normal_enc & to_move_mask) == 0x10000 || (normal_enc & to_move_mask) == 0x01000))
			continue;
		//brute force attempt to find what values make the terms to be moved 0
		for(int64_t i = 1; i < 256; ++i)
		{
			fixed_enc = rs16_encode_systematic(raw | (i << to_move_bits), chk_syms);
			if(!(fixed_enc & to_move_mask))
				break;
		}

		if(normal_enc & 0x0F000)
		{
			if(once_A)
				continue;
			else
				++once_A;
		}
		else
		{
			if(once_B)
				continue;
			else
				++once_B;
		}

		printf("0x%llX * Term %i\n", fixed_enc ^ normal_enc, (normal_enc & 0x0F000) ? 3 : 4 );
	}

	printf("\nn = 9, k = 3:\n");
	chk_syms = 6;
	to_move_mask = 0xFF000;
	to_move_bits = 0;
	once_A = 0;
	once_B = 0;
	for(int64_t j = 1; j < 4096; ++j)
	{
		raw = j << (4 * GF16_SYM_SZ);
		normal_enc = rs16_encode_systematic(raw, chk_syms);

		// we want only the effects of moving one of the terms, and since we have to mill through
		//  the posibilites, it might as well be one that doesn't require scaling. If it's not 
		//  either of those, try another one. Here it's well behaved and easily separable.
		if(!((normal_enc & to_move_mask) == 0x10000 || (normal_enc & to_move_mask) == 0x01000))
			continue;
		//brute force attempt to find what values make the terms to be moved 0
		for(int64_t i = 1; i < 256; ++i)
		{
			fixed_enc = rs16_encode_systematic(raw | (i << to_move_bits), chk_syms);
			if(!(fixed_enc & to_move_mask))
				break;
		}

		if(normal_enc & 0x0F000)
		{
			if(once_A)
				continue;
			else
				++once_A;
		}
		else
		{
			if(once_B)
				continue;
			else
				++once_B;
		}

		printf("0x%llX * Term %i\n", fixed_enc ^ normal_enc, (normal_enc & 0x0F000) ? 3 : 4 );
	}

	printf("\nn = 9, k = 2:\n");
	chk_syms = 7;
	to_move_mask = 0xFF000;
	to_move_bits = 2*GF16_SYM_SZ;
	once_A = 0;
	once_B = 0;
	for(int64_t j = 1; j < 256; ++j)
	{
		raw = j << (4 * GF16_SYM_SZ);
		normal_enc = rs16_encode_systematic(raw, chk_syms);

		// we want only the effects of moving one of the terms, and since we have to mill through
		//  the posibilites, it might as well be one that doesn't require scaling. If it's not 
		//  either of those, try another one. Here it's well behaved and easily separable.
		if(!((normal_enc & to_move_mask) == 0x10000 || (normal_enc & to_move_mask) == 0x01000))
			continue;
		//brute force attempt to find what values make the terms to be moved 0
		for(int64_t i = 1; i < 256; ++i)
		{
			fixed_enc = rs16_encode_systematic(raw | ((i & 0xF0) << to_move_bits) | (i & 0xF), chk_syms);
			if(!(fixed_enc & to_move_mask))
				break;
		}

		if(normal_enc & 0x0F000)
		{
			if(once_A)
				continue;
			else
				++once_A;
		}
		else
		{
			if(once_B)
				continue;
			else
				++once_B;
		}

		printf("0x%llX * Term %i\n", fixed_enc ^ normal_enc, (normal_enc & 0x0F000) ? 3 : 4 );
	}

	// following comment block is for n=9, k=1 but is too complicated for me to work out for something
	//  that results in a glorified repeat code
/*
	printf("\nn = 9, k = 1:\n");
	chk_syms = 8;
	to_move_mask = 0xFF000;
	to_move_bits = 2*GF16_SYM_SZ;
	for(int64_t j = 1; j < 16; ++j)
	{
		raw = j << (4 * GF16_SYM_SZ);
		normal_enc = rs16_encode_systematic(raw, chk_syms);
		printf("normal: 0x%012llX	", normal_enc);

		// we want only the effects of moving one of the terms, and since we have to mill through
		//  the posibilites, it might as well be one that doesn't require scaling, that way we only
		//  get one of each type. If it's not either of those, try another one.
		//if(!((normal_enc & to_move_mask) == 0x10000 || (normal_enc & to_move_mask) == 0x01000 || (normal_enc & to_move_mask) == 0x00100))
		//	continue;
		//brute force attempt to find what values make the terms to be moved 0
		for(int64_t i = 1; i < 256; ++i)
		{
			fixed_enc = rs16_encode_systematic(raw | (i << to_move_bits), chk_syms);
			if(!(fixed_enc & to_move_mask))
				break;
		}

		//int x = 4;
		//if(normal_enc & 0x00F00)
		//	x = 2;
		//else if (normal_enc & 0x0F000)
		//	x = 3;
		printf("fixed: 0x%012llX	diff: 0x%llX\n", fixed_enc, fixed_enc ^ normal_enc);
	}
*/

	printf("\nn = 12, k = 2:\n");
	chk_syms = 10;
	to_move_mask = 0xF0000F0000;
	to_move_bits = 0;
	once_A = 0;
	once_B = 0;
	for(int64_t j = 1; j < 256; ++j)
	{
		raw = j << (2 * GF16_SYM_SZ);
		normal_enc = rs16_encode_systematic(raw, chk_syms);

		// we want only the effects of moving one of the terms, and since we have to mill through
		//  the posibilites, it might as well be one that doesn't require scaling. If it's not 
		//  either of those, try another one. Here it's well behaved and easily separable.
		if(!((normal_enc & to_move_mask) == 0x10000 || (normal_enc & to_move_mask) == 0x1000000000))
			continue;
		//brute force attempt to find what values make the terms to be moved 0
		for(int64_t i = 1; i < 256; ++i)
		{
			fixed_enc = rs16_encode_systematic(raw | i, chk_syms);
			if(!(fixed_enc & to_move_mask))
				break;
		}

		if(normal_enc & 0xF0000)
		{
			if(once_A)
				continue;
			else
				++once_A;
		}
		else
		{
			if(once_B)
				continue;
			else
				++once_B;
		}

		printf("0x%llX * Term %i\n", fixed_enc ^ normal_enc, (normal_enc & 0xF0000) ? 4 : 9 );
	}

}