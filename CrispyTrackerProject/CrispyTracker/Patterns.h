#pragma once
#include "Channel.h"

using namespace std;

class Patterns
{
public:
	int Index;
	
	struct Row {
		int note = 256;
		int octave = 256;
		int instrument = 256;
		int volume = 127;
		int effect = 256;
		int effectvalue = 256;
	};

	Row SavedRows[256];
	Row StandardRow;
	void SetUp(int Length);
	int Pattern_EvaluateHexInput(int input, int index);
};