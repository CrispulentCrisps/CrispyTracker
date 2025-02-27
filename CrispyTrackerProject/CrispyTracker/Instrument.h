#pragma once
#include "Sample.h"
#include <math.h>
#include <iostream>
#include <string>
#include <vector>

#ifndef Instrument

const double StartValues[12] = {
16.35, 17.32, 18.35, 19.45,
20.60, 21.83, 23.12, 24.50,
25.96, 27.50, 29.14, 30.87
};

class Instrument
{
public:
	Instrument();
	~Instrument();
	//Instrument index
	int Index;
	//Instrument name
	std::string Name;
	//Sample pointer
	int SampleIndex = 0;

	//Total volume (will only go from -128 to 127)
	int Volume;
	//Left ear panning (will only go from -128 to 127)
	int LPan;
	//Right ear panning (will only go from -128 to 127)
	int RPan;
	//Gain for raising audio volume (0-255)
	int Gain; 
	//Invert left values
	bool InvL;
	//Invert right values
	bool InvR;
	//Snes FM
	bool PitchMod;
	//Echo
	bool Echo;
	//Activate noise mode
	bool Noise;

	//ADSR
	bool EnvelopeUsed;
	//Attack (0-15)
	int Attack;
	//Decay (0-7)
	int Decay;
	//Sustain (0-7)
	int Sustain;
	//Release (0-31)
	int Release;

	//Priority (0-255)
	int Priority;

	int NoteOff = 0;

	uint16_t InstADDR;

	Sample CurrentSample;

	uint16_t BRR_Pitch(double pit);
	double SetVolume(int Pan);//Determines volume, 1 for RIGHT, -1 for LEFT. Output is normalised, 0 - 1
};

#endif // !Instrument