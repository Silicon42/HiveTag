// This is a test program used for debugging the id_to_codeword() and codeword_to_id() functions to make sure
//  that encoding an ID results in a valid Reed-Solomon codeword and that encoding and decoding an ID in all
//  orientations results in the correct ID and orientation being returned

#include "id_to_codeword.h"
#include "codeword_to_id.h"
#include "rs_gf16.h"
#include "ht_math_defs.h"
#include <stdio.h>

gf16_poly cw_rotate(gf16_poly codeword)
{
	return ((THIRD_MASK & codeword) << 2*THIRD_SIZE) | (codeword >> THIRD_SIZE);
}

void cw_to_id_check(gf16_poly codeword, int8_t nDiv3, int8_t k, int64_t id, int8_t orientation)
{
	struct oriented_id id_returned;
	id_returned = codeword_to_id(codeword, nDiv3, k);
	if(id_returned.id != id)
		printf("ID mismatch %lli != %lli %i\n", id, id_returned.id, orientation);
	if(id_returned.orientation != orientation)
		printf("Orientation mismatch %i != %i at %lli\n", orientation, id_returned.orientation, id);
}

int main()
{
	int8_t nDiv3, k, csym_cnt;
	nDiv3 = 2;
	k = 3;
	csym_cnt = nDiv3 * 3 - k;
	gf16_poly codeword;
	gf16_idx cw_sz = 2*THIRD_SIZE + nDiv3 * GF16_SYM_SZ;
	int16_t tx_pos = 0b110001100011;
	//struct oriented_id id_returned;

	for(int id = 81; id <= 1376; ++id)
	{
		codeword = id_to_codeword(id, nDiv3, k);
		// nothing should print here if all codewords generated are valid Reed-Solomon codewords
		if(rs16_get_errata(codeword, cw_sz, csym_cnt, 0, tx_pos))
			printf("Invalid RS: id_to_codeword(%i, %i, %i) == 0x%llX\n", id, nDiv3, k, codeword);

		cw_to_id_check(codeword, nDiv3, k, id, 0);
		if(id > 1360)
			continue;
		codeword = cw_rotate(codeword);
		cw_to_id_check(codeword, nDiv3, k, id, 1);
		codeword = cw_rotate(codeword);
		cw_to_id_check(codeword, nDiv3, k, id, 2);
	}
	printf("Done.\n");
}