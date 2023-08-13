#pragma once
#ifndef Instrument

class Instrument
{
public:
	//Instrument index
	int Index;
	//Sample pointer
	int SampleIndex;
	//Total volume (will only go from 0 to 127)
	int Volume;
	//Left ear panning (will only go from 0 to 127)
	int LPan;
	//Right ear panning (will only go from 0 to 127)
	int RPan;
	//Frequency of noise in noise mode (0 to 32)
	int NoiseFreq;
	//Gain for raising audio volume
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
private:
};

#endif // !Instrument