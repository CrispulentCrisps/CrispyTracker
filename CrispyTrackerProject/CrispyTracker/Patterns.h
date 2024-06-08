#pragma once
#include "Channel.h"

using namespace std;
typedef struct Row {
	int note = NULL_COMMAND;
	int octave = NULL_COMMAND;
	int instrument = NULL_COMMAND;
	int volume = NULL_COMMAND;
	int effect = NULL_COMMAND;
	int effectvalue = NULL_COMMAND;
};

class Patterns
{
public:
	int Index;

	Row SavedRows[256];
	Row StandardRow;
	void SetUp(int Length);
	int Pattern_EvaluateHexInput(int input, int index);
};