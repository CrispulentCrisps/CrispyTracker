#ifndef SoundGenerator

#include <stdio.h>

#if !CT_UNIX
#include <conio.h>
#include <mmdeviceapi.h>
#include <Audioclient.h>
#endif
#if CT_UNIX
#define BYTE unsigned char
#endif

#include <ctype.h>
#include <iostream>
#include "Channel.h"

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
	void CheckSound(SDL_AudioSpec want, SDL_AudioSpec have, SDL_AudioDeviceID dev, Channel AudioData[]);
};

#endif // !SoundGenerator
