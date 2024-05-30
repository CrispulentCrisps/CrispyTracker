#pragma once
#ifndef Channel

#define RESAMPLE_QUALITY	20

#include "Macros.h"
#include "Instrument.h"
#include "Effects.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <SDL.h>
#include <SDL_keycode.h>
#include <SDL_audio.h>

using namespace std;

class Channel
{
public:
	int MAX_VALUE = NULL_COMMAND;
	
	//Sharp style notation
	string NoteNames[12] = {
	"C-", "C#", "D-", "D#",
	"E-", "F-", "F#", "G-",
	"G#", "A-", "A#", "B-",
	};

	//Microtonal notation
	string NoteNames_MT[12] = {
	"c-", "c#", "d-", "d#",
	"e-", "F-", "f#", "g-",
	"g#", "a-", "a#", "b-",
	};

	//German style notation
	string NoteNames_GR[12] = {
	"C-", "C#", "D-", "D#",	
	"E-", "F-", "F#", "G-",
	"G#", "A-", "B-", "H-",
	};

	//Flat style notation
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
		int note = NULL_COMMAND;
		int octave = 0;
		int instrument = NULL_COMMAND;
		int volume = NULL_COMMAND;
		int effect = NULL_COMMAND;
		int effectvalue = NULL_COMMAND;
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
	//fuck it we ball !!
	//Needs to be float for the Lanczos resampling
	float CurrentSamplePointIndex = 0;
	float CurrentPlayedNote = 0;
	int NoteType = 0;//Specifically for notation styling

	int CurrentInstrument = 0;

	bool PlayingNote = false;
	bool IsActive;
	bool Tickcheck = false;

	void SetUp(int Length);
	void TickCheck(int RowIndex, vector<Instrument>& instruments, vector<Sample>& samples);
	void UpdateChannel(vector<Instrument>& instruments, vector<Sample>& samples);
	float Resample(vector<Sint16>& SampleData);
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

*/

#endif // !Channel