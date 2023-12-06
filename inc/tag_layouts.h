#ifndef TAG_LAYOUTS_H
#define TAG_LAYOUTS_H
// a layout stores data about bit coordinates and what symbol each bit belongs to,
//  which allows for projecting bit locations onto an occlussion map to find the
//  locations of symbol erasures
#include <stdint.h>
struct int8_2D {
	int8_t x;
	int8_t y;
};

struct layout {
	const struct int8_2D* bits;	// length is implied by symbol count
	const struct int8_2D* bg_mask;	// background to mask gaps between black hexagons
	const int8_t bg_mask_len;	// number of points involved
	const struct int8_2D* fg_mask1;	// point list for one of the upper 2 alignment corners
	const int8_t fg_mask1_len;	// number of points involved
	const struct int8_2D* fg_mask2;	// point list for (half) the lower allignment corner
	const int8_t fg_mask2_len;	// number of points involved
	const float scale;	// how muchg to scale the resulting bit arrangement to fit it in the circle
	const int8_t rotate;	//signed rotation amount as 120 deg/256 to apply, meant for nDiv3 == 2
};

extern const struct layout default_layouts[5];

#endif //TAG_LAYOUTS_H