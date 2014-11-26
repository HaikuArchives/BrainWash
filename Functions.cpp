// Functions.cpp

#include <support/List.h>
#include <kernel/OS.h>
#include <stdlib.h>
#include <stdio.h>
#include "BrainWash.h"
#include "Functions.h"

#define pi		3.14159265
#define pi2		(pi * 2)
#define pih		(pi / 2)


// precalculation

// Func1
void Func1::CalculatePoints(BPoint *points, BPoint *results, int32 count)
{
	for (int32 i = 0; i < count; i++)
	{
		results[i].x = (1 - cos(points[i].x / w * pi)) / 2 * w;
		results[i].y = (1 - cos(points[i].y / h * pi)) / 2 * h;
	}
}

// Lens1
void Lens1::CalculatePoints(BPoint *points, BPoint *results, int32 count)
{
	for (int32 i = 0; i < count; i++)
	{
		float x = points[i].x, y = points[i].y;
		float dx = x0 - x, dy = y0 - y;
		float r = pow(abs(cos(
			sqrt(dx * dx + dy * dy) / (maxr + 0.01) * pi / 2)), loc) * inf;
		results[i].x = x + dx * r;
		results[i].y = y + dy * r;
	}
}

// Twist
void Twist::CalculatePoints(BPoint *points, BPoint *results, int32 count)
{
	for (int32 i = 0; i < count; i++)
	{
		float dx = x0 - points[i].x, dy = y0 - points[i].y;
		float dist = sqrt(dx * dx + dy * dy), r = pih;
		if (dist)
			r = asin(dy / (dist * 1.000001));
		if (dx > 0)
			r = pi - r;
		r += sqrt(dist / (1120)) * pi2;
		results[i].x = x0 + cos(r) * dist;
		results[i].y = y0 + sin(r) * dist;
	}
}


// postcalculation

// SpiralCos
void SpiralCos::CalculatePoints(BPoint *points, float *results, int32 count)
{
	for (int32 i = 0; i < count; i++)
	{
		float dx = x0 - points[i].x, dy = y0 - points[i].y;
		float dist = sqrt(dx * dx + dy * dy), r = pih;
		if (dist)
			r = asin(dy / (dist * 1.000001));
		if (dx > 0)
			r = pi - r;
		r = (r + sqrt(dist / (w + h)) * 3) / pi2 * tentacles;
		r -= floor(r);
		float m = (cos(r * pi2) + 1) * 0.4999;
		results[i] = ((r < 0.5) ? -m : m);
	}
}

// SpiralCos2
void SpiralCos2::CalculatePoints(BPoint *points, float *results, int32 count)
{
	for (int32 i = 0; i < count; i++)
	{
		float dx = x0 - points[i].x, dy = y0 - points[i].y;
		float dist = sqrt(dx * dx + dy * dy), r = pih;
		if (dist)
			r = asin(dy / (dist * 1.000001));
		if (dx > 0)
			r = pi - r;
		r = (r + sqrt(rad / (rad + dist)) * 3) / pi2 * tentacles;
		r -= floor(r);
		float m = (cos(r * pi2) + 1) * 0.4999;
		results[i] = ((r < 0.5) ? -m : m);
	}
}

// SpiralLin
void SpiralLin::CalculatePoints(BPoint *points, float *results, int32 count)
{
	for (int32 i = 0; i < count; i++)
	{
		float dx = x0 - points[i].x, dy = y0 - points[i].y;
		float dist = sqrt(dx * dx + dy * dy), r = pih;
		if (dist)
			r = asin(dy / (dist * 1.000001));
		if (dx > 0)
			r = pi - r;
		r = (r + sqrt(dist / (w + h)) * 3) / pi2 * tentacles;
		results[i] = (r - floor(r) - 0.5) * 1.999;
	}
}

// SpiralLin2
void SpiralLin2::CalculatePoints(BPoint *points, float *results, int32 count)
{
	for (int32 i = 0; i < count; i++)
	{
		float dx = x0 - points[i].x, dy = y0 - points[i].y;
		float dist = sqrt(dx * dx + dy * dy), r = pih;
		if (dist)
			r = asin(dy / (dist * 1.000001));
		if (dx > 0)
			r = pi - r;
		r = (r + sqrt(rad / (rad + dist)) * 3) / pi2 * tentacles;
		results[i] = (r - floor(r) - 0.5) * 1.999;
	}
}

// Snail
void Snail::CalculatePoints(BPoint *points, float *results, int32 count)
{
	for (int32 i = 0; i < count; i++)
	{
		float dx = x0 - points[i].x, dy = y0 - points[i].y;
		float dist = sqrt(dx * dx + dy * dy), r = pih;
		if (dist)
			r = asin(dy / (dist * 1.000001));
		if (dx > 0)
			r = pi - r;
		r = (r + sqrt(dist / (w + h)) * 100 * fak) / pi2;
		results[i] = (r - floor(r) - 0.5) * 1.999;
	}
}

// Grid
void Grid::CalculatePoints(BPoint *points, float *results, int32 count)
{
	for (int32 i = 0; i < count; i++)
	{
		float xx = (points[i].x - x0) / hor;
		float yy = (points[i].y - y0) / ver;
		xx -= floor(xx);
		yy -= floor(yy);
		if (xx > 0.5)
			xx = 1 - xx;
		if (yy > 0.5)
			yy = 1 - yy;
		xx = sin(pi2 * xx);
		yy = sin(pi2 * yy);
		results[i] = (1 - xx * yy) * weight;
	}
}

// Field
void Field::CalculatePoints(BPoint *points, float *results, int32 count)
{
	for (int32 i = 0; i < count; i++)
	{
		results[i] = (cos(points[i].x * hor / w * pi)
			+ cos(points[i].y * ver / h * pi) + 2) / 4 * weight;
	}
}

// Waves
void Waves::CalculatePoints(BPoint *points, float *results, int32 count)
{
	for (int32 i = 0; i < count; i++)
	{
		float z = ((points[i].x - x0) * hor
			+ (points[i].y - y0) * ver) / (w + h);
		results[i] = (z - 0.5) * 1.999;
	}
}

// CircleCos
void CircleCos::CalculatePoints(BPoint *points, float *results, int32 count)
{
	for (int32 i = 0; i < count; i++)
	{
		float dx = x0 - points[i].x, dy = y0 - points[i].y;
		float r = sqrt(dx * dx + dy * dy) / (maxr + 0.1) * stretch;
		r -= floor(r);
		float m = (cos(r * pi2) + 1) * 0.4999;
		results[i] = ((r < 0.5) ? -m : m);
	}
}

// WheelCos
void WheelCos::CalculatePoints(BPoint *points, float *results, int32 count)
{
	for (int32 i = 0; i < count; i++)
	{
		float dx = x0 - points[i].x, dy = y0 - points[i].y;
		float dist = sqrt(dx * dx + dy * dy), r = pih;
		if (dist)
			r = asin(dy / dist);
		if (dx > 0)
			r = pi - r;
		r = r / pi2 * tentacles;
		r -= floor(r);
		float m = (cos(r * pi2) + 1) * 0.4999;
		results[i] = ((r < 0.5) ? -m : m);
	}
}

// WheelLin
void WheelLin::CalculatePoints(BPoint *points, float *results, int32 count)
{
	for (int32 i = 0; i < count; i++)
	{
		float dx = x0 - points[i].x, dy = y0 - points[i].y, r = pih;
		float dist = sqrt(dx * dx + dy * dy);
		if (dist)
			r = asin(dy / dist);
		if (dx > 0)
			r = pi - r;
		r = r / pi2 * tentacles;
		results[i] = 1 - 2 * (r - floor(r));
	}
}

// CircleLin
void CircleLin::CalculatePoints(BPoint *points, float *results, int32 count)
{
	for (int32 i = 0; i < count; i++)
	{
		float dx = x0 - points[i].x, dy = y0 - points[i].y;
		float r = sqrt(dx * dx + dy * dy) / (maxr + 0.1) * stretch;
		results[i] = (r - floor(r) - 0.5) * 1.999;
	}
}

// Hole
void Hole::CalculatePoints(BPoint *points, float *results, int32 count)
{
	for (int32 i = 0; i < count; i++)
	{
		float dx = x0 - points[i].x, dy = y0 - points[i].y;
		float r = sqrt(dx * dx + dy * dy) / (maxr + 0.1) * stretch;
		r = dir * (0.1 / (r + 0.1));
		results[i] = (r - floor(r) - 0.5) * 1.999;
	}
}

