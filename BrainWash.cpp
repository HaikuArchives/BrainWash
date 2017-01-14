// a ScreenSaver module by Stephan Assmus, Ingo Weinhold

#include "BrainWash.h"
#include "Dimensions.h"
#include "Functions.h"
#include "Threads.h"
#include <math.h>
#include <PopUpMenu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern "C" { 
_EXPORT BScreenSaver *instantiate_screen_saver(BMessage *msg, image_id id); 
}

// version
const char *cmd_version = "BrainWash 1.3.1";

// config message field names
const char *cycle_speed_name = "CYCLE_SPEED";
const char *drawing_gap_name = "DRAWING_GAP";
const char *drawing_speed_name = "DRAWING_SPEED";
const char *screen_resolution_name = "SCREEN_RESOLUTION";

// min/max/default values for settings
enum
{
	MIN_CYCLE_SPEED			= 1,
	MAX_CYCLE_SPEED			= 10,
	DEF_CYCLE_SPEED			= 5,

	MIN_DRAWING_GAP			= 0,
	MAX_DRAWING_GAP			= 10,
	DEF_DRAWING_GAP			= 5,

	MIN_DRAWING_SPEED		= 1,
	MAX_DRAWING_SPEED		= 9,
	DEF_DRAWING_SPEED		= 5,

	DEF_SCREEN_RESOLUTION	= B_8_BIT_640x480,
};

// a pointer to the global settings and the default settings
Settings *settings;
Settings defaults = { DEF_CYCLE_SPEED, DEF_DRAWING_GAP, DEF_DRAWING_SPEED,
					  DEF_SCREEN_RESOLUTION };


// helpful functions

// add a tab page to a tab view
inline BView *add_tab_page(BTabView *tv_settings, BRect frame,
						   const char *label)
{
	BTab *tab = new BTab();
	BView *view = new BView(frame, 0, B_FOLLOW_ALL, B_WILL_DRAW);
	view->SetViewColor(tv_settings->ViewColor());
	tv_settings->AddTab(view, tab);
	tab->SetLabel(label);
	return view;
}


// classes

// SimpleSlider

class SimpleSlider : public BSlider
{
public:
	SimpleSlider(BRect frame, const char *label, int min, int max, int def,
				 const char *limlabl, const char *limlabr,
				 int32 *target_value = 0)
		: BSlider(frame, 0, label, 0, min, max)
	{
		this->target_value = target_value;
		ResizeToPreferred();
		SetLimitLabels(limlabl, limlabr);
		SetHashMarkCount(max - min + 1);
		SetHashMarks(B_HASH_MARKS_BOTTOM);
		BSlider::SetValue(def);
	}

	virtual void SetValue(int32 value)
	{
		if (target_value)
			*target_value = value;
		BSlider::SetValue(value);
	}
private:
	int32 *target_value;
};


// ScreenResMenuField
// a simple class that lets the user set the screen resolution
class ScreenResMenuField : public BMenuField
{
public:
	ScreenResMenuField(BRect frame, const char *label, uint32 res,
					   uint32 *target_value = 0)
		: BMenuField(frame, NULL, label, new BPopUpMenu("resolution"))
	{
		this->target_value = target_value;
		BMenu *menu = Menu();
		BMenuItem *item;
		// the first item - current workspace dimension
		BMessage *msg = new BMessage(VALUE_CHANGED);
		msg->AddInt32("value", CURRENT_WORKSPACE_DIMENSIONS);
		item = new BMenuItem("workspace res.", msg);
		menu->AddItem(item);

		int32 mark = 0;
		// add an item for each supported resolution
		for (int32 i = 0; i < color_spaces_count; i++)
		{
			status_t error;
			BWindowScreen *screen = new BWindowScreen("test",
				color_spaces[i], &error);
			if (error != B_OK)
			{
				// not supported
				delete screen;
				screen = 0;
			}
			else
			{
				// supported - add a menu item
				delete screen;
				screen = 0;
				msg = new BMessage(VALUE_CHANGED);
				msg->AddInt32("value", color_spaces[i]);
				item = new BMenuItem(color_space_names[i], msg);
				menu->AddItem(item);
				if (res == color_spaces[i])
					mark = menu->IndexOf(item);
			}
		}
		// mark the current resolution
		if (item = menu->ItemAt(mark))
		{
			item->SetMarked(true);
			item->Message()->FindInt32("value", (int32 *)&resolution);
		}
	};
	~ScreenResMenuField() {};

	virtual void MessageReceived(BMessage *message)
	{
		switch (message->what)
		{
			case VALUE_CHANGED:
				message->FindInt32("value", (int32 *)&resolution);
				*target_value = resolution;
				break;
			default:
				BMenuField::MessageReceived(message);
				break;
		}
	};
	
	virtual void AttachedToWindow()
	{
		BMenu *menu = Menu();
		for (int32 i = 0; BMenuItem *item = menu->ItemAt(i); i++)
			item->SetTarget(this);
		BMenuField::AttachedToWindow();
	};

	uint32	resolution;
private:
	uint32 *target_value;

	// message what value - used if a value has changed
	enum { VALUE_CHANGED = 'BW01' };
};


// BrainWindowScreen
// work around BeDoc bug -- SetColorList() only callable after
//							successful screen connection
class BrainWindowScreen : public BWindowScreen
{
public:
	BrainWindowScreen(const char *name, uint32 cres,
					  status_t *error)
		: BWindowScreen(name, cres, error)
	{
		draw_thread = 0;
		cycle_thread = 0;
		res = cres;
	}

	virtual void ScreenConnected(bool connected);
private:
	DrawThread		*draw_thread;
	CycleThread		*cycle_thread;
	uint32			res;
};

// ScreenConnected
void BrainWindowScreen::ScreenConnected(bool connected)
{
	if (connected)
	{
		cycle_thread = new CycleThread(this);
		draw_thread = new DrawThread(this, cycle_thread, res);
	}
	else
	{
		// let the threads die and destroy them
		if (draw_thread)
		{
			draw_thread->Die();
			delete draw_thread;
			draw_thread = 0;
		}
		if (cycle_thread)
		{
			cycle_thread->Die();
			delete cycle_thread;
			cycle_thread = 0;
		}
	}
}


// BrainWashSaver - our main class

// constructor
BrainWashSaver::BrainWashSaver(BMessage *msg, image_id id)
			  : BScreenSaver(msg, id)
{
	screen = 0;
	// set settings defaults
	settings = new Settings;
	*settings = defaults;
	// get settings values from message
	if (msg->HasInt32(cycle_speed_name))
		msg->FindInt32(cycle_speed_name, &settings->cycle_speed);
	if (msg->HasInt32(drawing_gap_name))
		msg->FindInt32(drawing_gap_name, &settings->drawing_gap);
	if (msg->HasInt32(drawing_speed_name))
		msg->FindInt32(drawing_speed_name, &settings->drawing_speed);
	if (msg->HasInt32(screen_resolution_name))
		msg->FindInt32(screen_resolution_name,
					   (int32 *)&settings->screen_resolution);
	srand(real_time_clock());
}

// destructor
BrainWashSaver::~BrainWashSaver()
{
	delete settings;
}

// StartSaver
status_t BrainWashSaver::StartSaver(BView *view, bool preview)
{
	if (preview)
		return B_ERROR;
	return B_OK;
}

// StopSaver
void BrainWashSaver::StopSaver(void)
{
	// close the screen
	if (screen)
	{
		screen->Hide();
		screen->PostMessage(B_QUIT_REQUESTED, screen);
		screen = 0;
	}
}

// StartConfig
void BrainWashSaver::StartConfig(BView *configView)
{
	BTabView *tv_settings = 0;
	BView *page1, *page2;
	BRect frame, tabframe;

	// title
	tv_settings = new BTabView(configView->Bounds(), "tab_view");
	tv_settings->SetViewColor(configView->ViewColor());
	configView->AddChild(tv_settings);
	tabframe = tv_settings->Bounds();
	tabframe.right -= 2;
	tabframe.bottom -= tv_settings->TabHeight() + 2;

	page1 = add_tab_page(tv_settings, tabframe, "Credits");
	page2 = add_tab_page(tv_settings, tabframe, "Settings");

	tabframe.InsetBy(5, 5);

	// the first page
	frame = tabframe;
	frame.top += 5;
	frame.left += 7.0;
	frame.bottom = frame.top + 14.0;
	BStringView *strview = 
		new BStringView(frame, B_EMPTY_STRING, cmd_version);
	page1->AddChild(strview);
	strview->SetViewColor(configView->ViewColor());
	strview->SetFont(be_bold_font);
	// info text
	frame.top = frame.bottom + 7.0;
	frame.bottom = frame.top + 40.0;
	BRect textArea = frame;
	textArea.OffsetTo(B_ORIGIN);
	BTextView *caption = new BTextView(frame, B_EMPTY_STRING, textArea,
						   			   B_FOLLOW_ALL, B_WILL_DRAW);
	caption->SetText("© 1998/99 Stephan Aßmus, Ingo Weinhold\n"
					 "<gottgeac@linux.zrz.tu-berlin.de>\n"
					 "<bonefish@cs.tu-berlin.de>\n");
	page1->AddChild(caption);
	caption->SetViewColor(configView->ViewColor());
	caption->SetDrawingMode(B_OP_OVER);
	caption->SetFont(be_plain_font);
	caption->MakeEditable(FALSE);
	caption->MakeSelectable(TRUE);
	caption->SetWordWrap(TRUE);	

	// the second page
	frame = tabframe;
	// the cycle_speed slider
	frame.bottom = frame.top + 40.0;
	page2->AddChild(new SimpleSlider(frame, "cycle speed: ", MIN_CYCLE_SPEED,
		MAX_CYCLE_SPEED, settings->cycle_speed, "slow", "fast",
		&settings->cycle_speed));
	// the drawing_gap slider
	frame.top = frame.bottom + 10;
	frame.bottom = frame.top + 40.0;
	page2->AddChild(new SimpleSlider(frame, "drawing gap (secs): ",
		MIN_DRAWING_GAP, MAX_DRAWING_GAP, settings->drawing_gap, "0", "10",
		&settings->drawing_gap));
	// the drawing_speed slider
	frame.top = frame.bottom + 10;
	frame.bottom = frame.top + 40.0;
	page2->AddChild(new SimpleSlider(frame, "drawing speed: ",
		MIN_DRAWING_SPEED, MAX_DRAWING_SPEED, settings->drawing_speed, "slow",
		"fast", &settings->drawing_speed));
	// the screen resolution menu
	frame.top = frame.bottom + 10;
	frame.bottom = frame.top + 40.0;
	frame.right = frame.left + 200.0;
	page2->AddChild(new ScreenResMenuField(frame, "resolution:",
		settings->screen_resolution, &settings->screen_resolution));
	// select a page in the tab view
	tv_settings->Select(0);
}

// Draw
void BrainWashSaver::Draw(BView *view, int32 frame)
{
	if (frame == 0)
	{
		// if screen_blanker's BDirectWindow is in full screen mode, we cannot
		// open our BWindowScreen, so set it to window mode
		if (BDirectWindow *win = dynamic_cast<BDirectWindow *>(view->Window()))
			win->SetFullScreen(false);
		// find out wanted resolution
		uint32 res = settings->screen_resolution;
		if (res == CURRENT_WORKSPACE_DIMENSIONS)
		{
			// current resolution
			BRect frame = BScreen().Frame();
			res = color_space_for(frame.Width() + 1, frame.Height() + 1);
		}
		// create screen
		status_t error;
		screen = new BrainWindowScreen("saver screen", res, &error);
		if (error != B_OK)
		{
			delete screen;
			screen = 0;
		}
		else // let the show begin
			screen->Show();
	}
}

// SaveState
status_t BrainWashSaver::SaveState(BMessage *state) const
{
	state->AddInt32(cycle_speed_name, settings->cycle_speed);
	state->AddInt32(drawing_gap_name, settings->drawing_gap);
	state->AddInt32(drawing_speed_name, settings->drawing_speed);
	state->AddInt32(screen_resolution_name,
					(int32)settings->screen_resolution);
}


// instantiate_screen
BScreenSaver *instantiate_screen_saver(BMessage *msg, image_id id)
{
	return new BrainWashSaver(msg, id);
}

