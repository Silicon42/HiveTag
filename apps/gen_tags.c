#include "id_to_codeword.h"
#include "tag_layouts.h"
#include "cw_utils.h"
#include <stdio.h>
#include <string.h>

#define SVG_TAG_HEADER \
"<svg viewBox=\"-4 -4 8 8\" xmlns=\"http://www.w3.org/2000/svg\">"\
	"<defs>"\
		"<polygon points=\"0,2 -1,1 -1,-1 0,-2 1,-1 1,1\" id=\"H\"/>"\
		"<ellipse id=\"C\" rx=\"1.15470054\" ry=\"2\" fill=\"#FFF\"/>"\
	"</defs>"\
	"<mask id=\"E\" fill=\"#FFF\">"
#define SVG_POLYGON \
		"<polygon points=\""
#define SVG_CLOSE_POLYGON "\"/>"
#define SVG_BITS_MIRROR " id=\"R\"/>"\
		"<use href=\"#R\" transform=\"scale(-1,1)\"/>"
#define SVG_MASK_HEADER \
	"</mask>"\
	"<mask id=\"M\">"\
		"<circle r=\"4\" fill=\"#FFF\"/>"\
		"<polygon points=\"0,4 -3.46410162,-2 3.46410162,-2\"/>"\
		"<g mask=\"url(#E)\" transform=\""
#define SVG_ROTATION					"rotate(%i)"
#define SVG_BITS_SCALE					"scale(%s,%s)\">"
// SVG_POLYGON re-used for bg_fill here
#define SVG_USE_BIT \
			"<use href=\"#%c\"%s y=\"%i\"/>"
#define SVG_X_STR " x=\"%i\""
#define TAG_SVG_FOOTER \
		"</g>"\
	"</mask>"\
	"<circle r=\"4\" mask=\"url(#M)\"/>"\
"</svg>"

// copies a c string literal to the buffer location and advances the pointer to the copied null char
#define SPUTS_LIT(buf_ptr, str_lit) \
{\
	memcpy(buf_ptr, str_lit, sizeof(str_lit));\
	buf_ptr += sizeof(str_lit) - 1;\
}

// returns pointer to postion of the end of the string written
char* write_polygon(char* buf, const struct int8_2D* points, int8_t size)
{
	SPUTS_LIT(buf, SVG_POLYGON);
	for(int i = 0; i < size; ++i)
		buf += sprintf(buf, "%i,%i ", points[i].x, points[i].y);
	// TODO: remove use of backspaces here, currently used just to experiment but could easily be replaced
	//  by moving pointer back and is likely more understandable
	--buf;
	SPUTS_LIT(buf, SVG_CLOSE_POLYGON);

	return buf;
}

void write_bits(FILE* marker_fp, gf16_poly codeword, const struct int8_2D* bits, int8_t bit_cnt, char element_id)
{
	for(uint8_t bit = 0; bit < bit_cnt; ++bit)
	{
		if(codeword & 1)
		{
			char x_str[8];
			if(bits[bit].x)
				sprintf(x_str, SVG_X_STR, bits[bit].x);
			else
				x_str[0] = '\0';
			fprintf(marker_fp, SVG_USE_BIT, element_id, x_str, bits[bit].y);
		}
		codeword >>= 1;
	}
}

int main()
{
	// copy pasted input parsing from rs_codeword_test since this isn't meant to be a final solution
	//TODO: implement HTML/JavaScript version that is interactive and allows the whole generation, sizing, page
	//  layout, and printing process to be done in a browser with option to export to a file for user convenience
	int nDiv3;

	while(1)
	{
		printf("Select a valid 'n/3' value (1 <= n <= 5): ");
		nDiv3 = getchar();
		getchar();	// eat the '\n'
		nDiv3 -= '0';
		if(1 <= nDiv3 && nDiv3 <= 5)
			break;
	}

	//TODO: currently assumes k == 2 and generates the whole marker range, javascript implementation will
	//  need a start id and a range
	int8_t bit_cnt = nDiv3 * 3 * GF16_SYM_SZ;
	const struct layout lo = default_layouts[nDiv3 - 1];

	char svg_header[1024];
	char* buf_pos = svg_header;	// end of the string/where to add on more
	SPUTS_LIT(buf_pos, SVG_TAG_HEADER);
	buf_pos = write_polygon(buf_pos, lo.bits_mask, lo.bits_mask_len);
	if(lo.is_bits_mask_mirrored)
	{
		buf_pos -= 2;	// needs to overwrite the last 2 chars
		SPUTS_LIT(buf_pos, SVG_BITS_MIRROR);
	}

	SPUTS_LIT(buf_pos, SVG_MASK_HEADER);

	// if a rotation is involved with the layout, then the alignment triangle must be drawn separately since it
	//  isn't supposed to be one of the parts rotated, at which point it's more space efficient to specify it
	//  in the un-scaled coordinate system
	if(lo.rotate)
		buf_pos += sprintf(buf_pos, SVG_ROTATION, lo.rotate);

	char x_scale[12], y_scale[12];	// printf has no way to omit the leading 0 so print to a string first
	sprintf(x_scale, "%.9g", SQRT_3*4.0/lo.scale_div);
	sprintf(y_scale, "%.9g", 4.0/lo.scale_div);
	buf_pos += sprintf(buf_pos, SVG_BITS_SCALE, x_scale + 1, y_scale + 1);
	
	buf_pos = write_polygon(buf_pos, lo.bg_fill, lo.bg_fill_len);
	
	for(gf16_elem i = 1; i <= 96; ++i)
	{
		gf16_poly codeword = id_to_codeword(i, nDiv3, 2);
		codeword = contract_cw(codeword, nDiv3);
		
		char filename[32];
		sprintf(filename, "markers/n%i/k2/n%i_k2_id%02i.svg", nDiv3*3, nDiv3*3, i);
		FILE* marker_fp = fopen(filename, "w");

		fprintf(marker_fp, svg_header);
		// write the clear, (circular) bits
		write_bits(marker_fp, ~codeword, lo.bits, bit_cnt, 'C');
		// write the set, (hexagonal) bits
		write_bits(marker_fp, codeword, lo.bits, bit_cnt, 'H');

		fprintf(marker_fp, TAG_SVG_FOOTER);

		fclose(marker_fp);
	}
}