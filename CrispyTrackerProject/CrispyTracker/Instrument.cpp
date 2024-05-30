#include "Instrument.h"

Instrument::Instrument()
{
	Name.reserve(64);
}

Instrument::~Instrument()
{
}

//Calculates the pitch of a sample based off of a pitch in Hz
uint16_t Instrument::BRR_Pitch(uint16_t pit)
{
	if (pit != 0)
	{
		int SampleDiff = 32000.0 - CurrentSample.SampleRate;
		float pitval = pit * (SampleDiff / 4096.0);
		if (SampleDiff < 0) pitval = pit * (-SampleDiff / 4096.0);
		//return 0x1000;
		return (uint16_t)(pitval);
	}
}
