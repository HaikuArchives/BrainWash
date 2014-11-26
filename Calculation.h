// Calculation.h

#ifndef CALCULATION_H
#define CALCULATION_H

#include <kernel/OS.h>
#include <support/List.h>
#include "Thread.h"

// PreCalc - the base class for precalculation
class PreCalc
{
public:
	PreCalc() {};
	virtual ~PreCalc() {};
	virtual void CalculatePoints(BPoint *points, BPoint *results, int32 count)
		= 0;
};

// PreCalcList
class PreCalcList : public BList
{
public:
	PreCalcList();
	~PreCalcList();
	void CalculatePoints(BPoint *points, BPoint *results, int32 count)
	{
		for (int i = 0; PreCalc *item = (PreCalc *)ItemAt(i); i++)
		{
			item->CalculatePoints(points, results, count);
			points = results;
		}
	};
};

// PostCalc - the base class for postcalculation
class PostCalc : public CmdThread
{
public:
	PostCalc();
	virtual ~PostCalc();
	virtual int32 CmdThreadFunction();
protected:
	virtual void CalculatePoints(BPoint *points, float *results, int32 count)
		= 0;
};


#endif	// CALCULATION_H

