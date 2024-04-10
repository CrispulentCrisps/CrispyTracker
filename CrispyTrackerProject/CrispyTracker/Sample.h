#pragma once
#include "BRRFile.h"
#include <stdlib.h>
#include <string>

class Sample
{
public:
	enum HByteFlags
	{
		EndFlag = 0b00000001,
		LoopFlag = 0b00000010,
	};
	int SampleIndex;
	long PlayingHZ;
	int NoteOffset = 0;
	
	string SampleName = "Empty!";
	vector<signed short> SampleData;
	vector<signed short> BRRSampleData;
	int SampleRate = 0;
	int FineTune = 0;
	bool Loop = false;
	int LoopStart = 0;
	int LoopEnd = 0;
	int HPoint = 0;
	int LPoint = 0;
	BRRFile brr;

	void BRRConvert();
	void LargestPoint();
};