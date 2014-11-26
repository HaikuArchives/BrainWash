// Functions.h

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <math.h>
#include "Calculation.h"

// min/max functions
template<class C> inline C min(C a, C b)
{
	return a < b ? a : b;
}

template<class C> inline C max(C a, C b)
{
	return a > b ? a : b;
}

// get the maximal distance
inline float GetMaxR(float w, float h, float x0, float y0)
{
	float maxx = max(x0, w - x0);
	float maxy = max(y0, h - y0);
	return sqrt(maxx * maxx + maxy * maxy);
}


// the precalc function types - used in Function::Init()
enum
{
	FUNC1			= 0,
	LENS1			= 1,
	TWIST			= 2,
	PRE_FUNC_COUNT
};

// the postcalc function types - used in Function::Init()
enum
{
	SPIRAL			= 0,
	SNAIL			= 1,
	CIRCLE			= 2,
	GRID			= 3,
	WAVES			= 4,
	FIELD			= 5,
	HOLE			= 6,
	POST_FUNC_COUNT
};

// the kinds of spirals
enum
{
	SPIRAL_COS		= 0,
	SPIRAL2_COS		= 1,
	SPIRAL_LIN		= 2,
	SPIRAL2_LIN		= 3,
	WHEEL_COS		= 4,
	WHEEL_LIN		= 5,
	SPIRAL_COUNT
};

// the kinds of circles
enum
{
	CIRCLE_COS		= 0,
	CIRCLE_LIN		= 1,
	CIRCLE_COUNT
};


// the PreCalc derived classes

// Func1
class Func1 : public PreCalc
{
public:
	Func1(float cw, float ch) { w = cw; h = ch; };
	virtual void CalculatePoints(BPoint *points, BPoint *results, int32 count);
private:
	float	w;
	float	h;
};

// Lens1
class Lens1 : public PreCalc
{
public:
	Lens1(float cx0, float cy0, float cw, float ch, float cloc, float cinf)
	{
		x0 = cx0; y0 = cy0; w = cw; h = ch; loc = cloc; inf = cinf;
		maxr = GetMaxR(w, h, x0, y0);
	};
	Lens1(float cw, float ch)
	{
		w = cw; h = ch;
		x0 = float(rand()) / RAND_MAX * (w - 1);
		y0 = float(rand()) / RAND_MAX * (h - 1);
		loc = float(rand()) / RAND_MAX * (2.7) + 0.3;
		inf = float(rand()) / RAND_MAX * 3 - 1.5;
		maxr = GetMaxR(w, h, x0, y0);
	};
	virtual void CalculatePoints(BPoint *points, BPoint *results, int32 count);
private:
	float	x0;
	float	y0;
	float	w;
	float	h;
	float	loc;
	float	inf;
	float	maxr;
};

// Twist
class Twist : public PreCalc
{
public:
	Twist(float cx0, float cy0, float crad, float cfak)
		{ x0 = cx0; y0 = cy0; rad = crad; fak = cfak; };
	Twist(float w, float h)
	{
		x0 = float(rand()) / RAND_MAX * (w - 1);
		y0 = float(rand()) / RAND_MAX * (h - 1);
		rad = float(rand()) / RAND_MAX * 9 + 1;
		fak = ((rand() % 2) * 2 - 1) * (float(rand()) / RAND_MAX * 1.9 + 0.1);
	};
	virtual void CalculatePoints(BPoint *points, BPoint *results, int32 count);
private:
	float	x0;
	float	y0;
	float	rad;
	float	fak;
};


// the PostCalc derived classes

// SpiralCos
class SpiralCos : public PostCalc
{
public:
	SpiralCos(float cw, float ch, float cx0, float cy0,
		float ctentacles = 2) : PostCalc()
		{ w = cw; h = ch; x0 = cx0; y0 = cy0; tentacles = ctentacles; };
	SpiralCos(float cw, float ch) : PostCalc()
	{
		w = cw; h = ch;
		x0 = float(rand()) / RAND_MAX * (w - 1);
		y0 = float(rand()) / RAND_MAX * (h - 1);
		tentacles = ((rand() % 2) * 2 - 1) * (rand() % 19 + 4);
	};
	virtual void CalculatePoints(BPoint *points, float *results, int32 count);
private:
	float	w;
	float	h;
	float	x0;
	float	y0;
	float	tentacles;	// an integral number
};

// SpiralCos2
class SpiralCos2 : public PostCalc
{
public:
	SpiralCos2(float cw, float ch, float cx0, float cy0, float crad,
		float ctentacles = 2) : PostCalc()
		{ w = cw; h = ch; x0 = cx0; y0 = cy0; rad = crad;
			tentacles = ctentacles; };
	SpiralCos2(float cw, float ch) : PostCalc()
	{
		w = cw; h = ch;
		x0 = float(rand()) / RAND_MAX * (w - 1);
		y0 = float(rand()) / RAND_MAX * (h - 1);
		tentacles = ((rand() % 2) * 2 - 1) * (rand() % 19 + 4);
		rad = float(rand()) / RAND_MAX * 9 + 1;
	};
	virtual void CalculatePoints(BPoint *points, float *results, int32 count);
private:
	float	w;
	float	h;
	float	x0;
	float	y0;
	float	tentacles;	// an integral number
	float	rad;
};

// SpiralLin
class SpiralLin : public PostCalc
{
public:
	SpiralLin(float cw, float ch, float cx0, float cy0,
		float ctentacles = 2) : PostCalc()
		{ w = cw; h = ch; x0 = cx0; y0 = cy0; tentacles = ctentacles; };
	SpiralLin(float cw, float ch) : PostCalc()
	{
		w = cw; h = ch;
		x0 = float(rand()) / RAND_MAX * (w - 1);
		y0 = float(rand()) / RAND_MAX * (h - 1);
		tentacles = ((rand() % 2) * 2 - 1) * (rand() % 19 + 4);
	};
	virtual void CalculatePoints(BPoint *points, float *results, int32 count);
private:
	float	w;
	float	h;
	float	x0;
	float	y0;
	float	tentacles;	// an integral number
};

// SpiralLin2
class SpiralLin2 : public PostCalc
{
public:
	SpiralLin2(float cw, float ch, float cx0, float cy0, float crad,
		float ctentacles = 2) : PostCalc()
		{ w = cw; h = ch; x0 = cx0; y0 = cy0; rad = crad;
			tentacles = ctentacles; };
	SpiralLin2(float cw, float ch) : PostCalc()
	{
		w = cw; h = ch;
		x0 = float(rand()) / RAND_MAX * (w - 1);
		y0 = float(rand()) / RAND_MAX * (h - 1);
		tentacles = ((rand() % 2) * 2 - 1) * (rand() % 19 + 4);
		rad = float(rand()) / RAND_MAX * 9 + 1;
	};
	virtual void CalculatePoints(BPoint *points, float *results, int32 count);
private:
	float	w;
	float	h;
	float	x0;
	float	y0;
	float	tentacles;	// an integral number
	float	rad;
};

// Snail
class Snail : public PostCalc
{
public:
	Snail(float cw, float ch, float cx0, float cy0, float cfak) : PostCalc()
		{ w = cw; h = ch; x0 = cx0; y0 = cy0; fak = cfak; };
	Snail(float cw, float ch) : PostCalc()
	{
		w = cw; h = ch;
		x0 = float(rand()) / RAND_MAX * (w - 1);
		y0 = float(rand()) / RAND_MAX * (h - 1);
		fak = ((rand() % 2) * 2 - 1) * (float(rand()) / RAND_MAX * 1.9 + 0.1);
	};
	virtual void CalculatePoints(BPoint *points, float *results, int32 count);
private:
	float	w;
	float	h;
	float	x0;
	float	y0;
	float	fak;
};

// Grid
class Grid : public PostCalc
{
public:
	Grid(float cx0, float cy0, float chor, float cver,
		float cweight = 1) : PostCalc()
		{ x0 = cx0; y0 = cy0; hor = chor; ver = cver; weight = cweight; };
	Grid(float w, float h) : PostCalc()
	{
		x0 = float(rand()) / RAND_MAX * (w - 1);
		y0 = float(rand()) / RAND_MAX * (h - 1);
		hor = float(rand()) / RAND_MAX * 385 + 15;
		ver = float(rand()) / RAND_MAX * 385 + 15;
		weight = float(rand()) / RAND_MAX * 0.5 + 0.05;
	};
	virtual void CalculatePoints(BPoint *points, float *results, int32 count);
private:
	float	x0;
	float	y0;
	float	hor;
	float	ver;
	float	weight;
};

// Field
class Field : public PostCalc
{
public:
	Field(float cx0, float cy0, float cw, float ch, float chor,
		float cver, float cweight = 1) : PostCalc()
		{ x0 = cx0; y0 = cy0; w = cw; h = ch; hor = chor; ver = cver;
			weight = cweight; };
	Field(float cw, float ch) : PostCalc()
	{
		w = cw; h = ch;
		x0 = float(rand()) / RAND_MAX * (w - 1);
		y0 = float(rand()) / RAND_MAX * (h - 1);
		hor = float(rand()) / RAND_MAX * 29 + 1;
		ver = float(rand()) / RAND_MAX * 29 + 1;
		weight = float(rand()) / RAND_MAX * 0.5 + 0.05;
	};
	virtual void CalculatePoints(BPoint *points, float *results, int32 count);
private:
	float	x0;
	float	y0;
	float	w;
	float	h;
	float	hor;
	float	ver;
	float	weight;
};

// Waves
class Waves : public PostCalc
{
public:
	Waves (float cx0, float cy0, float cw, float ch, float chor,
		float cver) : PostCalc()
		{ x0 = cx0; y0 = cy0; w = cw, h = ch; hor = chor; ver = cver; };
	Waves (float cw, float ch) : PostCalc()
	{
		w = cw, h = ch;
		x0 = float(rand()) / RAND_MAX * (w - 1);
		y0 = float(rand()) / RAND_MAX * (h - 1);
		hor = ((rand() % 2) * 2 - 1) * (float(rand()) / RAND_MAX * 19 + 1);
		ver = ((rand() % 2) * 2 - 1) * (float(rand()) / RAND_MAX * 19 + 1);
	};
	virtual void CalculatePoints(BPoint *points, float *results, int32 count);
private:
	float	x0;
	float	y0;
	float	w;
	float	h;
	float	hor;
	float	ver;
};

// CircleCos
class CircleCos : public PostCalc
{
public:
	CircleCos(float cx0, float cy0, float cw, float ch,
		float cstretch = 1) : PostCalc()
		{ x0 = cx0; y0 = cy0; stretch = cstretch;
			maxr = GetMaxR(cw, ch, x0, y0); };
	CircleCos(float w, float h) : PostCalc()
	{
		x0 = float(rand()) / RAND_MAX * (w - 1);
		y0 = float(rand()) / RAND_MAX * (h - 1);
		stretch = ((rand() % 2) * 2 - 1)
			* (float(rand()) / RAND_MAX * 14.5 + 0.5);
		maxr = GetMaxR(w, h, x0, y0);
	};
	virtual void CalculatePoints(BPoint *points, float *results, int32 count);
private:
	float	x0;
	float	y0;
	float	maxr;
	float	stretch;
};

// CircleLin
class CircleLin : public PostCalc
{
public:
	CircleLin(float cx0, float cy0, float cw, float ch,
		float cstretch = 1) : PostCalc()
		{ x0 = cx0; y0 = cy0; stretch = cstretch;
			maxr = GetMaxR(cw, ch, x0, y0); };
	CircleLin(float w, float h) : PostCalc()
	{
		x0 = float(rand()) / RAND_MAX * (w - 1);
		y0 = float(rand()) / RAND_MAX * (h - 1);
		stretch = ((rand() % 2) * 2 - 1)
			* (float(rand()) / RAND_MAX * 14.5 + 0.5);
		maxr = GetMaxR(w, h, x0, y0);
	};
	virtual void CalculatePoints(BPoint *points, float *results, int32 count);
private:
	float	x0;
	float	y0;
	float	maxr;
	float	stretch;
};

// WheelCos
class WheelCos : public PostCalc
{
public:
	WheelCos(float cx0, float cy0, float ctentacles) : PostCalc()
		{ x0 = cx0; y0 = cy0; tentacles = ctentacles; };
	WheelCos(float w, float h) : PostCalc()
	{
		x0 = float(rand()) / RAND_MAX * (w - 1);
		y0 = float(rand()) / RAND_MAX * (h - 1);
		tentacles = ((rand() % 2) * 2 - 1) * (rand() % 19 + 4);
	};
	virtual void CalculatePoints(BPoint *points, float *results, int32 count);
private:
	float	x0;
	float	y0;
	float	tentacles;
};

// WheelLin
class WheelLin : public PostCalc
{
public:
	WheelLin(float cx0, float cy0, float ctentacles) : PostCalc()
		{ x0 = cx0; y0 = cy0; tentacles = ctentacles; };
	WheelLin(float w, float h) : PostCalc()
	{
		x0 = float(rand()) / RAND_MAX * (w - 1);
		y0 = float(rand()) / RAND_MAX * (h - 1);
		tentacles = ((rand() % 2) * 2 - 1) * (rand() % 19 + 4);
	};
	virtual void CalculatePoints(BPoint *points, float *results, int32 count);
private:
	float	x0;
	float	y0;
	float	tentacles;
};

// Hole
class Hole : public PostCalc
{
public:
	Hole(float cx0, float cy0, float cw, float ch,
		float cstretch, float cdir) : PostCalc()
		{ x0 = cx0; y0 = cy0; stretch = cstretch; dir = cdir;
			maxr = GetMaxR(cw, ch, x0, y0); };
	Hole(float w, float h) : PostCalc()
	{
		x0 = float(rand()) / RAND_MAX * (w - 1);
		y0 = float(rand()) / RAND_MAX * (h - 1);
		stretch = rand() % 5 + 1; //stretch *=stretch;
		dir = (rand() % 2) * 2 - 1;
		maxr = GetMaxR(w, h, x0, y0);
	};
	virtual void CalculatePoints(BPoint *points, float *results, int32 count);
private:
	float	x0;
	float	y0;
	float	maxr;
	float	stretch;
	float	dir;
};



#endif	// FUNCTIONS_H

