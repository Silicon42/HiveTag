// a small utility program that helps me enter the coordinates of bits in a less error prone way.
//  Only 1/3 of the bit coords must be specified and the other 2/3 are calculated from that and
//  checked for validity and transformed to the stretched coordinate system that the .svg's use to
//  save on disk space while maintaining maximum precision

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define HELP_STRING "\
Commands are as follows, with capital letters representing variables:\n\
:XYSB  - Set cell at column X, diagonal row Y to bit B of symbol S. Will fail\n\
       if out of bounds, SB was already used, or cell XY already occupied\n\
       Ex: :03@2\n\
cSB    - Clear specified bit B of symbol S.\n\
       Ex: c!1\n\
h      Print the Help screen.\n\
p      Print the calculated layout and quit.\n\
q      Quit without printing the layout.\n\
\n\
ADVANCED USAGE:\n\
Multiple commands can be specified consecutively and will be applied in order\n\
without reprinting the UI, however in the event of multiple errors only the last\n\
error will be printed.\n\
\n\
HINT:\n\
Layouts such that bits within a symbol are recommended for best occlusion\n\
performance as occlusions are modeled as burst errors/erasures in the terms of\n\
the Reed-Solomon error correction scheme.\n\
\n\
	PRESS ENTER TO CONTINUE\n"

#define HEXAGON_GRID_HEADER "\n\n\n\n\
        1               Input Example      0\n\
    0   |   2           >:03#1         3  _|_    (!@#$%%):  symbol indicator\n\
1   |`*.|   |   3       Legend:         `/#1#\\    (0123):  bit ID\n\
 `*.|   |`*.|  _|_  4                    \\_a_/     (abc):  third differentiator\n"
#define HEXAGON_GRID "\
2   |`*.|  _|_/   \\_|_  5              \n\
 `*.|  _|_/   \\___/   \\_|_  6          \n\
3  _|_/   \\___/   \\___/   \\_|_  7      \n\
 `/   \\___/   \\___/   \\___/   \\_|_  8  \n\
4 \\___/   \\___/   \\___/   \\___/   \\_|_ \n\
 `/   \\___/   \\___/   \\___/   \\___/   \\\n\
5 \\___/   \\___/   \\___/   \\___/   \\___/\n\
 `/   \\___/   \\___/   \\___/   \\___/   \\\n\
6 \\___/   \\___/   \\___/   \\___/   \\___/\n\
 `/   \\___/   \\___/   \\___/   \\___/   \\\n\
7 \\___/   \\___/   \\___/   \\___/   \\___/\n\
 `/   \\___/   \\___/   \\___/   \\___/   \\\n\
8 \\___/   \\___/   \\___/   \\___/   \\___/\n\
 `/   \\___/   \\___/   \\___/   \\___/    \n\
  \\___/   \\___/   \\___/   \\___/        \n\
      \\___/   \\___/   \\___/            \n\
          \\___/   \\___/                \n\
              \\___/\n"

// mask to divide the occupancy vector in half such that each half corresponds to a sixth of the grid
#define OCC_DIV 0b1001101111111

char ui[] = HEXAGON_GRID;
uint8_t bit_x[20];	// index represents the symbol number in the hi bits and the bit number in the low bits
uint8_t bit_y[20];
// 20-bit vector where a 1 represents an occupied cell and its rotated counterparts, position represents an
//  index where one of the 3 possible coordinates is < 4 and the other used is >= 4, ie is tied to the
//  physical location in the layout
uint32_t occupancy= 0;
// feedback string gets printed on the next print of the ui, this means that it won't appear in the middle
//  of parsing a string and only the last occuring error will print but it's a tiny utility program and I
//  wanted to keep the ability to chain instructions indefinitely.
const char* feedback = "Specify column, row, symbol, and bit id. Enter 'h' for help.\n";

int get_str_pos(uint8_t x, uint8_t y) // calculates the position in the string that the cell is at
{
	return x * 44 - 116 + y * 80;
}

// when looking at rotated coordinates, all 3 coords exchange meaning, the third is determined by
//  the other 2
uint8_t get_3rd_coord(uint8_t x, uint8_t y)
{
	return 11 - x - y;
}

uint8_t get_occupancy_pos(uint8_t x, uint8_t y)
{
	if(x < 4 && y >= 4)
		return x | ((y - 4) << 2);
	uint8_t z = get_3rd_coord(x, y);
	if(y < 4 && z >= 4)
		return y | ((z - 4) << 2);

	return z | ((x - 4) << 2);
}

uint8_t is_cell_occupied(uint8_t x, uint8_t y)
{
	return (occupancy >> get_occupancy_pos(x, y)) & 1;
}

void toggle_cell_occupancy(uint8_t x, uint8_t y)
{
	occupancy ^= 1L << get_occupancy_pos(x, y);
}

void update_ui_occupancy(uint8_t x, uint8_t y, char symbol, char bit_id)
{
	toggle_cell_occupancy(x, y);
	uint8_t coords[4];	// this allows easy rotation through the x, y, and z coordinates
	coords[0] = x;
	coords[1] = y;
	coords[2] = get_3rd_coord(x, y);
	coords[3] = x;
	int str_pos;

	for(int i = 0; i < 3; ++i)
	{
		str_pos = get_str_pos(coords[i], coords[i+1]);
		ui[str_pos] = bit_id;
		ui[str_pos + 1] = symbol;
		ui[str_pos - 1] = symbol;
		ui[str_pos + 40] = ui[str_pos + 40] == '_' ? 'a' + i : '_';
	}
}

int8_t get_svg_x(uint8_t x, uint8_t y)
{
	return 11 - 2 * y - x;
}

int8_t get_svg_y(uint8_t x)
{
	return 3 * x - 11;
}

int write_layout()	// returns 1 if invalid
{
	uint8_t syms_found = 0;
	uint8_t bit_count = 0;

	// check that all present symbols have all their bits accounted for
	for(int i = 0; i < 20; ++i)
	{
		if(bit_x[i] <= 11)
		{
			++bit_count;
			syms_found |= 1 << i/4;
		}
	}

	if(!syms_found)	// empty layout, assumes the user meant to quit
		return 0;

	int8_t sym_count = __builtin_popcount(syms_found);
	if(sym_count*4 != bit_count)
	{
		feedback = "Layout is invalid, some symbol(s) are missing bits.\n";
		return 1;
	}

	// used to ensure that the wider "heavier" side of the bit arrangement is the bottom
	int8_t sixth_inner = __builtin_popcount(occupancy & OCC_DIV);
	int8_t sixth_outer = __builtin_popcount(occupancy & ~OCC_DIV);
	int8_t heavyside = (sixth_inner >= sixth_outer) ? 1 : -1;

	int8_t s = 0;
	printf("\nint8_t layout[%i][4][3][2] = {", sym_count);
	// iterate over the symbols, which may not be contiguous and, construct a contiguous version in
	//  the stretched integer aligned coordinate system that the svgs use for compact storage.
	for(int i = 0; i < 20; ++i)
	{
		uint8_t x = bit_x[i];
		if(x > 11)	//skip empty bits
			continue;
		
		if(i % 4 == 0)	// symbol boundary comment
		{
			printf("\n  {// [%i][0][_]  [%i][1][_]  [%i][2][_]", s, s, s);
			++s;
		}
		
		uint8_t y = bit_y[i];
		uint8_t z = get_3rd_coord(x, y);
		int8_t x0, y0, x1, y1, x2, y2;
		x0 = get_svg_x(x, y);
		x1 = get_svg_x(y, z);
		x2 = get_svg_x(z, x);
		y0 = get_svg_y(x) * heavyside;
		y1 = get_svg_y(y) * heavyside;
		y2 = get_svg_y(z) * heavyside;
		printf("\n    { {%3i,%3i}, {%3i,%3i}, {%3i,%3i} },", x0, y0, x1, y1, x2, y2);
		
		if(i % 4 == 3)	// symbol boundary comment
			printf("\b  },");
	}
	printf("\b \n};");

	return 0;
}

// parsing is my passion <\s>
void read_option()
{
	unsigned char in = 0, x = -1, y, symbol, bit_id;

	while(in != '\n')
	{
		in = getchar();
		switch (in)
		{
		case ':':	// specifier follows
			// read the x coordinate
			in = getchar();
			x = in - '0';
			if(x > 8)
			{
				feedback = "Invalid column number.\n";
				break;
			}

			// read the y coordinate
			in = getchar();
			y = in - '0';
			if(y > 8)
			{
				feedback = "Invalid row number.\n";
				break;
			}
			
			if((x + y < 3) || (x + y > 11))	//check upper left and lower right bounds
			{
				feedback = "Invalid column + row combination.\n";
				break;
			}
			
		case 'c':	// clear specified bit
			// read symbol specifier
			in = getchar();
			symbol = in;
			if((in - '!' > 4  && in != '@') || in == '"')	// check if valid symbol indicator input
			{
				feedback = "Invalid symbol indicator.\n";
				break;
			}

			// read bit id
			in = getchar();
			bit_id = in;
			if(in - '0' > 3)	// check bit id was 0 through 3
			{
				feedback = "Invalid bit ID.\n";
				break;
			}

			// get index for coord arrays
			int8_t idx = symbol - '!';
			if(idx > 4)	// special case for '@'
				idx = 1;
			idx <<= 2;
			idx |= bit_id - '0';
			if(x <= 11)	// x was set so entry was through the "specifier" option
			{
				if(bit_x[idx] <= 11)	// using already placed bits isn't allowed w/o first clearing them
				{
					feedback = "Symbol + bit combination already in use.\n";
					break;
				}
				
				if(is_cell_occupied(x, y))	// setting already occupied cells isn't allowed w/o first clearing them
				{
					feedback = "Specified cell is already occupied.\n";
					break;
				}
				
				bit_x[idx] = x;
				bit_y[idx] = y;
			}
			else	// x wasn't set so entry was through the "clear" option
			{
				// infer coords from symbol and bit number
				x = bit_x[idx];
				y = bit_y[idx];
				if(x > 11)
				{
					feedback = "Symbol + bit not currently assigned.\n";
					break;
				}

				bit_x[idx] = -1;	// reset the given entry to an invalid value

				symbol = ' ';
				bit_id = ' ';
			}

			update_ui_occupancy(x, y, symbol, bit_id);

			break;
		case 'h':	// help
			printf(HELP_STRING);
			while(getchar() != '\n');	// eat any remaining input until newline
			
			in = 0;	// then wait for enter to be pressed again, (a command can be entered with help open)
			break;
		case 'p':	// print and quit
			if(write_layout())	// check for validity and if not valid, don't exit
				break;
		case 'q':	// quit
			exit(0);
	//	case '\n':	// end of input
		default:	// invalid input, can occur because failure earlier in parsing, so no feedback setting
			break;
		}
		x = -1;	// set x value to 255 (invalid) so it can still be used as a flag
	}
}

int main()
{
	memset(bit_x, -1, 20);	// initialize x-coords to an invailid value
	while(1)
	{
		printf(HEXAGON_GRID_HEADER);
		printf(ui);
		printf(feedback);
		putchar('>');
		feedback = "Enter 'h' for help.\n";

		// handle input and updating of the ui string
		read_option();
	}
}