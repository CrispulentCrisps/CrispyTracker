#pragma once
#include "Channel.h"
#include "Macros.h"

using namespace std;

class Patterns
{
public:
	uint8_t Index;
	uint16_t Addr;

	Row SavedRows[256];
	void SetUp(int Length);
	int Pattern_EvaluateHexInput(int input, int index);
	void ShiftRowValues(int ypos, int step);
};