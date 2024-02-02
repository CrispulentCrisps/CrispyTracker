#include "SoundGenerator.h"

int T = 0; //Time value
const float SampleRate = 48000;

SoundGenerator::SoundGenerator()
{
}

HRESULT SoundGenerator::LoadData(UINT count, BYTE* data, DWORD* flags)
{
	Sint16* dp = (Sint16*)data;
	float Freq = NVT[NoteIndex];
	for (int i = 0; i < count; i++)
	{
		LastBufferPosition++ /* % AUDIO_BUFFER*/;
		dp[2 * i + 0] = TotalbufferLeft[i];//Left ear
		dp[2 * i + 1] = TotalbufferRight[i];//Right ear
	}
	return S_OK;
}

void SoundGenerator::MixChannels(int Index)
{
	Sint16 ResultL = 0;
	Sint16 ResultR = 0;
	for (int i = 0; i < 8; i++)
	{
		ResultL += ch[i]->AudioDataL / 8;
		ResultR += ch[i]->AudioDataR / 8;
	}
	TotalbufferLeft[Index] = ResultL;
	TotalbufferRight[Index] = ResultR;
	//cout << "\nCurrent Buffer: " << ResultL;
}

void SoundGenerator::Update(float ElapsedTime, Channel* ch)
{
	TimeBetweenSamplePoints += ElapsedTime;
	for (int i = 0; i < 8; i++)
	{
		if (TimeBetweenSamplePoints > 0.33)
		{
			ch[i].CurrentSamplePointIndex++;
		}
	}
	TimeBetweenSamplePoints -= 0.33f;
}

void SoundGenerator::DEBUG_WriteToFile()
{

}

SoundGenerator::SoundGenerator(int TV, int NI, int POS, Channel channels[]) {
    PlayingNoise == false;
	//A=440
	float StartValues[12] = {
		16.35, 17.32, 18.35, 19.45,
		20.60, 21.83, 32.12, 24.50,
		25.96, 27.50, 29.14, 30.87
	};
	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 12; j++)
		{
			NVT[(i * 12) + j] = StartValues[j] * (1 << i);
		}
	}

	TotalVolume = TV;
	NotePos = POS;
    NoteIndex = NI;
}