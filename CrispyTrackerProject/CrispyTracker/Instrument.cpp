#include "Instrument.h"

void Instrument::PlaySample()//Use once for playing the sample, will repeatedly call function
{
	if (CurrentSample.brr.DBlocks.size() > 0)
	{
		CurrentSamplePoint = 0;
		StopSample = false;
	}
}

void Instrument::NextSamplePoint()
{
	if (StopSample = false)
	{
		//Need to reverse the bit shifting from the BRR file

	}
	else
	{

	}
}