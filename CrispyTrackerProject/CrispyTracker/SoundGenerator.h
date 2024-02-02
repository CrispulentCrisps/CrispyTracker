#ifndef SoundGenerator

#define AUDIO_BUFFER	512
#define AUDIO_RATE		44100

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
#include <imgui.h>
#include "Channel.h"

#include <math.h>
#include <vector>

#pragma once
using namespace std;
class SoundGenerator
{
public:
	SoundGenerator();
	
	int TotalVolume = 1;
	int Hz;
	int NoteIndex;//Going from C0 to C8
	int NotePos;//Position of note in channel
	int LastBufferPosition = 0;
	float Tuning;

	float NVT[111];
	
	bool IsPlaying;
	bool PlayingNoise;

	float TimeBetweenSamplePoints = 0;
	float TotalbufferLeft[AUDIO_BUFFER];
	float TotalbufferRight[AUDIO_BUFFER];

	Channel* ch[8];

	SoundGenerator(int TV, int NI, int POS, Channel channels[]);
	HRESULT LoadData(UINT count, BYTE* data, DWORD* flags);
	void MixChannels(int Index);
	void Update(float ElapsedTime, Channel* ch);//This should be used for updating the sample index of the channels
	
	void DEBUG_WriteToFile();
};

#endif // !SoundGenerator
