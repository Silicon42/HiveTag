
#include "rs_gf16.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define THIRD_MASK 0xFFFFF
#define TWO_THIRD_MASK 0xFFFFFFFFFF
#define THIRD_SIZE 20

int main()
{
	// I wanted to have the program read the base svg from a file but from the below and how much time I spent
	//  writing it, that seems to be too much added complexity compared to just including it as a string literal
	/*
	FILE* marker_fp = fopen(MARKER_BASE_SVG, "r");
	if (marker_fp == NULL)
	{
		perror("Couldn't open " MARKER_BASE_SVG ". Is it missing?\n");
		return 1;
	}
	
	char marker_base[1024] = {0};
	// read the whole marker base into a c string
	size_t read = fread(marker_base, 1, sizeof(marker_base) - 34, marker_fp);	//34 is the number of decimal characters to fully specify 2 doubles
	if (!feof(marker_fp))	// if we didn't reach the end of the file,
	{
		if (ferror(marker_fp))	// then an error occured while reading it
		{
			perror("Error while reading " MARKER_BASE_SVG ".\n");
			fclose(marker_fp);
			return 2;
		}
		// or the file was too long to fit in the buffer, this shouldn't happen unless marker_base.svg is modified
		perror(MARKER_BASE_SVG " was too long. Recompile with a larger buffer and try again.\n");
		fclose(marker_fp);
		return 3;
	}
	fclose(marker_fp);

	// find the start of the footer section
	int footer_i;
	do
	{
		// seek backward from file end to find the last closing g tag "</g>" candidate
		//  8 is the minimum number of characters the 'g' could be from the end of a valid svg file
		for (footer_i = read - 8; marker_base[footer_i] != 'g'; --footer_i);
		footer_i -= 2;
	} while (strncmp(&marker_base[footer_i], "</g>", 4));	// check that it actually is a closing "</g>"
	// continue seeking backward to the next newline
	for (--footer_i; marker_base[footer_i] != '\n'; --footer_i);
	++footer_i;

	int footer_len = footer_i - read;
	for(int i = 0; i >= 0; ++i)
	{
		marker_base[sizeof(marker_base) - j]
	}
	*/
	/*
	int8_t syms_per_third = 1;	// valid values are 1 thru 5, and values outside get limited to this range
	int8_t mode = 1;	// shingled mode == 0, standard == 1 thru 14 capped to syms_per_third symbols if syms_per_third < 5
	gf16_poly pad = 0;	// what to place into the un-transmitted code region, anything that would extend past the remaining symbols in a third is discarded
	//TODO: padding content seems to not effect encoding values so this variable can probably be removed

	//input range protection logic
	syms_per_third = (syms_per_third < 1) ? 1 : syms_per_third;	//input protection lower bound
	if(syms_per_third < 5)
		mode = (mode > syms_per_third) ? syms_per_third : mode;	// cap mode to syms_per_third if syms_per_third != 5
	else
	{
		syms_per_third = 5;	//input protection upper bound
		mode = (mode > 14) ? 14 : mode;	// cap mode to 14 check symbols, 1 data symbol
	}

	gf16_idx bits_per_third = GF16_SYM_SZ * syms_per_third;
	pad &= THIRD_MASK >> bits_per_third;	//only keep enough to pad the untransmitted portion

	if(mode != 0)	// shingled mode doesn't get repeated padding
		pad |= (pad << THIRD_SIZE) | (pad << (2 * THIRD_SIZE));

	// NOTE: everything beyond this point assumes mode != 0 for now, WIP
	uint32_t third_tx_mask = ~((uint32_t)-1 << bits_per_third);
	int8_t data_syms = 3 * syms_per_third - mode;
	uint64_t limit = GF16_MAX + 1;
	for(int i = 1; i < data_syms; ++i)
		limit *= limit;
	// the number of codewords has the potential to be huge and size isn't known at compile time so it must be
	//  malloc'd on the heap
	//gf16_poly* codewords = malloc(limit*sizeof(gf16_poly));
	//memset(codewords, -1, limit*sizeof(gf16_poly));	//init to invalid data to easily differentiate set values
*/
	FILE* marker_fp = fopen("marker_test3.txt", "w");
	uint32_t non_orientable_cnt = 0;
	for (uint64_t i = 0; i < 256; ++i)
	{
		gf16_poly raw_data;
		raw_data = i ;//& 0xF;
		//raw_data |= (i >> 4) << THIRD_SIZE;	// 2nd third
		//raw_data |= ((i >> (2 * bits_per_third)) & third_tx_mask) << (2 * THIRD_SIZE);	// 3rd third, only has an effect when syms_per_third == 5
		raw_data <<= 6*GF16_SYM_SZ;//THIRD_SIZE - 8;

		gf16_poly codeword = rs16_encode(raw_data, 4);
		codeword ^= gf16_poly_scale(0xBB001E9, (codeword >> 8) & GF16_MAX);
		codeword ^= gf16_poly_scale(0xD4010FD, (codeword >> 12) & GF16_MAX);

		if((codeword & TWO_THIRD_MASK) == codeword >> THIRD_SIZE)
		{
			fprintf(marker_fp, "Non-orientable ");
			++non_orientable_cnt;
		}

		uint32_t third0, third1, third2;
		third0 = codeword & THIRD_MASK;
		third1 = (codeword >> THIRD_SIZE) & THIRD_MASK;
		third2 = codeword >> (2*THIRD_SIZE);
		fprintf(marker_fp, "CW: %02X/%02X/%02X", third2, third1, third0);
		//fprintf(marker_fp, "CW: %012llX", codeword);

		gf16_elem sum2 = third0 ^ third1;
		gf16_elem sum3 = sum2 ^ third2;
		//gf16_elem prod = gf16_mul(gf16_mul(i & GF16_MAX, (i >> GF16_SYM_SZ) & GF16_MAX), codeword & GF16_MAX);
		fprintf(marker_fp, " SUM2: %02X SUM3: %02X\n", sum2, sum3);
		//marker_fp = fopen("markers/", "w");
	}
	fclose(marker_fp);

	printf("Found %i non-orientable codewords.\n", non_orientable_cnt);

	//free(codewords);
//*/
	return 0;
}