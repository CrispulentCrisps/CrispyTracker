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
#include <imgui.h>
#include "Channel.h"
#include <string>
#include <iostream>
#include <fstream>

#include <math.h>
#include <vector>

#pragma once
class SoundGenerator
{
public:
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
	Sint16 Totalbuffer[AUDIO_BUFFER][2];

	Channel* ch[8];
	void InitAudio(SDL_AudioSpec* w, SDL_AudioSpec* h);
	SoundGenerator(int TV, int NI, int POS, Channel channels[]);
	HRESULT LoadData(UINT count, BYTE* data, DWORD* flags);
	void MixChannels(int Index);
	void Update(float ElapsedTime, Channel* ch);//This should be used for updating the sample index of the channels

	int P = 0;


	ofstream AudioOutputFile;

	void DEBUG_Open_File();
	void DEBUG_Close_File();

	void DEBUG_Output_Audio_Buffer_Log(Sint16 AudioData[][2], int Frame, int BufferIndex, int BufferSize);
};

#endif // !SoundGenerator
