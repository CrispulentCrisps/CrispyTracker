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
	
	std::string SampleName = "Sample: ";
	std::vector<Uint8> SampleData;
	std::vector<Sint8> BRRSampleData;
	int32_t SampleRate;
	int16_t FineTune;
	bool Loop;
	int16_t LoopStart;
	int16_t LoopEnd;
};