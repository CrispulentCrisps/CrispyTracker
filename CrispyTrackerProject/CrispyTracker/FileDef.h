#pragma once
#include "Patterns.h"

enum InstEntryEffectFlags {
	InvL = 1,
	InvR = 2,
	PMod = 4,
	Noise = 8,
	Echo = 16,
};

typedef struct Module {
	//Tracker Specific
	std::string AuthorName;
	std::string TrackName;
	std::string TrakcDesc;
	std::vector<Sample> samples;
	std::vector<Instrument> inst;
	std::vector<Patterns> patterns;
	uint8_t TrackLength;
	uint8_t Speed1;
	uint8_t TempoDivider;
	uint8_t Highlight1;
	uint8_t Highlight2;

	//SNES Specific
	unsigned char dsp_mem[65536];
	uint8_t SelectedRegion;
	//Echo
	int8_t EchoVol;
	uint8_t Delay;
	uint8_t Feedback;
	int8_t EchoFilter[8];

};

//This is for each instrument used in every track, this is to save memory from having to write the same values in the sequence data again and again
typedef struct InstEntry {
	int8_t Vol_L;			//Left channel volume
	int8_t Vol_R;			//Right channel volume
	uint8_t ADSR1;			//EDDD AAAA	ADSR enable (E), decay rate (D), attack rate (A).
	uint8_t ADSR2;			//SSSR RRRR	Sustain level (S), sustain rate (R).
	uint8_t Gain;			//0VVV VVVV 1MMV VVVV	Mode(M), value(V).
	uint8_t EffectState;	//Holds the state of the effects in the instrument to reference in DSP memory
	uint8_t SampleIndex;	//Index of the sample, to be used by SCRN registers while they look in the DIR page
	uint8_t Priority;		//Priority of instrument based off of SFX priority
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
	uint8_t PatternIndex;			//Index that the pattern uses
	uint8_t SequenceAmount;			//Amount of sequence entries used in said pattern
	std::vector<uint16_t> SequenceList;	//Sequence entries index in pattern
};

//This is for a single tune that can be used in the SPC file
typedef struct SubtuneEntry {
	uint16_t SongADDR;		//Where in memory the start of the song is. This is defined as the first pattern in memory
	uint8_t SongSpeed;		//Speed of said song in terms of ticks per row
	uint8_t SongResetADDR;	//Where the song restarts
};