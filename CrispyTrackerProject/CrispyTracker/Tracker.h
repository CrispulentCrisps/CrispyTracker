#pragma once
#include <ImgUtil.h>
#include "Channel.h"
#include <iostream>
class Tracker
{
public:
	int TickLimit;
	int Length;
	int YPos;
	bool PlayingTrack;
	Channel Channels[];
	void Run();
	void CheckNotes(Channel Channels[]);
	void TickAlong(Channel Channels[], int tick);
};