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
	int Input;
	Channel Channels[];
	void Run();
	HANDLE CheckInput();
};