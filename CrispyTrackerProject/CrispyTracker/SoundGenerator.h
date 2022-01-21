#include <stdio.h>
#pragma once
using namespace std;
class SoundGenerator
{
public:
	int TotalVolume;
	int Hz;
	int NoteIndex;//Going from C0 to C8
	int NotePos;//Position of note in channel
	float Tuning;

	float NVT[110];

	bool IsPlaying;

	SoundGenerator(int TV, int NI, int POS);
};