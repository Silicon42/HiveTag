// This function constructs a codeword in its base orientation from a marker ID number, typically only used
//  for generating markers but could theoretically be used to transmit data optically as marker IDs

// ID ordering is hierarchical where ID values for lower k values within a given rotationally independent
//  symbol bracket will have the same encoding for higher k values within the bracket. This would allows for
//  increasing the ID count after the fact (in exchange for check symbols) without already used IDs shuffling
//  their order or orientation, and for the possibility of using side-channel knowledge to increase the error
//  correcting capability of objects only tracked by lower rotationally dependent symbol bracket codewords.
//  This is only possible within a given rotationally independent symbol bracket because as soon as one gets
//  added to the rotationally independent region it would replace the one that would be the orientation
//  determiner, but wouldn't have the corresponding basis component.

//TODO: Consider where to put non-orientable codewords in terms of ID numbers and how to handle them.
//  Currently they are just assumed to be illegal and thus have no valid ID assigned but they need an ID
//  to be properly rejectable if something gets mis-corrected to one of them in the error-correction step

#include "gf16.h"
#include "ht_math_defs.h"

// by convention, the right most symbols are the ones that determine scale such that at tag generation
//  time, the selected factor can be applied via polynomial multiply with the id data that will be stored
//  as rotation independent data and at decode time, can be read back via masking without an additional
//  multiplicative inversion
const int32_t rotation_independent[] = {
	0x00001,
	0x000F1,
	0x00C51,
	0x0F591,
	0x11111
};

// by convention the left most symbols are the ones that determine the scale such that at decode time,
//  the symbol to scale a factor by will always be the highest symbol after subtracting pervious ones
//  and can be isolated with only a shift. Also this form is easier to calculate the basis vectors for
//  in a reasonable amount of time since they appear early in trying normal Reed-Solomon encodings
const gf16_poly rotation_dependent[] = {
	// n/3 == 1
	0x000010000700006,	// k == 2
	// n/3 == 2
	0x000140007F0006B,	// k == 2
	0x000010007900078,	// k == 3,4
	0x000000001200012,	// k == 5
	// n/3 == 3
	0x0012B007E4006CF,	// k == 2
	0x0001A00E9100E8B,	// k == 3,4
	0x000010011100110,	// k == 5
	0x000000016800168,	// k == 6,7
	0x000000001200012,	// k == 8
	// n/3 == 4
	0x017110767706166,	// k == 2
	0x0015D0499C048C1,	// k == 3,4
	0x000170B2150B202,	// k == 5
	0x000010A5630A562,	// k == 6,7
	0x000000152B0152B,	// k == 8
	0x000000016800168,	// k == 9,10
	0x000000001200012,	// k == 11		????
	// n/3 == 5
	0x19DFE7A5BC63842,	// k == 2
	0x014CCD9594D8158,	// k == 3,4
	0x001AAAB1B1AB01B,	// k == 5
	0x0001DF1C32F1C2F,	// k == 6,7
	0x00001814B2814B3,	// k == 8
	0x0000013F7F13F7F,	// k == 9,10
	0x000000152B0152B,	// k == 11
	0x000000016800168,	// k == 12,13
	0x000000001200012	// k == 14
};

gf16_poly id_to_codeword(int64_t id, int8_t nDiv3, int8_t k)
{
	--id;	//convert to 0 indexed
	//FIXME: add ID range check
	int8_t indep_syms = (k + 2) / 3;
	gf16_idx split_loc = indep_syms * GF16_SYM_SZ;
	int32_t indep_component = id & ((1L << split_loc) - 1);	// mask to isolate the rotationally independent data

	// encode the rotationally independent data in codeword form
	gf16_poly codeword = gf16_poly_mul(rotation_dependent[nDiv3 - indep_syms], indep_component);

	id >>= split_loc;	// down shift to isolate rotationally dependent data

	//FIXME: handle non-orientable codeword IDs
	// find which offset to use, the offset is what allows for hierarchical id ordering to work
	int8_t o_pos = 0;
	while(id < id_offsets[o_pos])
		++o_pos;

	id -= id_offsets[o_pos];	// remove the offset
	// o_pos is now the symbol index of the most significant non-zero ID data symbol
	// This symbol has limited range and special handling since it also encodes the orientation information
	gf16_idx orientor_loc = o_pos * GF16_SYM_SZ;
	uint32_t dep_data = id & ((1L << orientor_loc) - 1);	// mask to isolate the normally encoded ID data
	gf16_elem orientor = id >> orientor_loc;	// down shift to isolate the orientor symbol
	int8_t dep_basis_offset = (nDiv3 - 1);	// TODO: consider converting some settings to a struct or C++ class so that they don't need re-calculation
	dep_basis_offset *= dep_basis_offset;
	const gf16_poly* dep_basis = &rotation_dependent[dep_basis_offset];

	codeword ^= gf16_poly_scale(dep_basis[o_pos], gf16_exp[orientor]);

	for(int8_t i = 0; i < o_pos; ++i)
	{
		codeword ^= gf16_poly_scale(dep_basis[i], dep_data & GF16_MAX);
		dep_data >>= GF16_SYM_SZ;
	}

	return codeword;
}