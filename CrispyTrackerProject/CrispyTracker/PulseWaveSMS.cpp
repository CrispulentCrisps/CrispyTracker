#include "PulseWaveSMS.h"
#include "SoundGenerator.h"
int pos = 0; //Time value
const float pi = 3.14;
const float SampleRate = 48000;

HRESULT LoadData(UINT count, BYTE* data, DWORD* flags)
{
    float* dp = (float*)data;
    float Volume = 0.25f;
    float Freq = 440;

    for (int i = 0; i < count; i++)
    {
        dp[2 * i + 0] = Volume * floor(sin(pos * (2 * pi) * Freq * (1 / SampleRate)));
        dp[2 * i + 1] = Volume * floor(sin(pos * (2 * pi) * Freq * (1 / SampleRate)));
        pos++;
    }

    return S_OK;
}

void PulseWaveSMS::GenerateNoise()
{

}
