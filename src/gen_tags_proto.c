#include <stdio.h>
#include "gf16.h"

#define MARKER_SVG_HEADER "\
<svg width=\"256\" height=\"256\" viewBox=\"-2 -2 4 4\" xmlns=\"http://www.w3.org/2000/svg\">\n\
	<circle r=\"2\" fill=\"#000\"/>\n\
	<polygon points=\"0,2 -1.73205081,-1 1.73205081,-1\" fill=\"#FFF\"/>\n\
	<defs>\n\
		<polygon id=\"H\" points=\"0,2 -1.73205081,1 -1.73205081,-1 0,-2 1.73205081,-1 1.73205081,1\" shape-rendering=\"geometricPrecision\"/>\n\
	</defs>\n\
	<g transform=\"scale(%f)\">\n"
#define USE_SVG_BIT "\
		<use href=\"#H\" fill=\"#%03X\" y=\"%i\" x=\"%.8f\"/>\n"
#define MARKER_SVG_FOOTER "\
	</g>\n\
</svg>"

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

			FILE* marker_fp = fopen(filename, "w");
			//fwrite(MARKER_SVG_HEADER, 1, sizeof(MARKER_SVG_HEADER) - 1, marker_fp);
			fprintf(marker_fp, MARKER_SVG_HEADER, 0.2);
			for(uint16_t bit = 0; bit < 12; ++bit)
			{
				fprintf(marker_fp, USE_SVG_BIT, (codeword & 1) ? 0xFFF : 0, bit_y_coord[bit], bit_x_coord[bit] * 1.7320508075688773);
				codeword >>= 1;
			}
			fwrite(MARKER_SVG_FOOTER, 1, sizeof(MARKER_SVG_FOOTER) - 1, marker_fp);
			fclose(marker_fp);
		}
	}
}