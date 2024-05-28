#include "Patterns.h"

void Patterns::SetUp(int Length)
{
	StandardRow.note = StandardRow.instrument = StandardRow.volume = StandardRow.effect = StandardRow.effectvalue = NULL_COMMAND;
	for (int i = 0; i < Length; i++)
	{
		SavedRows[i] = StandardRow;
	}
}

int Patterns::Pattern_EvaluateHexInput(int input, int index)
{
	//Taxes a value and then converts it to hexidecimal
	int surrogate = 0;
	if (input)
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
