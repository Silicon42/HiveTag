// generates the lookup tables that the k=3 and k=4 specialized codeword to ID functions use to speed up computations
#include "id_to_codeword.h"
#include "ht_math_defs.h"
#include <stdio.h>

int main()
{
	uint8_t cw_to_id_k3_k4_LUT [256] = {0};
	int8_t nDiv3 = 2;
	for(int i = 0; i <= 0x55; ++i)
	{
		int8_t id = (i << GF16_SYM_SZ) + 1;
		gf16_poly codeword = id_to_codeword(id, nDiv3, 3);
		cw_to_id_k3_k4_LUT[(uint8_t)codeword] = i * 3;
		cw_to_id_k3_k4_LUT[(uint8_t)(codeword >> THIRD_SIZE)] = i * 3 - 2;
		cw_to_id_k3_k4_LUT[(uint8_t)(codeword >> 2*THIRD_SIZE)] = i * 3 - 1;
	}
	
	for(int i = 0; i < 256; ++i)
		printf("[0x%02X]: IDpart:%02lX orientation:%i\n", i, (cw_to_id_k3_k4_LUT[i] + 2L)/3, cw_to_id_k3_k4_LUT[i] % 3);
}