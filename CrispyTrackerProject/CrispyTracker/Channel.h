#pragma once
#ifndef Channel

#include <iostream>
#include <vector>
#include <string>
#include <SDL.h>
#include <SDL_keycode.h>
#include <SDL_audio.h>

using namespace std;

class Channel
{
public:
	string NoteNames[12] = {
	"C-", "C#", "D-", "D#",
	"E-", "F-", "F#", "G-",
	"G#", "A-", "A#", "B-",
	};

	string NoteNames_GR[12] = {
	"C-", "C#", "D-", "D#",	
	"E-", "F-", "F#", "G-",
	"G#", "A-", "B", "H-",
	};

	string NoteNames_FL[12] = {
	"C-", "Db", "D-", "Eb",
	"E-", "F-", "Gb", "G-",
	"Ab", "A-", "Bb", "B-",
	};

	struct Row {
		int note = 255;
		int octave = 0;
		int instrument = 255;
		int volume = 255;
		int effect = 255;
		int effectvalue = 255;
	};
	
	//Visible rows
	vector<Row> Rows;

	Sint16 AudioData;
	int MarginLeft, MarginRight, MarginTop, MarginBottom;
	int Index;
	int Tick;
	int ChannelLength;
	int WaveType;
	int Volume;
	int ChannelPatternIndex;

	bool IsActive;

	void SetUp(int Length);
	void TickCheck();

	string Row_View(int index);
	string NoteView(int index);
	string VolumeView(int index);
	string InstrumentView(int index);
	string EffectView(int index);
	string Effectvalue(int index);
};

/*
rows should be done like this

3 chars for note
2 chars for volume
3 chars for each effect command line

visual example
C-4 7F 04 30A

Empty parts can be represented with -

Off commands can be shown as =

May or may not keep hexidecimal

Audio bytes should be sent to an accumulator to then play every channels sound
*/

#endif // !Channel