// Calculation.cpp

#include "Calculation.h"

// PreCalcList

// constructor
PreCalcList::PreCalcList()
			: BList(5)
{}

// destructor
PreCalcList::~PreCalcList()
{
	// delete list entries
	for (int i = 0; PreCalc *item = (PreCalc *)ItemAt(i); i++)
		delete item;
}


// PostCalc

// contructor
PostCalc::PostCalc()
		: CmdThread("postcalc", B_LOW_PRIORITY)
{
	Resume();
}

// destructor
PostCalc::~PostCalc()
{}

// CmdThreadFunction
int32 PostCalc::CmdThreadFunction()
{
	bool terminate = false;
	while (!terminate)
	{
		Command *cmd = ReceiveCommand();
		switch (cmd->code)
		{
			case CMD_TERMINATE:
				terminate = true;
				break;
			case CMD_POST_CALCULATE:
			{
				CmdPostCalculate *postcmd;
				if (postcmd = dynamic_cast<CmdPostCalculate *>(cmd))
				{
					CalculatePoints(postcmd->points, postcmd->results,
						postcmd->count);
				}
				break;
			}
		}
		CommandProcessed(cmd);
	}
	return 0;
};
