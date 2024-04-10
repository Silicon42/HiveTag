// I don't know if "basis vector" is actually the technically correct term for these but each codeword is
//  composed of linear combinations of a list for given n value of the code and uses the first 
//  k - ceiling(k/3) entries of that list for the rotation dependent portion and the polynomial product of
//  the data and entry n/3 - cieling(k/3) in the rotation independent table

// This program is the successor to rs_codeword_test in that I used what I learned by looking for patterns
//  in the outputs there to systematically generate just the constants I needed here

#include "rs_gf16.h"
#include "ht_math_defs.h"
#include <stdio.h>

// constants specific to n and k combinations for which scaling by the value of a given position in a normally
//  encoded Reed-Solomon codeword forces that position to 0 and swaps the check symbol position to a transmitted
//  but otherwise unused term earlier in the codeword. Obtained via gen_chk_sym_shifter.c with more explanation
//  there.
const gf16_poly csym_mover1[] = {
	0xD0013A,	0xBB001E9,
	0x901CBB,	0x2B016DE,	0x77D01F8C,	0xA00217016BD,
	0x517DA5,	0x2E16B7B,	0x8A213D92,	0xDF141E8A3,	0xF0957312282,	0xCE06DB91A674
};

const gf16_poly csym_mover2[] = {
	0,			0xD4010FD,
	0,			0xE110CF9,	0x13610F25,	0xD00E9B10776,
	0,			0,			0,			0,				0,				0xC314686035E1
};

gf16_poly encode_cyclic(gf16_poly data, int8_t nDiv3, int8_t k)
{
	int8_t num_chk_syms = nDiv3 * 3 - k;
/*
	gf16_poly raw_data = 0;// data << (nDiv3 + 8 - num_chk_syms)*GF16_SYM_SZ;
	int32_t tx_third_mask = (1 << (nDiv3 * GF16_SYM_SZ)) -1;
	int8_t third_cnt = 0;
	while(k >= nDiv3)	//handles data that fills a full third, executes at most twice
	{
		k -= nDiv3;
		raw_data |= (data >> (k*GF16_SYM_SZ)) & tx_third_mask;
		raw_data <<= THIRD_SIZE;
		++third_cnt;
	}
	raw_data |= (data << ((nDiv3 - k)*GF16_SYM_SZ)) & tx_third_mask;
	while (++third_cnt < 3)
		raw_data <<= THIRD_SIZE;
	raw_data >>= num_chk_syms*GF16_SYM_SZ;
*/
	// generate the cyclic codeword by first encoding and then adding the linearly scaled factor corresponding
	//  to holding any terms that must be padding at zero and thus pushing the effective check symbol position
	//  backwards to an otherwise unused data position
	gf16_poly codeword = rs16_encode_systematic(data, num_chk_syms);
	if(nDiv3 < 5 && num_chk_syms > nDiv3)
	{
		int bit_idx = nDiv3*GF16_SYM_SZ;
		int csm_idx = (nDiv3-2)*(nDiv3-2) + num_chk_syms - 3;
		codeword ^= gf16_poly_scale(csym_mover1[csm_idx], (codeword >> bit_idx) & GF16_MAX);
		// for many values there isn't a valid 2nd check symbol to move but extra logic here doesn't make sense
		//  for what is essentially a program I should only need a handful of times so instead the second array
		//  just has 0 padding for those values which should result in no change
		bit_idx = nDiv3 < 4 ? bit_idx + GF16_SYM_SZ : 9 * GF16_SYM_SZ;
		codeword ^= gf16_poly_scale(csym_mover2[csm_idx], (codeword >> bit_idx) & GF16_MAX);
	}

	return codeword;
}

int main()
{
	printf("Rotationally independent factors:\n");
	printf("0x00001,\n");	// n/3 == 1 manually because I don't want to deal with separating input nibbles
	for(int8_t nDiv3 = 2; nDiv3 <= 5; ++nDiv3)
	{
		// why not use single symbol data? because I did't calculate symbol shifting constants for most of those
		//  so instead here this ugly mess of recycled code
		int32_t factor;

		gf16_poly data = 1 << 2*(5 - nDiv3)*GF16_SYM_SZ;
		gf16_poly codeword = encode_cyclic(data, nDiv3, 2);

		factor = ((codeword >> (2*THIRD_SIZE)) ^ (codeword >> THIRD_SIZE) ^ codeword) & THIRD_MASK;

		factor = gf16_poly_scale(factor, gf16_inverse(factor & GF16_MAX));
		
		printf("0x%05X,\n", factor);
	}

	printf("\nRotationally dependent factors:");
	for(int8_t nDiv3 = 1; nDiv3 <= 5; ++nDiv3)
	{
		int8_t syms_last = -1;
		gf16_poly tx_third_mask = ~(-1LL << (nDiv3 * GF16_SYM_SZ));

		printf("\n// n/3 == %i:", nDiv3);
		for(int8_t k = 2; k < 3*nDiv3; ++k)
		{
			int8_t dep_syms = k - (k + 2)/3;
			if(dep_syms == syms_last)
			{	// handle the stutter where a symbol gets added to the independent portion instead
				printf(",%i", k);
				continue;
			}
			syms_last = dep_syms;
			int8_t num_chk_syms = nDiv3 * 3 - k;
			gf16_poly codeword;
			
			int64_t i_bound = (int64_t)1 << (k*GF16_SYM_SZ);
			for(int64_t i = 1; i < i_bound; ++i)
			{
				gf16_poly raw_data = 0;
				int k_copy = k;
				int third_cnt = 0;
				while(k_copy >= nDiv3)	//handles data that fills a full third, executes at most twice
				{
					k_copy -= nDiv3;
					raw_data |= (i >> (k_copy*GF16_SYM_SZ)) & tx_third_mask;
					raw_data <<= THIRD_SIZE;
					++third_cnt;
				}
				raw_data |= (i << ((nDiv3 - k_copy)*GF16_SYM_SZ)) & tx_third_mask;
				while (++third_cnt < 3)
					raw_data <<= THIRD_SIZE;
				raw_data >>= num_chk_syms*GF16_SYM_SZ;

				codeword = encode_cyclic(raw_data, nDiv3, k);

				int32_t indep_factor = (codeword ^ (codeword >> THIRD_SIZE) ^ (codeword >> 2*THIRD_SIZE)) & THIRD_MASK;
				codeword ^= 0x10000100001 * indep_factor;

				int32_t third1, third2;
				third1 = (codeword >> THIRD_SIZE) & THIRD_MASK;
				third2 = codeword >> 2*THIRD_SIZE;
				int64_t cw_shrunk = ((int64_t)third2 << nDiv3*GF16_SYM_SZ) | third1;
				cw_shrunk >>= (nDiv3*2 - dep_syms)*GF16_SYM_SZ;
				if(cw_shrunk == 1)
					break;
			}

			printf("\n0x%015llX,	// k == %i", codeword, k);
		}
	}
	printf("\n");//*/
}