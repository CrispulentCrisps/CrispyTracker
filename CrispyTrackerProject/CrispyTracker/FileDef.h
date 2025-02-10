#pragma once
#include "Patterns.h"

enum InstEntryEffectFlags {
	InvL = 1,
	InvR = 2,
	PMod = 4,
	Noise = 8,
	Echo = 16,
};
typedef struct Subtune {
	char aubuf[256];
	char trbuf[256];
	char dcbuf[256];
	std::string AuthorName = " ";
	std::string TrackName = " ";
	std::string TrackDesc = " ";
	uint8_t TrackLength = 64;
	uint8_t SongLength = 1;
	uint8_t Speed1 = 6;
	uint8_t TempoDivider = 2;
	uint8_t Highlight1 = 4;
	uint8_t Highlight2 = 16;
	std::vector<uint8_t> Orders[8];
	//Echo
	int8_t EchoVol = 64;
	uint8_t Delay = 0;
	uint8_t Feedback = 64;
	int8_t EchoFilter[8] = { 32, 24, 16 , 8 , 4 , 2 , 1 , 0 };
	uint8_t SFXFlag = 0;
};

typedef struct Module {
	//Tracker Specific
	std::vector<Subtune> subtune;
	std::vector<Sample> samples;
	std::vector<Instrument> inst;
	std::vector<Patterns> patterns;

	//SNES Specific
	uint8_t SelectedRegion;
};

//This is for each instrument used in every track, this is to save memory from having to write the same values in the sequence data again and again
typedef struct InstEntry {
	uint8_t SampleIndex;	//Index of the sample, to be used by SCRN registers while they look in the DIR page
	uint8_t ADSR1;			//EDDD AAAA	ADSR enable (E), decay rate (D), attack rate (A).
	uint8_t ADSR2;			//SSSR RRRR	Sustain level (S), sustain rate (R).
	uint8_t Gain;			//0VVV VVVV 1MMV VVVV	Mode(M), value(V).
	uint8_t EffectState;	//Holds the state of the effects in the instrument to reference in DSP memory
};

//Sequence entry reffers to a single row in the tracker for a single file
typedef struct SequenceEntry {
	uint8_t Pitch;			//pitch value for the pitch table
	int8_t Volume_L;		//Left Volume of said note
	int8_t Volume_R;		//Right Volume of said note
	uint16_t instADDR;		//Instrument Reference
	uint8_t EffectsState;	//The state of the effects flags in the row
	uint8_t EffectsValue;	//The state of the effects flags in the row
};

//This is for storing repeated sequences
typedef struct PatternEntry {
	uint16_t PatternIndex;			//Address to pattern
	uint8_t SequenceAmount;			//Amount of sequence entries used in said pattern
	std::vector<uint16_t> SequenceList;	//Sequence entries index in pattern
};

//This is for a single tune that can be used in the SPC file
typedef struct SubtuneEntry {
	uint16_t SubStart;		//Where the start of a subtune is, defined by order address
};