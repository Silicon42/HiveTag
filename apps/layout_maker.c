// a small utility program that helps me enter the coordinates of bits in a less error prone way.
//  Only 1/3 of the bit coords must be specified and the other 2/3 are calculated from that and
//  checked for validity and transformed to the stretched coordinate system that the .svg's use to
//  save on disk space while maintaining maximum precision

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define HELP_STRING "\n\
COMMANDS:\n\
Commands are as follows, with capital letters representing variable values:\n\
\n\
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
	PRESS ENTER TO CONTINUE"

#define HEXAGON_GRID_HEADER "\n\n\n\n\
        1               Input Example      0\n\
    0   |   2           >:03#1         3  _|_    (!@#$%):  symbol indicator\n\
1   |`*.|   |   3       Legend:         `/#1#\\    (0123):  bit ID\n\
 `*.|   |`*.|  _|_  4                    \\_a_/     (abc):  third differentiator"
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
const char* feedback = "Specify column, row, symbol, and bit id. Enter 'h' for help.";

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

	int8_t nDiv3 = __builtin_popcount(syms_found);
	if(nDiv3*4 != bit_count)
	{
		feedback = "Layout is invalid, some symbol(s) are missing bits.";
		return 1;
	}

	// used to ensure that the wider "heavier" side of the bit arrangement is the bottom
	int8_t sixth_inner = __builtin_popcount(occupancy & OCC_DIV);
	int8_t sixth_outer = __builtin_popcount(occupancy & ~OCC_DIV);
	int8_t heavyside = (sixth_inner > sixth_outer) ? 1 : -1;
	
	// construct the svg coordinates in an array so that they are contiguous and symbols are spaced
	//  proportionally to how they occur in the Reed-Solomon view
	int8_t svgx[60], svgy[60];
	bit_count = 0;
	for(int i = 0; i < 20; ++i)
	{
		uint8_t x, y, z;
		x = bit_x[i];
		if(x > 11)	//skip empty bits
			continue;
		
		y = bit_y[i];
		z = get_3rd_coord(x, y);
		svgx[bit_count]     = get_svg_x(x, y) * heavyside;	// if the "heavy" side of the marker would be negative in x, rotate 180 deg
		svgx[bit_count + 20]= get_svg_x(y, z) * heavyside;
		svgx[bit_count + 40]= get_svg_x(z, x) * heavyside;
		svgy[bit_count]     = get_svg_y(x) * heavyside;
		svgy[bit_count + 20]= get_svg_y(y) * heavyside;
		svgy[bit_count + 40]= get_svg_y(z) * heavyside;
		++bit_count;
	}

	// print the formatted array for easy copy pasting of the layout
	// indices of the printed array correspond to bit number in the contracted view
	printf("\nconst struct int8_2D bits[] = {");
	for(int i = 0; i < 3; ++i)
	{
		printf("\n    //third %i", i);
		int8_t i_off = i * 20;
		for(int j = 0; j < nDiv3; ++j)
		{
			int8_t j_off = j * 4;
			printf("\n    ");
			for(int k = 0; k < 4; ++k)
				printf("{%2i,%3i}, ", svgx[k + j_off + i_off], svgy[k + j_off + i_off]);
		}
	}
	puts("\b\b \n};\n");

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
				feedback = "Invalid column number.";
				break;
			}

			// read the y coordinate
			in = getchar();
			y = in - '0';
			if(y > 8)
			{
				feedback = "Invalid row number.";
				break;
			}
			
			if((x + y < 3) || (x + y > 11))	//check upper left and lower right bounds
			{
				feedback = "Invalid column + row combination.";
				break;
			}
			
		case 'c':	// clear specified bit
			// read symbol specifier
			in = getchar();
			symbol = in;
			if((in - '!' > 4  && in != '@') || in == '"')	// check if valid symbol indicator input
			{
				feedback = "Invalid symbol indicator.";
				break;
			}

			// read bit id
			in = getchar();
			bit_id = in;
			if(in - '0' > 3)	// check bit id was 0 through 3
			{
				feedback = "Invalid bit ID.";
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
					feedback = "Symbol + bit combination already in use.";
					break;
				}
				
				if(is_cell_occupied(x, y))	// setting already occupied cells isn't allowed w/o first clearing them
				{
					feedback = "Specified cell is already occupied.";
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
					feedback = "Symbol + bit not currently assigned.";
					break;
				}

				bit_x[idx] = -1;	// reset the given entry to an invalid value

				symbol = ' ';
				bit_id = ' ';
			}

			update_ui_occupancy(x, y, symbol, bit_id);

			break;
		case 'h':	// help
			puts(HELP_STRING);
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
		puts(HEXAGON_GRID_HEADER);
		puts(ui);
		puts(feedback);
		putchar('>');
		feedback = "Enter 'h' for help.";

		// handle input and updating of the ui string
		read_option();
	}
}