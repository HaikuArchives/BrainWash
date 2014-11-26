// Drawing.h

#ifndef DRAWING_H
#define DRAWING_H

// inline functions
inline void WritePixel(char *fb, int x, int y, int c, int w)
{
	fb[y * w + x] = c;
}

inline int ReadPixel(char *fb, int x, int y, int w)
{
	return fb[y * w + x];
}

inline void ClearScreen(char *fb, int wi, int hi, int col = 0)
{
	for (int i = 0; i < wi * hi; i++)
		fb[i] = col;
}

#endif	// DRAWING_H
