// Dimensions.cpp

#include "Dimensions.h"

// the possible resolutions
uint32 color_spaces[] =
{
	B_8_BIT_640x480,
	B_8_BIT_800x600,
	B_8_BIT_1024x768,
	B_8_BIT_1152x900,
	B_8_BIT_1280x1024,
	B_8_BIT_1600x1200,
};

// the names of the resolutions
const char *color_space_names[] =
{
	"640x480",
	"800x600",
	"1024x768",
	"1152x900",
	"1280x1024",
	"1600x1200",
};

size_t color_spaces_count = sizeof(color_spaces) / sizeof(uint32);

