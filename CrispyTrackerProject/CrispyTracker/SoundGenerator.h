#ifndef SoundGenerator

#define AUDIO_BUFFER	1024
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
#include <imgui.h>
#include "Channel.h"
#include <string>
#include <iostream>
#include <fstream>

#include <math.h>
#include <vector>
#include <array>

#pragma once

class SoundGenerator
{
public:

	int TRACKER_AUDIO_BUFFER = AUDIO_BUFFER;
	SoundGenerator();

	int TotalVolume = 1;
	int Hz;
	int NoteIndex;//Going from C0 to C8
	int NotePos;//Position of note in channel
	float Tuning;

	float NVT[111];

	bool IsPlaying;
	bool PlayingNoise;

	float TimeBetweenSamplePoints = 0;
	vector<array<Sint16, 2>> Totalbuffer;

	Channel* ch[8];
	SoundGenerator(int TV, int NI, int POS, Channel channels[]);
	HRESULT LoadData(UINT count, BYTE* data, DWORD* flags);
	void MixChannels(int Index, Channel* ch);
	void Update(float ElapsedTime, Channel* ch, vector<Sample>& Samples, int YPos, vector<Instrument>& inst);//This should be used for updating the sample index of the channels
	void SetBufferSize(int Length);
	int P = 0;


	ofstream AudioOutputFile;

	void DEBUG_Open_File();
	void DEBUG_Close_File();

	void DEBUG_Output_Audio_Buffer_Log(Sint16 AudioData[][2], int Frame, int BufferIndex, int BufferSize);
};

#endif // !SoundGenerator
