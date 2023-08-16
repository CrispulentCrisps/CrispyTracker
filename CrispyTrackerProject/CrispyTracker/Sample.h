#pragma once
#include <stdlib.h>
#include <string>
#include <vector>

class Sample
{
public:
	int SampleIndex;
	std::string SampleName;
	std::vector<int16_t> SampleData;
	std::vector<int16_t> BRRSampleData;
	int32_t SampleRate;
	int16_t FineTune;
	bool Loop;
	int16_t LoopStart;
	int16_t LoopEnd;
};