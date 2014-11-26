// Threads.cpp

#include <interface/GraphicsDefs.h>
#include <interface/Screen.h>
#include <game/WindowScreen.h>
#include <stdlib.h>
#include <stdio.h>
#include "Threads.h"
#include "BrainWash.h"
#include "Functions.h"
#include "Dimensions.h"
#include "Drawing.h"

// the cycle thread

// constructor
CycleThread::CycleThread(BWindowScreen *cscreen)
			: CmdThread("cycle colors", B_NORMAL_PRIORITY)
{
	screen = cscreen;
//printf("Screen erzeugt, oder was auch immer...\n");
	cycle_2nd_palette = false;
	Resume();
}


inline void CalcColor(rgb_color src, rgb_color dest, rgb_color &cur,
	int steps, int curstep)
{
	cur.red = ((dest.red - src.red) * curstep) / steps + src.red;
	cur.green = ((dest.green - src.green) * curstep) / steps
		+ src.green;
	cur.blue = ((dest.blue - src.blue) * curstep) / steps
		+ src.blue;
}

inline void CalcPalette(rgb_color *colors, rgb_color &col, int pos)
{
	rgb_color black = {0, 0, 0, 0};
	for (int i = 0; i < 64; i++)
		CalcColor(black, col, colors[i + pos], 63, i);
}

static void NewDest(rgb_color &src, rgb_color &dest, int &steps)
{
	const int minval = 382;
	src = dest;
	dest.red = rand() % 256;
	int left = (minval - 255) - dest.red;
	if (left < 0)
		left = 0;
	dest.green = (rand() % (256 - left)) + left;
	left = minval - dest.red - dest.green;
	if (left < 0)
		left = 0;
	dest.blue = (rand() % (256 - left)) + left;
	steps = max(abs(dest.red - src.red), abs(dest.green - src.green));
	steps = max(steps, abs(dest.blue - src.blue));
}

// the cycle function
int32 CycleThread::CmdThreadFunction()
{
	// set palette
	rgb_color *colors = new rgb_color[256];
	rgb_color *colors2 = new rgb_color[128];
	int offset = 0;
	rgb_color src1, dest1 = {0, 0, 0, 0}, cur1;
	rgb_color src2, dest2 = {0, 0, 0, 0}, cur2;
	int steps1 = 0, curstep1 = 0;
	int steps2 = 0, curstep2 = 0;
	colors[128] = dest2;	// initialize the background color with black

	// color cycling
	while (go)
	{
		if (offset % 16 == 0)
		{
			// palette 1
			while (curstep1 == steps1)
			{
				NewDest(src1, dest1, steps1);
				curstep1 = 0;
			}

			CalcColor(src1, dest1, cur1, steps1, curstep1);
			CalcPalette(colors2, cur1, 0);
			curstep1++;

			// palette 2
			while (curstep2 == steps2)
			{
				NewDest(src2, dest2, steps2);
				curstep2 = 0;
			}
			CalcColor(src2, dest2, cur2, steps2, curstep2);
			CalcPalette(colors2, cur2, 64);
			curstep2++;
		}

		int i;
		for (i = 0; i < 64; i++)
			colors[(offset + 127 - i) % 128]
				= colors[(offset + i) % 128] = colors2[i];

		if (cycle_2nd_palette)
		{
			for (i = 0; i < 64; i++)
				colors[((offset + 127 - i) % 128) + 128]
					= colors[((offset + i) % 128) + 128] = colors2[i + 64];
		}
//printf("SetColorList()\n");
		screen->SetColorList(colors);
//printf("Farben gesetzt.\n");
		offset = (offset + 1) % 128;
		snooze((11 - settings->cycle_speed) * 500);
	}
	delete[] colors2;
	delete[] colors;
	return B_OK;
}


// the drawing thread

// constructor
DrawThread::DrawThread(BWindowScreen *cscreen, CycleThread *ccycle_thread,
	uint32 space)
			: CmdThread("cycle colors", B_LOW_PRIORITY)
{
	screen = cscreen;
	cycle_thread = ccycle_thread;
	color_space = space;
	Resume();
}

// the drawing function
int32 DrawThread::CmdThreadFunction()
{
	bool terminate = false;
	graphics_card_info *cardinfo = screen->CardInfo();
	frame_buffer_info *fbi = screen->FrameBufferInfo();
	char *fb = (char *)cardinfo->frame_buffer;
	int32 wi = 640, hi = 480;
	get_dimensions_for(color_space, &wi, &hi);
	float w = wi, h = hi;

	// clear screen
	ClearScreen(fb, wi, hi, 128);

	// draw
	bool drawing_done = true;
	Function *func[2];
	func[0] = new Function(fb, w, h, 0);
	func[1] = new Function(fb, w, h, 128);
	int curfunc = 0;
	bool notified = false;	// to tell the cycle thread when to use the 2nd palette

	while (!terminate)
	{
		int x0 = float(rand()) / RAND_MAX * (w / 2 - 1) + w / 4;
		int y0 = float(rand()) / RAND_MAX * (h / 2 - 1) + h / 4;
		func[curfunc]->Init();
		func[curfunc]->SetXY(x0, y0);
		SendCommand(func[curfunc], new CmdDrawFunction);
		drawing_done = false;
		// wait for the reply
		while (!drawing_done)
		{
			Command *cmd = ReceiveCommand();
			switch (cmd->code)
			{
				case CMD_TERMINATE:
					terminate = true;
					// tell the current function to terminate
					SendCommand(func[curfunc], new CmdTerminate);
					break;
				case CMD_REPLY:
				{
					CmdReply *reply = dynamic_cast<CmdReply *>(cmd);
					if (!reply)
						break;
					switch (reply->reply_code)
					{
						case CMD_DRAW_FUNCTION:
							drawing_done = true;
							break;
					}
					break;
				}
			}
			CommandProcessed(cmd);
		}
		for (int i = 0; i < 100 * settings->drawing_gap && !terminate; i++)
		{
			// check for a terminate command
			if (Command *cmd = CheckForCommand())
			{
				if (cmd->code == CMD_TERMINATE)
					terminate = true;
				CommandProcessed(cmd);
			}
			if (!terminate)
				snooze(10000);
		}
		curfunc = 1 - curfunc;
		if (!notified)
		{
			notified = true;
			cycle_thread->cycle_2nd_palette = true;
		}
	}
	func[0]->Die();
	func[1]->Die();
	delete func[0];
	delete func[1];
	return 0;
}


// Function

// constructor
Function::Function(char *cfb, float cw, float ch, int cstartcol)
		: CmdThread("Function", B_LOW_PRIORITY)
{
	fb = cfb;
	w = cw;
	h = ch;
	startcol = cstartcol;
	prelist = new PreCalcList;
	postlist = new BList(10);
	Resume();
}

// destructor
Function::~Function()
{
	// delete prelist
	delete prelist;
	// empty and delete postlist
	for (int i = 0; PostCalc *item = (PostCalc *)postlist->ItemAt(i); i++)
		delete item;
	delete postlist;
}

// create calculation objects randomly
void Function::Init()
{
	// empty prelist
	while (PreCalc *item = (PreCalc *)prelist->ItemAt(0))
	{
		prelist->RemoveItem((long)0);
		delete item;
	}
	// empty postlist
	while (PostCalc *item = (PostCalc *)postlist->ItemAt(0))
	{
		postlist->RemoveItem((long)0);
		delete item;
	}
	// precalc functions
	int num = rand() % 2 + 1;				// 1...2 functions
	for (int i = 0, numf1 = 1, numle1 = 1, numtw = 1; i < num; i++)
	{
		int choice = rand() + 1;
		while (choice >= 0)
		{
			choice %= PRE_FUNC_COUNT;
			switch (choice)
			{
				// Func1
				case FUNC1:
					if (numf1)
					{
						prelist->AddItem(new Func1(w, h));
						numf1--;
						choice = -1;
					}
					else
						choice++;
					break;
				// Lens1
				case LENS1:
					if (numle1)
					{
						prelist->AddItem(new Lens1(w, h));
						numle1--;
						choice = -1;
					}
					else
						choice++;
					break;
				// Twist
				case TWIST:
					if (numtw)
					{
						prelist->AddItem(new Twist(w, h));
						numtw--;
						choice = -1;
					}
					else
						choice++;
					break;
				default:
					return;
			}
		}
	}
	// postcalc functions
	num = rand() % 4 + 2;					// 2...5 functions
	for (int i = 0, numsp = 2, numgr = 1, numfi = 1, numwa = 1,
		numci = 2, numsn = 1, numho = 1; i < num; i++)
	{
		int choice = rand() + 1;
		while (choice >= 0)
		{
			choice %= POST_FUNC_COUNT;
			switch (choice)
			{
				// SPIRAL
				case SPIRAL:
					if (numsp)
					{
						switch (choice = rand() % SPIRAL_COUNT)
						{
							// SpiralCos
							case SPIRAL_COS:
							{
								postlist->AddItem(new SpiralCos(w, h));
								break;
							}
							// SpiralLin
							case SPIRAL_LIN:
							{
								postlist->AddItem(new SpiralLin(w, h));
								break;
							}
							// SpiralCos2
							case SPIRAL2_COS:
							{
								postlist->AddItem(new SpiralCos2(w, h));
								break;
							}
							// SpiralLin2
							case SPIRAL2_LIN:
							{
								postlist->AddItem(new SpiralLin2(w, h));
								break;
							}
							// WheelCos
							case WHEEL_COS:
							{
								postlist->AddItem(new WheelCos(w, h));
								break;
							}
							// WheelLin
							case WHEEL_LIN:
							{
								postlist->AddItem(new WheelLin(w, h));
								break;
							}
						}
						numsp--;
						choice = -1;
					}
					else
						choice++;
					break;	// SPIRAL
				// CIRCLE
				case CIRCLE:
					if (numci)
					{
						switch (choice = rand() % CIRCLE_COUNT)
						{
							// CircleCos
							case CIRCLE_COS:
							{
								postlist->AddItem(new CircleCos(w, h));
								break;
							}
							// CircleLin
							case CIRCLE_LIN:
							{
								postlist->AddItem(new CircleCos(w, h));
								break;
							}
						}
						numci--;
						choice = -1;
					}
					else
						choice++;
					break;	// CIRCLE
				// Hole
				case HOLE:
					if (numho)
					{
						postlist->AddItem(new Hole(w, h));
						numho--;
						choice = -1;
					}
					else
						choice++;
					break;
				// Snail
				case SNAIL:
					if (numsn)
					{
						postlist->AddItem(new Snail(w, h));
						numsn--;
						choice = -1;
					}
					else
						choice++;
					break;
				// Grid
				case GRID:
					if (numgr)
					{
						postlist->AddItem(new Grid(w, h));
						numgr--;
						choice = -1;
					}
					else
						choice++;
					break;
				// Waves
				case WAVES:
					if (numwa)
					{
						postlist->AddItem(new Waves(w, h));
						numwa--;
						choice = -1;
					}
					else
						choice++;
					break;
				// Field
				case FIELD:
					if (numgr)
					{
						postlist->AddItem(new Field(w, h));
						numgr--;
						choice = -1;
					}
					else
						choice++;
					break;
				default:
					return;
			}
		}
	}
}

// GetNextLine
int32 Function::GetNextLine(BPoint *points)
{
	int wi = w, he = h;
	int32 count = 0;
	if (t >= 0 || r < wi || b < he || l >= 0)
	{
		bool horiz = false, reverse = false;
		int32 x, y;
		while (!count)
		{
			side = (side + 1) % 4;
			switch (side)
			{
				// top
				case 0:
					if (t >= 0)
					{
						x = l + 1; y = t; count = r - l - 1;
						horiz = true;
						t--;
					}
					break;
				// right
				case 1:
					if (r < wi)
					{
						x = r; y = t + 1; count = b - t - 1;
						r++;
					}
					break;
				// bottom
				case 2:
					if (b < he)
					{
						x = l + 1; y = b; count = r - l - 1;
						horiz = true; reverse = true;
						b++;
					}
					break;
				// left
				case 3:
					if (l >= 0)
					{
						x = l; y = t + 1; count = b - t - 1;
						reverse = true;
						l--;
					}
					break;
			}
		}
		// init the points for precalculation functions
		if (horiz)
		{
			if (reverse)
			{
				float xx = x + count, yy = y;
				for (int i = 0; i < count; i++)
				{
					points[i].x = --xx;
					points[i].y = yy;
				}
			}
			else
			{
				float xx = x, yy = y;
				for (int i = 0; i < count; i++)
				{
					points[i].x = xx++;
					points[i].y = yy;
				}
			}
		}
		else
		{
			if (reverse)
			{
				float xx = x, yy = y + count;
				for (int i = 0; i < count; i++)
				{
					points[i].x = xx;
					points[i].y = --yy;
				}
			}
			else
			{
				float xx = x, yy = y;
				for (int i = 0; i < count; i++)
				{
					points[i].x = xx;
					points[i].y = yy++;
				}
			}
		}
	}
	return count;
}

// CmdThreadFunction
int32 Function::CmdThreadFunction()
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
			case CMD_DRAW_FUNCTION:
			{
				terminate = Draw();
				break;
			}
			default:
				break;
		}
		CommandProcessed(cmd);
	}
	return 0;
}

// draw function starting at (x0, y0)
bool Function::Draw()
{
	bool terminate = false;
	int32 postcount = postlist->CountItems();
	int32 wi = w;
	int32 count, nextcount;
	int32 pause_count = 0;
	// allocate the points arrays
	BPoint *points[] = {
		(BPoint *) new char[max_pixels_per_line * sizeof(BPoint)],
		(BPoint *) new char[max_pixels_per_line * sizeof(BPoint)] };
	// allocate point arrays for precalculation
	BPoint *prearr[] = {
		(BPoint *) new char[max_pixels_per_line * sizeof(BPoint)],
		(BPoint *) new char[max_pixels_per_line * sizeof(BPoint)] };
	int32 curcmd = 0;
	// allocate commands for postcalculation
	CmdPostCalculate **postcmds = new CmdPostCalculate *[postcount];
	for (int32 i = 0; i < postcount; i++)
		postcmds[i] = new CmdPostCalculate(max_pixels_per_line);
	// the first line - we start with the right side (=> bottom + 1)
	side = 0;
	l = x0 - 1; r = x0; t = y0 - 1; b = y0 + 1;
	if (nextcount = GetNextLine(points[1 - curcmd]))
	{
		prelist->CalculatePoints(points[1 - curcmd],
		prearr[1 - curcmd], nextcount);
	}
	// while there is a next line
	while (nextcount && !terminate)
	{
		pause_count = (pause_count + 1) % 4;
		curcmd = 1 - curcmd;
		count = nextcount;
		// init and send the postcalculation commands
		for (int i = 0; i < postcount; i++)
		{
			postcmds[i]->points = prearr[curcmd];
			postcmds[i]->count = count;
			SendCommand((PostCalc *)postlist->ItemAt(i), postcmds[i]);
		}

		// try to get the next line
		if (nextcount = GetNextLine(points[1 - curcmd]))
		{
			prelist->CalculatePoints(points[1 - curcmd],
			prearr[1 - curcmd], nextcount);
		}

		float *results = 0;
		int postcalcs_done = 0;
		// wait for the postcalculation results
		while (postcalcs_done < postcount)
		{
			Command *cmd = ReceiveCommand();
			switch (cmd->code)
			{
				case CMD_TERMINATE:
					terminate = true;
					break;
				case CMD_REPLY:
				{
					CmdReply *reply = dynamic_cast<CmdReply *>(cmd);
					if (!reply || !reply->reply)
						break;
					switch (reply->reply->code)
					{
						case CMD_POST_CALCULATE:
						{
							if (CmdPostCalculate *postcmd
								= dynamic_cast<CmdPostCalculate *>(reply->reply))
							{
								postcalcs_done++;
								// take the results array of the first
								// returning command further calculations
								if (!results)
									results = postcmd->results;
								else
								{
									// calculate
									for (int i = 0; i < count; i++)
										results[i] += postcmd->results[i];
								}
							}
							break;
						}
					}
					break;
				}
			}
			CommandProcessed(cmd);
		}
		if (terminate)
			break;
		// calculate color and write pixels
		for (int i = 0; i < count; i++)
		{
			int32 col = floor(results[i] * 64 + 0.5);
			col = ((col % 128) + 128) % 128;
			WritePixel(fb, points[curcmd][i].x, points[curcmd][i].y,
				col + startcol, wi);
		}
		// wait a little bit
		if (!(pause_count & 3))
			if (11 - settings->drawing_speed)
				snooze(2000 * (10 - settings->drawing_speed));
	}

	// delete post commands
	for (int32 i = 0; i < postcount; i++)
		delete postcmds[i];
	delete[] postcmds;
	// delete precalc arrays
	delete[] (char *)prearr[0];
	delete[] (char *)prearr[1];
	delete[] (char *)points[0];
	delete[] (char *)points[1];
	return terminate;
}

