#pragma once
#include <stdlib.h>
#include <string>
#include <vector>

class Sample
{
public:
	int SampleIndex;
	long PlayingHZ;
	int NoteOffset = 0;
	
	std::string SampleName = "Sample: 0";
	std::vector<Sint16> SampleData;
	std::vector<Sint16> BRRSampleData;
	int32_t SampleRate = 0;
	int FineTune = 0;
	bool Loop = false;
	int LoopStart = 0;
	int LoopEnd = 0;
};