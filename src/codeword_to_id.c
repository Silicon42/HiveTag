// function that takes a properly arranged cyclic codeword and computes the ID number and orientation it was read in
// no input protection, assumes that errors have already been corrected and that symbols are distributed in even
//  thirds starting at symbol positions 0, 5, and 10, with 0 padding in unused symbols

#include "gf16.h"
#include "ht_math_defs.h"
#include "codeword_to_id.h"

// by convention, the right most symbols are the ones that determine scale such that at tag generation
//  time, the selected factor can be applied via polynomial multiply with the id data that will be stored
//  as rotation independent data and at decode time, can be read back via masking without an additional
//  multiplicative inversion
const int16_t rotation_indep[] = {
	0x0000,
	0x000F,
	0x00C5,
	0x0F59,
	0x1111
};

// a shrunk version of the table in id_to_codeword that works on the contracted and truncated form of the
//  codewords and drops the leading 1's in favor of improved 32-bit performance
const uint32_t rotation_dep[] = {
	// n/3 == 1
	0x70000000,	// k == 2
	// n/3 == 2
	0x47F00000,	// k == 2
	0x79000000,	// k == 3,4
	0x20000000,	// k == 5
	// n/3 == 3
	0x2B7E4000,	// k == 2
	0xAE910000,	// k == 3,4
	0x11100000,	// k == 5
	0x68000000,	// k == 6,7
	0x20000000,	// k == 8
	// n/3 == 4
	0x71176770,	// k == 2
	0x5D499C00,	// k == 3,4
	0x7B215000,	// k == 5
	0xA5630000,	// k == 6,7
	0x52B00000,	// k == 8
	0x68000000,	// k == 9,10
	0x20000000,	// k == 11
	// n/3 == 5
	0x9DFE7A5B,	// k == 2
	0x4CCD9590,	// k == 3,4
	0xAAAB1B00,	// k == 5
	0xDF1C3000,	// k == 6,7
	0x814B0000,	// k == 8
	0x37F00000,	// k == 9,10
	0x52000000,	// k == 11
	0x60000000,	// k == 12,13
	0x00000000	// k == 14
};

//returns the orientation on the stack and the ID via pass by ref
struct oriented_id codeword_to_id(gf16_poly codeword, int8_t nDiv3, int8_t k)
{
	uint32_t third0, third1, third2;
	third0 = codeword & THIRD_MASK;
	third1 = (codeword >> THIRD_SIZE) & THIRD_MASK;
	third2 = codeword >> 2*THIRD_SIZE;

	int32_t indep_component = third0 ^ third1 ^ third2;
	// remove the rotationally independent component from the codeword for later
	third0 ^= indep_component;
	third1 ^= indep_component;
	third2 ^= indep_component;

	int32_t indep_data = 0;	// accumulator for the rotation indepent data
	int8_t indep_syms = (k + 2)/3;	// the number of data symbols encoded in the sum of all 3 thirds
	int8_t dep_syms = k - indep_syms;
	
	gf16_idx indep_sz = 0;	// WARNING: because of potential early completion of the following while loop, this is not reliably the actual size in bits of the independent data segment
	int32_t indep_factor = rotation_indep[nDiv3 - indep_syms];	//which entry in the table should be used

	while(indep_component)	// while there is remaining cyclically independent component
	{
		indep_data |= (indep_component & GF16_MAX) << indep_sz;	// transfer the least significant symbol
		// cancel the effects of that symbol
		indep_component >>= GF16_SYM_SZ;
		indep_component ^= gf16_poly_scale(indep_factor, indep_data);
		indep_sz += GF16_SYM_SZ;	// increment shift amount
	}

	// at this point indep_data contains the rotationally independent portion of the data which is taken as the
	//  least significant portion of the ID by convention as the most significant portion will not be a power of 2
	//  due to having the maximum number of IDs roughly divided by 3 to encode orientation as well, this allows
	//  the resulting dep_data to be shifted and OR'ed together with the indep_data to recover the ID

	//int8_t dep_syms = k - indep_syms;
	int8_t dep_basis_offset = (nDiv3-1);	// TODO: consider converting some settings to a struct or C++ class so that they don't need re-calculation
	dep_basis_offset *= dep_basis_offset;
	const uint32_t* dep_basis = &rotation_dep[dep_basis_offset];
	struct oriented_id tag = {0,0};

	const gf16_idx third_tx_sz = nDiv3 * GF16_SYM_SZ;
	const gf16_idx third_tx_m1_sz = third_tx_sz - GF16_SYM_SZ;
	int8_t factors;

	// this code gets run either once or twice depending on if the read orientation is the base orientation
	//TODO: verify there is no way to simply correct the calculated ID rather than running again
	while(1)
	{
		int32_t dep_data = 0;
		uint32_t cw_shrunk = third1 << (32 - third_tx_sz);	// overall shift of third1 in cw_shrunk can be + or -, so must be done as 2 shifts
		cw_shrunk >>= third_tx_m1_sz;
		cw_shrunk |= third2 << (32 - third_tx_m1_sz);
		
		// preload dep_last, allows for full support of k == 14 while using uint32 for calculations
		gf16_elem dep_last = third2 >> third_tx_m1_sz;
		factors = 0;

		// scale the current basis vector by the last (most significant) symbol and subtract (XOR) it with the
		//  codeword to remove the effects of it continuing until the codeword is completely canceled out,
		//  the amount scaled by becomes the id data
		while(1)
		{
			cw_shrunk ^= gf16_poly_scale(dep_basis[factors], dep_last);
			if(!cw_shrunk)
				break;
			dep_data <<= GF16_SYM_SZ;
			dep_data |= dep_last;
			dep_last = cw_shrunk >> 7*GF16_SYM_SZ;
			cw_shrunk <<= GF16_SYM_SZ;
			++factors;
		}

		gf16_elem last_log = gf16_log[dep_last];
		if(last_log < 5)		// if base orientation or non-orientable, data is correct
		{
			if(dep_last)	// if last symbol isn't 0, ie not non-orientable
				tag.id = (int64_t)last_log << (factors * GF16_SYM_SZ);	// insert the log of the last symbol as the most significant symbol
			else	// else if it's non-orientable it gets tacked at the end of the ID range
				factors = dep_syms;
			
			tag.id |= dep_data;	// OR in the rest of the rotationally dependent data
			break;
		}/*
		else if(last_log >= 10)	// unfortunately which log value indicates a +120 vs +240 degree rotation swaps and I haven't yet determined the pattern
		{
			third1 = third2;
			third2 = third0;
			tag.orientation = 2;
		}*/
		else
		{
			int32_t temp = third2;
			third2 = third1;
			third1 = third0;
			third0 = temp;
			tag.orientation += 1;
		}
	}

	// orientation and rotation dependent data has been extracted, now all that is left is to fix offsets
	//  and combine the rotationally dependent and independent parts
	
	// unfortunately, since orientation data is always stored on the last non-zero rotationally dependent
	//  symbol to be extracted, there isn't really a nice purely mathematical way to convert between
	//  contiguous integer values (IDs) and codewords if there are any more than 1 rotationally dependent
	//  data symbol which would hard limit the ID count to 80. This occurs because which symbol determines
	//  the orientation can change which in turn effects the allowed range of that symbol and how it gets
	//  interepreted, but it is easily fixed with conditional logic. That being said if you know that you
	//  will never need more than 80 (+16 non-orientable) IDs (*2 if using color inversion) then you should
	//  use the specialized version for that and it will have faster decoding and less memory usage, and
	//  if you instead know you need no more than 21760 (+256 non-orientable) IDs (*2 if using color inversion)
	//  you should use the pre-baked version that is faster and has comparable memory usage
	// TODO: make speciallized versions


	// add ID offset for when the the last symbol was not the maximum of the independent portion, this allows
	//  ID values to be contiguous and non-orientable IDs will always be the very last ones and so can be 
	//  easily discarded with single compare
	//NOTE: there might be a faster way to calculate this
	tag.id += id_offsets[factors];
	// possible alternative:
	// tag.id += 0x55555555 & ((1L << factors*GF16_SYM_SZ) - 1)
	
	tag.id <<= indep_syms*GF16_SYM_SZ;	// shift to have room for the rotationally independent data
	tag.id |= indep_data;	// OR in the rotationally independent data
	++tag.id;	// IDs are 1 indexed such that color inverted markers can easily occupy the negative IDs
	return tag;
}