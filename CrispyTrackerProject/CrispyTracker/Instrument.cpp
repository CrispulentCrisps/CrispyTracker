#include "Instrument.h"

Instrument::Instrument()
{
	Name.reserve(64);
}

Instrument::~Instrument()
{
}

//Calculates the pitch of a sample based off of a pitch in Hz
short Instrument::BRR_Pitch(double pit)
{
	short ReturnVal = (pit / CurrentSample.SampleRate) * 0x1000;

	return ReturnVal;
}
