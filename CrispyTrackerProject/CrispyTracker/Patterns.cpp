#include "Patterns.h"

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
