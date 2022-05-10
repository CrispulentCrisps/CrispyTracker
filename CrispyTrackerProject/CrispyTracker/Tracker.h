#pragma once
#include <ImgUtil.h>
#include "Channel.h"
#include <iostream>
#include <SDL_keyboard.h>
#include <vector>

using namespace std;

class Tracker
{
public:
	int TickLimit;
	int Length;
	int YPos;
	bool PlayingTrack;
	int Input;
	vector<Channel> Channels;
	void Run();
	void CheckInput();
};