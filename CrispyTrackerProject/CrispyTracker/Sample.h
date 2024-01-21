#pragma once
#include "BRRFile.h"
#include <stdlib.h>
#include <string>

class Sample
{
public:
	int SampleIndex;
	long PlayingHZ;
	int NoteOffset = 0;
	
	std::string SampleName = "Empty!";
	std::vector<signed short> SampleData;
	std::vector<signed short> BRRSampleData;
	int SampleRate = 0;
	int FineTune = 0;
	bool Loop = false;
	int LoopStart = 0;
	int LoopEnd = 0;
	BRRFile brr;

	void BRRConvert();
};