#pragma once
#include "Channel.h"
#include <iostream> 
class Tracker
{
public:
	int TickLimit;
	int Length;
	int YPos;
	Channel Channels[];
	void Run();
	void CheckNotes(Channel Channels[]);
	void TickAlong(Channel Channels[], int tick);
};

