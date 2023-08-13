#pragma once
#include <vector>
#include <string>
#include <SDL.h>
#include <SDL_keycode.h>
#include <SDL_audio.h>

using namespace std;

class Channel
{
public:
	vector<string> Rows;

	Sint16 AudioData;
	
	int MarginLeft, MarginRight, MarginTop, MarginBottom;
	int Index;
	int Tick;
	int ChannelLength;
	int WaveType;
	int EffectsSize;
	int Volume;
	int ChannelPatternIndex;

	bool IsActive;

	void SetUp(int Length);
	void TickCheck();
};

/*
rows should be done like this

3 chars for note
2 chars for volume
3 chars for each effect command line

visual example
C-4 7F 30A

Empty parts can be represented with -

Off commands can be shown as =

May or may not keep hexidecimal

Audio bytes should be sent to an accumulator to then play every channels sound
*/