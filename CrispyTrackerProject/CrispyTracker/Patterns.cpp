#include "Patterns.h"

void Patterns::SetUp(int Length)
{
	StandardRow.note = 256;
	StandardRow.instrument = 256;
	StandardRow.volume = 256;
	StandardRow.effect = 256;
	StandardRow.effectvalue = 256;
	for (int i = 0; i < Length; i++)
	{
		SavedRows[i] = StandardRow;
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
