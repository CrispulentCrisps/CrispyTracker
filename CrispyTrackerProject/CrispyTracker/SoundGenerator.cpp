#include "SoundGenerator.h"

int T = 0; //Time value
const float SampleRate = 48000;

SoundGenerator::SoundGenerator(int TV, int NI, int POS) {
    PlayingNoise == false;
	//A=440
	float StartValues[12] = {
		16.35, 17.32, 18.35, 19.45,
		20.60, 21.83, 32.12, 24.50,
		25.96, 27.50, 29.14, 30.87
	};
	for (char i = 0; i < 9; i++)
	{
		for (char j = 0; j < 12; j++)
		{
			NVT[(i * 12) + j] = StartValues[j] * (1 << i);
		}
	}

	TotalVolume = TV;
	NotePos = POS;
    NoteIndex = NI;
	for (char i = 0; i < 9; i++)
	{
		for (char j = 0; j < 12; j++)
		{
			cout << NVT[(i * 12) + j] << "\n";
		}
		cout << "\n";
	}
}

void SoundGenerator::CheckSound(SDL_AudioSpec want, SDL_AudioSpec have, SDL_AudioDeviceID dev, Channel AudioData[])
{
	//Last step is to take all the channel bytes and then output the audio
	int16_t Result = 0;
	for (size_t i = 0; i < 8; i++)
	{
		Result += AudioData[i].AudioData;
	}
	SDL_QueueAudio(dev, &Result, sizeof(AudioData)/sizeof(AudioData[0]));
}
