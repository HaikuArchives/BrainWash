// Threads.h

#ifndef THREADS_H
#define THREADS_H

#include <kernel/OS.h>
#include <interface/Point.h>
#include "Thread.h"

const int32 max_pixels_per_line = 2048;

// CycleThread
class CycleThread : public CmdThread
{
public:
	CycleThread(BWindowScreen *screen);
	bool	cycle_2nd_palette;
private:
	virtual int32 CmdThreadFunction();
	BWindowScreen	*screen;
};

// DrawThread
class DrawThread : public CmdThread
{
public:
	DrawThread(BWindowScreen *screen, CycleThread *cycle_thread, uint32 space);
private:
	virtual int32 CmdThreadFunction();
	BWindowScreen	*screen;
	CycleThread		*cycle_thread;
	uint32			color_space;
};

class PreCalcList;

// Function
class Function : public CmdThread
{
public:
	Function(char *fb, float w, float h, int cstartcol = 0);
	~Function();
	virtual int32 CmdThreadFunction();
	bool Draw();
	int32 GetNextLine(BPoint *);
	void Init();
	void SetXY(int cx0, int cy0) { x0 = cx0; y0 = cy0; };

private:
	char		*fb;
	float		w;
	float		h;
	int32		startcol;
	PreCalcList	*prelist;
	BList		*postlist;
	int			x0, y0;
	uint32		side;
	int			l, t, r, b;
};


#endif	// THREADS_H

