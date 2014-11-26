// BrainWash.h

#ifndef BRAIN_WASH_H
#define	BRAIN_WASH_H

#include <add-ons/screen_saver/ScreenSaver.h>

class BMessage;
class DrawThread;
class CycleThread;
class BrainWindowScreen;

// the screen saver class
class BrainWashSaver : public BScreenSaver
{
public:
	BrainWashSaver(BMessage *msg, image_id id);
	virtual status_t StartSaver(BView *view, bool preview);
	virtual void StopSaver(void);
	virtual void StartConfig(BView *configView);
	virtual void Draw(BView *view, int32 frame);
	virtual status_t SaveState(BMessage *state) const;
	~BrainWashSaver();
private:
	BrainWindowScreen	*screen;
};

// BrainWash 1.2 Settings (default values in BrainWash.cpp)
struct Settings
{
	int32	cycle_speed;		// 1 ... 10
	int32	drawing_gap;		// 0 ... 10 sec
	int32	drawing_speed;		// 1 ... 9
	uint32	screen_resolution;
};

extern Settings *settings;


#endif	// BRAIN_WASH_H
