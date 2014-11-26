// Dimensions.h

#ifndef DIMENSIONS_H
#define DIMENSIONS_H

#include <kernel/OS.h>
#include <game/WindowScreen.h>

// the possible resolutions
extern uint32 color_spaces[];
// the names of the resolutions
extern char *color_space_names[];
// the number of color spaces
extern size_t color_spaces_count;

enum
{
	CURRENT_WORKSPACE_DIMENSIONS = 0
};

inline void get_dimensions_for(uint32 space, int32 *w, int32 *h)
{
	switch (space)
	{
		case B_8_BIT_640x480:
			*w = 640; *h = 480;
			break;
		case B_8_BIT_800x600:
			*w = 800; *h = 600;
			break;
		case B_8_BIT_1024x768:
			*w = 1024; *h = 768;
			break;
		case B_8_BIT_1152x900:
			*w = 1152; *h = 900;
			break;
		case B_8_BIT_1280x1024:
			*w = 1280; *h = 1024;
			break;
		case B_8_BIT_1600x1200:
			*w = 1600; *h = 1200;
			break;
	}
}

inline uint32 color_space_for(int32 w, int32)
{
	if (w <= 640)
		return B_8_BIT_640x480;
	if (w <= 800)
		return B_8_BIT_800x600;
	if (w <= 1024)
		return B_8_BIT_1024x768;
	if (w <= 1152)
		return B_8_BIT_1152x900;
	if (w <= 1280)
		return B_8_BIT_1280x1024;
//	if (w <= 1600)
//		return B_8_BIT_1600x1200;
	return B_8_BIT_1600x1200;
}


#endif	// DIMENSIONS_H

