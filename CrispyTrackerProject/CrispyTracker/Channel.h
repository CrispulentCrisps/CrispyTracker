#pragma once
class Channel
{
public:
	int Index;
	int Tick;
	int ChannelLength;
	int WaveType;
	int EffectsSize;
	int Volume;
	int ChannelPatternIndex;

	bool IsActive;

	void SetUp();
	void Tick();
};