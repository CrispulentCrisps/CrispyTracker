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
	Channel ChannelRef[8];
	
	int TotalVolume = 1;
	int Hz;
	int NoteIndex;//Going from C0 to C8
	int NotePos;//Position of note in channel
	float Tuning;

	float NVT[111];
	
	bool IsPlaying;
	bool PlayingNoise;

	float TimeBetweenSamplePoints = 0;
	float TotalbufferLeft = 0;
	float TotalbufferRight = 0;
	SoundGenerator(int TV, int NI, int POS, Channel channels[]);
	HRESULT LoadData(UINT count, BYTE* data, DWORD* flags);
	void MixChannels(Channel ch[8]);
	void Update(float ElapsedTime, Channel ch[8]);//This should be used for updating the sample index of the channels
};

#endif // !SoundGenerator
