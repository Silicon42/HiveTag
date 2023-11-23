#include <stdio.h>
#include "gf16.h"

#define MARKER_SVG_HEADER \
"<svg viewBox=\"-2 -2 4 4\" xmlns=\"http://www.w3.org/2000/svg\">"\
	"<defs>"\
		"<polygon points=\"0,2 -1,1 -1,-1 0,-2 1,-1 1,1\" id=\"H\"/>"\
		"<ellipse id=\"C\" rx=\"1.15470054\" ry=\"2\" fill=\"#FFF\"/>"\
	"</defs>"\
	"<mask id=\"M\">"\
		"<circle r=\"2\" fill=\"#FFF\"/>"\
		"<g transform=\"scale(.346410162 .2)\">"\
			"<polygon points=\"0,10 -5,-5 5,-5\"/>"\
			"<polygon points=\"0,-6 -3,3 3,3\"/>"

//#define HEXAGON_INT "-1,1 -1,-1 0,-2 1,-1 1,1"
//#define HEXAGON_FLOAT "-1.73205081,1 -1.73205081,-1 0,-2 1.73205081,-1 1.73205081,1\" transform=\"scale(.577350269 1)\" stroke"
#define USE_SVG_BIT \
			"<use href=\"#%c\" x=\"%i\" y=\"%i\"/>"
#define MARKER_SVG_FOOTER \
			"<g id=\"R\">"\
				"<polygon points=\"0,10 0,6 1,5 1.5,5.5\"/>"\
				"<polygon points=\"5,-5 2,-5 2,-4 3,-3 3,-1 3.5,-.5\"/>"\
			"</g>"\
			"<use href=\"#R\" transform=\"scale(-1, 1)\"/>"\
		"</g>"\
	"</mask>"\
	"<circle r=\"2\" mask=\"url(#M)\"/>"\
"</svg>"

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
	uint32_t third_tx_mask = ~(-1L << bits_per_third);
	int8_t data_syms = 3 * syms_per_third - mode;
	uint64_t limit = GF16_MAX + 1;
	for(int i = 1; i < data_syms; ++i)
		limit *= limit;
	// the number of codewords has the potential to be huge and size isn't known at compile time so it must be
	//  malloc'd on the heap
	//gf16_poly* codewords = malloc(limit*sizeof(gf16_poly));
	//memset(codewords, -1, limit*sizeof(gf16_poly));	//init to invalid data to easily differentiate set values
*/

int main()
{
	const int8_t bit_y_coord[] = {-5, -5, -2, -2, 1, 4, 1, 4, 4, 1, 1, -2};
	const int8_t bit_x_coord[] = {-1, 1, 0, 2, 3, 2, 1, 0, -2, -3, -1, -2};

	for(gf16_elem i = 0; i < 5; ++i)
	{
		uint16_t cw_base = gf16_exp[i+10];
		cw_base <<= GF16_SYM_SZ;
		cw_base |= gf16_exp[i+5];
		cw_base <<= GF16_SYM_SZ;
		cw_base |= gf16_exp[i];
		uint8_t id_base = i << GF16_SYM_SZ;
		
		for(gf16_elem j = 0; j <= GF16_MAX; ++j)
		{
			uint16_t codeword = cw_base ^ (j * 0x111);
			uint8_t id = id_base | j;
			printf("ID: %02X	CW: %03X\n", id, codeword);
			char filename[32];
			sprintf(filename, "markers/n3_k2_id%02X.svg", id);

			uint16_t cw_copy = codeword;
			FILE* marker_fp = fopen(filename, "w");
			fprintf(marker_fp, MARKER_SVG_HEADER);
			for(uint16_t bit = 0; bit < 12; ++bit)
			{
				if(~cw_copy & 1)
					fprintf(marker_fp, USE_SVG_BIT, 'C', bit_x_coord[bit], bit_y_coord[bit]);
				cw_copy >>= 1;
			}
			for(uint16_t bit = 0; bit < 12; ++bit)
			{
				if(codeword & 1)
					fprintf(marker_fp, USE_SVG_BIT, 'H', bit_x_coord[bit], bit_y_coord[bit]);
				codeword >>= 1;
			}
			fprintf(marker_fp, MARKER_SVG_FOOTER);
			fclose(marker_fp);
		}
	}
}