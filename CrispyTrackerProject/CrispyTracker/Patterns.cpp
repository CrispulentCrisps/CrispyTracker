#include "Patterns.h"

void Patterns::SetUp(int Length)
{
	Row StandardRow;
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

void Patterns::ShiftRowValues(int ypos, int step)
{
	vector<Row> temp;
	temp.resize(256);
	for (int x = 0; x < 256; x++)
	{
		temp[x] = SavedRows[x];
	}
	if (step >= 0)
	{
		temp.insert(temp.begin() + ypos, Row());
		temp.erase(temp.begin() + 256);
	}
	else
	{
		temp.erase(temp.begin() + ypos);
		temp.insert(temp.begin() + 255, Row());
	}
	for (int x = 0; x < 256; x++)
	{
		SavedRows[x] = temp[x];
	}
}
