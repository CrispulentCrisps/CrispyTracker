#pragma once
#include "Channel.h"

using namespace std;
typedef struct Row {
	int note = 256;
	int octave = 256;
	int instrument = 256;
	int volume = 127;
	int effect = 256;
	int effectvalue = 256;
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