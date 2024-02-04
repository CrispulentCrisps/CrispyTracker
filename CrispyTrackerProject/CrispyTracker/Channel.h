#pragma once
#ifndef Channel

#include "Instrument.h"
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
	int MAX_VALUE = 256;
	string NoteNames[12] = {
	"C-", "C#", "D-", "D#",
	"E-", "F-", "F#", "G-",
	"G#", "A-", "A#", "B-",
	};

	string NoteNames_MT[12] = {
	"c-", "c#", "d-", "d#",
	"e-", "F-", "f#", "g-",
	"g#", "a-", "a#", "b-",
	};

	string NoteNames_GR[12] = {
	"C-", "C#", "D-", "D#",	
	"E-", "F-", "F#", "G-",
	"G#", "A-", "B-", "H-",
	};

	string NoteNames_FL[12] = {
	"C-", "Db", "D-", "Eb",
	"E-", "F-", "Gb", "G-",
	"Ab", "A-", "Bb", "B-",
	};

	string HexValues[16] = {
		"0","1","2","3",
		"4","5","6","7",
		"8","9","A","B",
		"C","D","E","F",
	};

	struct Row {
		int note = 256;
		int octave = 0;
		int instrument = 256;
		int volume = 256;
		int effect = 256;
		int effectvalue = 256;
	};
	
	//Visible rows
	vector<Row> Rows;

	Sint16 AudioDataL;
	Sint16 AudioDataR;
	int MarginLeft, MarginRight, MarginTop, MarginBottom;
	int Index;
	int Tick;
	int ChannelLength;
	int WaveType;
	int Volume;
	int ChannelPatternIndex = 0;
	int CurrentSamplePointIndex = 0;
	int CurrentInstrument = 0;

	bool PlayingNote;
	bool IsActive;

	void SetUp(int Length);
	void TickCheck(int RowIndex, vector<Instrument>& inst);
	void UpdateChannel(vector<Instrument>& inst, vector<Sample>& samples);
	string NoteView(int index);
	string VolumeView(int index);
	string InstrumentView(int index);
	string EffectView(int index);
	string Effectvalue(int index);
	int EvaluateHexInput(int input, int index, int max, int valuetype);
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