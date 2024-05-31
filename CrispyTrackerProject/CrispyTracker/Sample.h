#pragma once
#include "BRRtools/BRRtools-master/src/brr.h"
#include "BRRFile.h"
#include <stdio.h>
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
	int SampleADDR;
	long PlayingHZ;
	
	string SampleName = "Empty!";
	vector<signed short> SampleData;
	int SampleRate = 0;
	int FineTune = 0;
	bool Loop = false;
	int LoopStart = 0;
	int LoopStartAddr = 0;
	int LoopEnd = 0;
	int HPoint = 0;
	int LPoint = 0;
	BRRFile brr;

	void BRRConvert();
};