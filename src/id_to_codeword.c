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

//NOTE: IDs could be restructured as combinations of 2 separate numbers where one would be a "family" number
//  such that a family corresponds to a lower k value code with the rotation independent component used as a
//  mask, this would allow one to have significantly better error correction if you knew that the marker being
//  identified belonged to a specific family and range, as good as if you were using the lower k value, and if
//  for example multiple users agreed to use different families ahead of time, both could later be used together
//  in exchange for the otherwise improved error correction, however this will likely not be implemented anytime
//  soon since it would require an overhaul of how IDs are represented, since they wouldn't be as easy to
//  consider as a single contiguous set of numbers and the non-orientable codes would have to be moved to the
//  bottom of the range to work correctly and 1 indexing the IDs definitely wouldn't work that way

#include "gf16.h"
#include "ht_math_defs.h"

// by convention, the right most symbols are the ones that determine scale such that at tag generation
//  time, the selected factor can be applied via polynomial multiply with the id data that will be stored
//  as rotation independent data and at decode time, can be read back via masking without an additional
//  multiplicative inversion
const gf16_poly rotation_independent[] = {
	0x000010000100001,
	0x000F1000F1000F1,
	0x00C5100C5100C51,
	0x0F5910F5910F591,
	0x111111111111111
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
	0x000000001200012,	// k == 11
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

// normal IDs are always positive, inverted color IDs must be negated first before passing
gf16_poly id_to_codeword(int64_t id, int8_t nDiv3, int8_t k)
{
	//FIXME: add upper ID range check, requires calculation
	if(id <= 0)
		return -1;
	
	--id;	//convert to 0 indexed
	int8_t indep_syms = (k + 2) / 3;
	int8_t dep_syms = k - indep_syms;
	gf16_idx split_loc = indep_syms * GF16_SYM_SZ;
	int32_t indep_component = id & ((1L << split_loc) - 1);	// mask to isolate the rotationally independent data

	// encode the rotationally independent data in codeword form
	gf16_poly codeword = gf16_poly_mul(rotation_independent[nDiv3 - indep_syms], indep_component);

	id >>= split_loc;	// down shift to isolate rotationally dependent data

	// find which offset to use, the offset is what allows for contiguous ID ordering to work
	int8_t o_pos = 0;
	while(id >= id_offsets[++o_pos]);

	--o_pos;

	//TODO: move most of this and other large explainer blocks to documentation with a reference to it.
	// check if the ID is for a non-orientable codeword. They have no rotationally dependent component and look
	//  the same in all 3 possible orientations and so cannot provide a full pose solution on their own without
	//  some form of side-channel information. As such they are always tacked on at the end of the ID range
	//  because they are the least desirable to use and should only be used as a last resort if all preceding
	//  IDs have already been used. THEIR IDs ARE NOT STATIC with increasing k values as they occupy the position
	//  of the next set to be added normally. I chose not to put them in the negative range to allow for color
	//  inversion to be natively supported to allow for relatively cheap doubling of the available IDs with IDs
	//  of color inverted markers being negative. I chose not put them in the extreme high range of the int64
	//  type to allow for systems that know they are using a limited range to use smaller data types to store
	//  the IDs
	if(o_pos >= dep_syms)	// technically this should only ever be o_pos == dep_syms at most but safety first
		return codeword;

	id -= id_offsets[o_pos];	// remove the offset
	// o_pos is now the symbol index of the most significant non-zero ID data symbol
	// This symbol has limited range and special handling since it also encodes the orientation information
	gf16_idx orientor_loc = o_pos * GF16_SYM_SZ;
	uint32_t dep_data = id & ((1L << orientor_loc) - 1);	// mask to isolate the normally encoded ID data
	gf16_elem orientor = id >> orientor_loc;	// down shift to isolate the orientor symbol
	int8_t dep_basis_offset = (nDiv3 - 1);		// TODO: consider converting some settings to a struct or C++ class so that they don't need re-calculation
	dep_basis_offset *= dep_basis_offset;
	const gf16_poly* dep_basis = &rotation_dependent[dep_basis_offset];

	codeword ^= gf16_poly_scale(dep_basis[o_pos], gf16_exp[orientor]);

	gf16_idx shift_sz = o_pos * GF16_SYM_SZ;
	for(int8_t i = 0; i < o_pos; ++i)
	{
		shift_sz -= GF16_SYM_SZ;
		gf16_elem scalar = (dep_data >> shift_sz) & GF16_MAX;
		codeword ^= gf16_poly_scale(dep_basis[i], scalar);
		//dep_data >>= GF16_SYM_SZ;
	}

	return codeword;
}