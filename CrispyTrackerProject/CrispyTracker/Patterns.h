#pragma once
#include "Channel.h"

using namespace std;

class Patterns
{
	int Index;
	
	struct Row {
		int note = 256;
		int octave = 0;
		int instrument = 256;
		int volume = 256;
		int effect = 256;
		int effectvalue = 256;
	};

	vector<Row> SavedRows;


	int Pattern_EvaluateHexInput(int input, int index);
};