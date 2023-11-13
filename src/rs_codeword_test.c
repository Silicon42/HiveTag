// This is a test program to help me understand how to algorithmically convert between an ID and
//  orientation, and the encoded reed-solomon data and vice versa such that no actual dictionary
//  of codewords is ever required

#include "rs_gf16.h"
#include <stdio.h>

#define THIRD_MASK 0xFFFFF
#define TWO_THIRD_MASK 0xFFFFFFFFFF
#define THIRD_SYMS 5
#define THIRD_SIZE THIRD_SYMS*GF16_SYM_SZ

int main()
{
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

	int nDiv3, k;

	while(1)
	{
		printf("Select a valid 'n/3' value (1 <= n <= 5): ");
		nDiv3 = getchar();
		getchar();	// eat the '\n'
		nDiv3 -= '0';
		if(1 <= nDiv3 && nDiv3 <= 5)
			break;
	}
	const int n = 3*nDiv3;
	if(n > 3)
	{
		while(1)
		{
			printf("Select a valid 'k' value in hex (2 <= k <= %X): ", n-1);
			k = getchar();
			getchar();	// eat the '\n'
			//quick and dirty hex to integer conversion, '@' and '`' get misread as 9
			k |= 0x20;
			k -= '0';
			if(k > 9)
				k -= 39;
			if(1 < k && k < n)
				break;
		}
	}
	else
		k = 2;	//only valid k value at that size

	//mask that represents the valid transmit region for the cyclic version of the RS code for given n
	const gf16_poly tx_third_mask = ~((uint64_t)-1 << (n/3 * GF16_SYM_SZ));
	//const gf16_poly tx_mask = tx_third_mask | tx_third_mask << (THIRD_SIZE) | tx_third_mask << (2*THIRD_SIZE);

	const uint64_t i_bound = (uint64_t)1 << (k*GF16_SYM_SZ);
	const int num_chk_syms = n - k;

	char format_cw[] = "CW: %00X/%00X/%00X";
	format_cw[6] = format_cw[11] = format_cw[16] = '0' + nDiv3;
	char format_sums[] = " SUM2: %00X/%00X SUM3: %00X\n";
	format_sums[9] = format_sums[14] = format_sums[25] = '0' + nDiv3;

	char filename[16];
	sprintf(filename, "n%i_k%i.txt", n, k);
	FILE* output_fp = fopen(filename, "w");
	uint32_t non_orientable_cnt = 0;
	for (uint64_t i = 0; i < i_bound; ++i)
	{
		// convert loop index to raw data to be entered to Reed-Solomon encoder, injecting padding where needed
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
		
		// generate the cyclic codeword by first encoding and then adding the linearly scaled factor corresponding
		//  to holding any terms that must be padding at zero and thus pushing the effective check symbol position
		//  backwards to an otherwise unused data position
		gf16_poly codeword = rs16_encode(raw_data, num_chk_syms);
		if(nDiv3 < 5 && num_chk_syms > nDiv3)
		{
			int bit_idx = nDiv3*GF16_SYM_SZ;
			int csm_idx = (nDiv3-2)*(nDiv3-2) + num_chk_syms - 3;
			codeword ^= gf16_poly_scale(csym_mover1[csm_idx], (codeword >> bit_idx) & GF16_MAX);
			// for many values there isn't a valid 2nd check symbol to move but extra logic here doesn't make sense
			//  for what is essentially a program I should only need a handful of times so instead the second array
			//  just has 0 padding for those values which should result in no change
			bit_idx = nDiv3 < 5 ? bit_idx + GF16_SYM_SZ : 9 * GF16_SYM_SZ;
			codeword ^= gf16_poly_scale(csym_mover2[csm_idx], (codeword >> bit_idx) & GF16_MAX);
		}
		
		if((codeword & TWO_THIRD_MASK) == codeword >> THIRD_SIZE)
		{
			fprintf(output_fp, " N/O ");
			++non_orientable_cnt;
		}

		uint32_t third0, third1, third2;
		third0 = codeword & THIRD_MASK;
		third1 = (codeword >> THIRD_SIZE) & THIRD_MASK;
		third2 = codeword >> (2*THIRD_SIZE);
		fprintf(output_fp, format_cw, third2, third1, third0);
		//fprintf(output_fp, "CW: %013llX", codeword);

		gf16_elem sum2r = third0 ^ third1;
		gf16_elem sum2l = third2 ^ third1;
		gf16_elem sum3 = sum2r ^ third2;
		fprintf(output_fp, format_sums, sum2l, sum2r, sum3);
	}
	fclose(output_fp);

	printf("Found %i non-orientable codewords.\n", non_orientable_cnt);

	return 0;
}