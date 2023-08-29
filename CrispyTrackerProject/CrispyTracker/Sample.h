#pragma once
#include <stdlib.h>
#include <string>
#include <vector>

class Sample
{
public:
	int SampleIndex;
	long PlayingHZ;
	int NoteOffset;
	
	std::string SampleName = "Sample: 0";
	std::vector<Uint16> SampleData;
	std::vector<Sint16> BRRSampleData;
	int32_t SampleRate;
	int16_t FineTune;
	bool Loop;
	int16_t LoopStart;
	int16_t LoopEnd;
};