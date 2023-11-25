#include "id_to_codeword.h"
//#include "codeword_to_id.h"
#include "rs_gf16.h"
#include "ht_math_defs.h"
#include <stdio.h>

int main()
{
	int8_t nDiv3, k, csym_cnt;
	nDiv3 = 2;
	k = 3;
	csym_cnt = nDiv3 * 3 - k;
	gf16_poly codeword;
	gf16_idx cw_sz = 2*THIRD_SIZE + nDiv3 * GF16_SYM_SZ;
	int16_t tx_pos = 0b110001100011;
	//int64_t id_returned;

	for(int id = 1; id < 1300; ++id)
	{
		codeword = id_to_codeword(id, nDiv3, k);
		if(rs16_get_errata(codeword, cw_sz, csym_cnt, 0, tx_pos))
			printf("id_to_codeword(%i, %i, %i) == 0x%llX\n", id, nDiv3, k, codeword);
		
	}
}