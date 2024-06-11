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
		float basepit = (pit * CurrentSample.SampleRate * 16.0) / 125.0;
		basepit = max(basepit, (float)0);
		basepit = min(basepit, (float)0x3FFF);
		//return 0x1000;
		return (uint16_t)basepit;
	}
	else
	{
		return 0;
	}
}

double Instrument::SetVolume(int Pan)//Determines volume, 1 for LEFT, -1 for RIGHT
{
	if (Pan >= 0)//Assume L
	{
		return (LPan / 127.0) * (Volume / 127.0) * (InvL ? -1 : 1);
	}
	else //Assume L
	{
		return (RPan / 127.0) * (Volume / 127.0) * (InvR ? -1 : 1);
	}
}