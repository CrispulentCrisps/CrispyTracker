#include "Patterns.h"

void Patterns::SetUp(int Length)
{
	for (char i = 0; i < Length; i++)
	{
		Row row;
		SavedRows.push_back(row);
		SavedRows[i].note = 256;
		SavedRows[i].instrument = 256;
	}
}

int Patterns::Pattern_EvaluateHexInput(int input, int index)
{
	int surrogate = 0;
	if (input == 0)
	{
		surrogate = 0;
	}
	else
	{
		surrogate = ((Index & 0x0f) << 4) + input;
	}

	//Clamp input
	/*
	if (surrogate > max && surrogate != 256)
	{
		surrogate = 127;
	}
	*/
	return surrogate;
}
