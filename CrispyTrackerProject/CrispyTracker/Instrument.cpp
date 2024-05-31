#include "Instrument.h"

Instrument::Instrument()
{
	Name.reserve(64);
}

Instrument::~Instrument()
{
}

//Calculates the pitch of a sample based off of note tuning in irl Hz
uint16_t Instrument::BRR_Pitch(double pit)
{
	if (pit != 0)
	{
		/*
		float SampleDiff = 32000.0 - CurrentSample.SampleRate;
		float pitval = pit * (SampleDiff / 4096.0);
		if (SampleDiff < 0) pitval *= -1;
		*/
		//float basepit = (32000.0 * 4096.0) / (pit * CurrentSample.SampleRate);
		float basepit = (pit * CurrentSample.SampleRate * 16.0) / 125.0;
		basepit = max(basepit, (float)0);
		basepit = min(basepit, (float)0x3FFF);
		//return 0x1000;
		return (uint16_t)basepit;
	}
}
