#pragma once
#include "Sample.h"
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
	//Overall data size is 8 bytes and 2 bits
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
	//ADSR type (0-3)
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

	Sample CurrentSample;
	int CurrentSampleIndex = 0;//This is for the index of the sample within the sample list
	int CurrentSamplePoint = 0;//This is for the data bytes used in the BRR file
	signed char CurrentSampleSubPoint = 0;//For the 4 bit samples in the data byte
	bool StopSample = true;

	uint16_t BRR_Pitch(uint16_t pit);
};

#endif // !Instrument