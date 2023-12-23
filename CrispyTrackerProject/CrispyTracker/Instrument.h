#pragma once
#include <string>
#include "BRRFile.h"

//	
//	Multi channel instrument
//	

#ifndef Instrument

class Instrument
{
public:
	//Instrument index
	int Index;
	//Instrument name
	std::string Name;
	//Sample pointer
	int SampleIndex;

	//Total volume (will only go from 0 to 127)
	int Volume;
	//Left ear panning (will only go from 0 to 127)
	int LPan;
	//Right ear panning (will only go from 0 to 127)
	int RPan;
	//Frequency of noise in noise mode (0 to 31)
	int NoiseFreq;
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
	//ADSR type (0-4)
	int ADSRType;
	//Attack (0-15)
	int Attack;
	//Decay (0-7)
	int Decay;
	//Sustain (0-7)
	int Sustain;
	//Decay2 (0-31)
	int Decay2;
	//Release (0-31)
	int Release;

	BRRFile brr;
};

#endif // !Instrument