#ifndef TAG_LAYOUTS_H
#define TAG_LAYOUTS_H
// a layout stores data about bit coordinates and what symbol each bit belongs to,
//  which allows for projecting bit locations onto an occlussion map to find the
//  locations of symbol erasures
#include <stdint.h>

#define SQRT_3 1.7320508075688773

struct int8_2D {
	int8_t x;
	int8_t y;
};

/* TODO: convert layout to use this so that length is properly associated with the array
struct sized_pos_arr {
	const struct int8_2D* pos;
	int8_t len;
}*/

struct layout {
	const struct int8_2D* bits;		// length is implied by symbol count
	const struct int8_2D* bg_fill;	// background to mask gaps between black hexagons
	const int8_t bg_fill_len;		// number of points involved
	const struct int8_2D* bits_mask;// point list for half the bit region mask except for  nDiv3 == 2, which is chiral
	const int8_t bits_mask_len;		// number of points involved
	const int8_t is_bits_mask_mirrored;
	const int8_t scale_div;			// divisor to scale the resulting bit arrangement to fit it in the circle
	const int8_t rotate;			// signed rotation amount in degrees, meant for nDiv3 == 2
};

extern const struct layout default_layouts[5];

#endif //TAG_LAYOUTS_H