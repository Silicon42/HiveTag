// default layouts for how components of the svg should be arranged in the svg's pre-transformation
//  coordinate system and what scaling/rotation should be applied if any
//  commands used to generate the layout via layout_maker.exe are provided in comments

#include "tag_layouts.h"

// bit coordinates are specified with all bits belonging to the same symbol on one line and the
//  boundary between thirds of the codeword marked with comments

//----- nDiv3 == 1 -----//
// :53!0:54!1:44!2:45!3
const struct int8_2D bits1[] = {
	//third 0
	{ 0,  4}, {-2,  4}, {-1,  1}, {-3,  1},
	//third 1
	{ 2, -2}, { 3,  1}, { 1,  1}, { 2,  4},
	//third 2
	{-2, -2}, {-1, -5}, { 0, -2}, { 1, -5}
};

const struct int8_2D bg_fill1[] = {
	{0, -6}, {-3, 3}, {3, 3}
};

const struct int8_2D bits_mask1[] = {
	{-1,0}, {0,6}, {1,5}, {2,6}, {4,4}, {4,2}, {4,0}, {3,-1}, {3,-3}, {2,-4}, {2,-6}, {0,-8}
};

#define DEFAULT_LAYOUT1 {\
	.bits = bits1,\
	.bg_fill = bg_fill1,\
	.bg_fill_len = 3,\
	.bits_mask = bits_mask1,\
	.bits_mask_len = 12,\
	.is_bits_mask_mirrored = 1,\
	.scale_div = 10,\
	.rotate = 0\
}

// :63!0:64!1:54!2:55!3 :44@0:45@1:35@2:36@3
const struct int8_2D bits2[] = {
	//third 0
	{-1,  7}, {-3,  7}, {-2,  4}, {-4,  4},
	{-1,  1}, {-3,  1}, {-2, -2}, {-4, -2},
	//third 1
	{ 4, -2}, { 5,  1}, { 3,  1}, { 4,  4},
	{ 1,  1}, { 2,  4}, { 0,  4}, { 1,  7},
	//third 2
	{-3, -5}, {-2, -8}, {-1, -5}, { 0, -8},
	{ 0, -2}, { 1, -5}, { 2, -2}, { 3, -5}
};

const struct int8_2D bg_fill2[] = {
	{0, -6}
};

const struct int8_2D bits_mask2[] = {
	{-1,0}, {0,6}, {1,5}, {2,6}, {4,4}, {4,2}, {4,0}, {3,-1}, {3,-3}, {2,-4}, {2,-6}, {0,-8}
};

#define DEFAULT_LAYOUT2 {\
	.bits = bits2,\
	.bg_fill = bg_fill2,\
	.bg_fill_len = 6,\
	.bits_mask = bits_mask2,\
	.bits_mask_len = 12,\
	.is_bits_mask_mirrored = 1,\
	.scale_div = 13,\
	.rotate = 5\
}

// :34!0:24!1:25!2:15!3 :14@0:13@1:23@2:22@3 :33#0:32#1:31#2:41#3
const struct int8_2D bits3[] = {
	//third 0
	{ 0,  2}, {-1,  5}, { 1,  5}, { 0,  8},
	{-2,  8}, {-4,  8}, {-3,  5}, {-5,  5},
	{-2,  2}, {-4,  2}, {-6,  2}, {-5, -1},
	//third 1
	{ 1, -1}, { 3, -1}, { 2, -4}, { 4, -4},
	{ 5, -1}, { 6,  2}, { 4,  2}, { 5,  5},
	{ 2,  2}, { 3,  5}, { 4,  8}, { 2,  8},
	//third 2
	{-1, -1}, {-2, -4}, {-3, -1}, {-4, -4},
	{-3, -7}, {-2,-10}, {-1, -7}, { 0,-10},
	{ 0, -4}, { 1, -7}, { 2,-10}, { 3, -7}
};

const struct int8_2D bg_fill3[] = {
	{7,-1},{1, -11},{-1, -11},{-7,-1},
};

const struct int8_2D bits_mask3[] = {
	{-1,0}, {0,6}, {1,5}, {2,6}, {4,4}, {4,2}, {4,0}, {3,-1}, {3,-3}, {2,-4}, {2,-6}, {0,-8}
};

#define DEFAULT_LAYOUT3 {\
	.bits = bits3,\
	.bg_fill = bg_fill3,\
	.bg_fill_len = 6,\
	.bits_mask = bits_mask3,\
	.bits_mask_len = 12,\
	.is_bits_mask_mirrored = 1,\
	.scale_div = 15,\
	.rotate = 5\
}

// :34!0:24!1:25!2:15!3 :16@0:06@1:17@2:07@3 :26#0:35#1:27#2:36#3 :37$0:46$1:47$2:56$3
const struct int8_2D bits4[] = {
	//third 0
	{ 0, -2}, { 1, -5}, {-1, -5}, { 0, -8},
	{-2, -8}, {-1,-11}, {-4, -8}, {-3,-11},
	{-3, -5}, {-2, -2}, {-5, -5}, {-4, -2},
	{-6, -2}, {-5,  1}, {-7,  1}, {-6,  4},
	//third 1
	{-1,  1}, {-3,  1}, {-2,  4}, {-4,  4},
	{-3,  7}, {-5,  7}, {-2, 10}, {-4, 10},
	{-1,  7}, { 0,  4}, { 0, 10}, { 1,  7},
	{ 2, 10}, { 3,  7}, { 4, 10}, { 5,  7},
	//third 2
	{ 1,  1}, { 2,  4}, { 3,  1}, { 4,  4},
	{ 5,  1}, { 6,  4}, { 6, -2}, { 7,  1},
	{ 4, -2}, { 2, -2}, { 5, -5}, { 3, -5},
	{ 4, -8}, { 2, -8}, { 3,-11}, { 1,-11}
};

const struct int8_2D bg_fill4[] = {
	{0, -6}
};

const struct int8_2D bits_mask4[] = {
	{-1,0}, {0,6}, {1,5}, {2,6}, {4,4}, {4,2}, {4,0}, {3,-1}, {3,-3}, {2,-4}, {2,-6}, {0,-8}
};

#define DEFAULT_LAYOUT4 {\
	.bits = bits4,\
	.bg_fill = bg_fill4,\
	.bg_fill_len = 6,\
	.bits_mask = bits_mask4,\
	.bits_mask_len = 12,\
	.is_bits_mask_mirrored = 1,\
	.scale_div = 16,\
	.rotate = 5\
}

// :34!0:24!1:25!2:15!3 :16@0:06@1:17@2:07@3 :26#0:35#1:27#2:36#3 :37$0:46$1:47$2:56$3 :08%0:18%1:28%2:38%3
const struct int8_2D bits5[] = {
	//third 0
	{ 0,  2}, {-1,  5}, { 1,  5}, { 0,  8},
	{ 2,  8}, { 1, 11}, { 4,  8}, { 3, 11},
	{ 3,  5}, { 2,  2}, { 5,  5}, { 4,  2},
	{ 6,  2}, { 5, -1}, { 7, -1}, { 6, -4},
	{ 5, 11}, { 6,  8}, { 7,  5}, { 8,  2},
	//third 1
	{ 1, -1}, { 3, -1}, { 2, -4}, { 4, -4},
	{ 3, -7}, { 5, -7}, { 2,-10}, { 4,-10},
	{ 1, -7}, { 0, -4}, { 0,-10}, {-1, -7},
	{-2,-10}, {-3, -7}, {-4,-10}, {-5, -7},
	{ 3,-13}, { 1,-13}, {-1,-13}, {-3,-13},
	//third 2
	{-1, -1}, {-2, -4}, {-3, -1}, {-4, -4},
	{-5, -1}, {-6, -4}, {-6,  2}, {-7, -1},
	{-4,  2}, {-2,  2}, {-5,  5}, {-3,  5},
	{-4,  8}, {-2,  8}, {-3, 11}, {-1, 11},
	{-8,  2}, {-7,  5}, {-6,  8}, {-5, 11}
};

const struct int8_2D bg_fill3[] = {
	{0, -6}
};

const struct int8_2D bits_mask3[] = {
	{-1,0}, {0,6}, {1,5}, {2,6}, {4,4}, {4,2}, {4,0}, {3,-1}, {3,-3}, {2,-4}, {2,-6}, {0,-8}
};

#define DEFAULT_LAYOUT5 {\
	.bits = bits5,\
	.bg_fill = bg_fill5,\
	.bg_fill_len = 6,\
	.bits_mask = bits_mask5,\
	.bits_mask_len = 12,\
	.is_bits_mask_mirrored = 1,\
	.scale_div = 18,\
	.rotate = 5\
}

const struct layout default_layouts[5] = {DEFAULT_LAYOUT1, DEFAULT_LAYOUT2, DEFAULT_LAYOUT3, DEFAULT_LAYOUT4, DEFAULT_LAYOUT5};