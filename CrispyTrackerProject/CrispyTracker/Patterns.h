#pragma once
#include "Channel.h"

using namespace std;
typedef struct Row {
	uint16_t note = NULL_COMMAND;
	uint16_t octave = NULL_COMMAND;
	uint16_t instrument = NULL_COMMAND;
	uint16_t volume = NULL_COMMAND;
	uint16_t effect = NULL_COMMAND;
	uint16_t effectvalue = NULL_COMMAND;
};

class Patterns
{
public:
	uint8_t Index;

	Row SavedRows[256];
	void SetUp(int Length);
	int Pattern_EvaluateHexInput(int input, int index);
	void ShiftRowValues(int ypos, int step);
};