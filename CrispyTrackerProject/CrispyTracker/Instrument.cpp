#include "Instrument.h"

Instrument::Instrument()
{
	Name.reserve(64);
}

Instrument::~Instrument()
{
}

//Calculates the pitch of a sample based off of a pitch in KHz
short Instrument::BRR_Pitch(double pit)
{
	//Base rate of 0x1000 = 32KHz
	//This means that 1 hex value = 8hz
	short ReturnVal = 0x1000;



	return ReturnVal;
}
