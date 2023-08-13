#include <stdio.h>
#include <conio.h>
#include <ctype.h>
#include <iostream>
#include "Channel.h"

#include <mmdeviceapi.h>
#include <Audioclient.h>

#include <math.h>
#include <vector>

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

	float NVT[111];
	
	bool IsPlaying;
	bool PlayingNoise;

	BYTE* Totalbuffer;
	SoundGenerator(int TV, int NI, int POS);
	void CheckSound(SDL_AudioSpec want, SDL_AudioSpec have, SDL_AudioDeviceID dev, vector<Channel> AudioData);
};