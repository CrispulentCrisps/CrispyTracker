#pragma once
#include "Patterns.h"

enum InstEntryEffectFlags {
	InvL = 1,
	InvR = 2,
	PMod = 4,
	Noise = 8,
	Delay = 16,
};

typedef struct Module {
	std::string AuthorName;
	std::string TrackName;
	std::string TrakcDesc;
	std::vector<Sample> samples;
	std::vector<Instrument> inst;
	std::vector<Patterns> patterns;
	char dsp_mem[65536];
};

//This is for each instrument used in every track, this is to save memory from having to write the same values in the sequence data again and again
//Size is 8 Bytes
typedef struct InstEntry {
	int8_t Vol_L;			//Left channel volume
	int8_t Vol_R;			//Right channel volume
	uint8_t ADSR1;			//EDDD AAAA	ADSR enable (E), decay rate (D), attack rate (A).
	uint8_t ADSR2;			//SSSR RRRR	Sustain level (S), sustain rate (R).
	uint16_t Gain;			//0VVV VVVV 1MMV VVVV	Mode(M), value(V).
	uint8_t EffectState;	//Holds the state of the effects in the instrument to reference in DSP memory
	uint8_t SampleIndex;	//Index of the sample, to be used by SCRN registers while they look in the DIR page
};

//Sequence entry reffers to a single row in the tracker for a single file
//Size is 14 Bytes
typedef struct SequenceEntry {
	uint16_t Pitch;			//absolute pitch register value for the given note
	uint16_t WaitTime;		//Time to wait before the next note, so as to not write every blank value down
	int8_t Volume_L;		//Left Volume of said note
	int8_t Volume_R;		//Right Volume of said note
	InstEntry inst;			//Instrument Reference
};

//This is for an
//Size is XX Bytes
typedef struct Pattern {
	SequenceEntry* sequence;
};

//This is for an
//Size is XX Bytes
typedef struct SubtuneEntry {
	uint8_t SongIndex;
	uint8_t SongSpeed;
};