#include <stdio.h>
#include <conio.h>
#include <ctype.h>
#include <iostream>
#include <windows.h>
#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <math.h>

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

	BYTE* Totalbuffer;
	SoundGenerator(int TV, int NI, int POS);
	HRESULT LoadData(UINT count, BYTE* data, DWORD* flags);
	HRESULT PlayAudioStream(void);
};