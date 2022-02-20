#pragma once
#include <ImgUtil.h>
#include "Channel.h"
#include <iostream>
#include <SDL_keyboard.h>
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
	void CheckInput();
};