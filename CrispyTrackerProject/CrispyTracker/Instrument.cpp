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
		cout << "\n\npit: " << pit << "\nPitch calc: " << (pit / (uint16_t)CurrentSample.SampleRate) * 0x1000;
		return (pit / (uint16_t)CurrentSample.SampleRate) * 0x1000;
	}
}
