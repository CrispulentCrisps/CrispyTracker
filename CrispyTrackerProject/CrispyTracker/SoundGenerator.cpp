#include "SoundGenerator.h"

int T = 0; //Time value

SoundGenerator::SoundGenerator() {

}

//Used for loading data into the audio queue for the program
HRESULT SoundGenerator::LoadData(UINT count, BYTE* data, DWORD* flags)
{
	Sint16* dp = (Sint16*)data;
	float Freq = NVT[NoteIndex];
	for (int i = 0; i < count; i++)
	{
		dp[2 * i + 0] = Totalbuffer[i][0];//Left ear
		dp[2 * i + 1] = Totalbuffer[i][1];//Right ear
	}
	return S_OK;
}

//Update the registers for the emulation
void SoundGenerator::Update(float ElapsedTime, Channel* ch, vector<Sample>& Samples, int YPos, vector<Instrument>& inst)
{
	for (int i = 0; i < 8; i++)
	{
		if (ch[i].Tickcheck)
		{
			ch[i].Index = i;
			Emu_APU.APU_Grab_Channel_Status(&ch[i], &inst[ch[i].CurrentInstrument], YPos);
			ch[i].Tickcheck = false;
			/*
			int ChannelNote = inst[ch[i].CurrentInstrument].SampleIndex;
			
			if (Samples[ChannelNote].Loop)
			{
				if (ch[i].CurrentSamplePointIndex >= Samples[ChannelNote].LoopEnd)
				{
					ch[i].CurrentSamplePointIndex = Samples[inst[ch[i].CurrentInstrument].SampleIndex].LoopStart;
				}
			}
			else if (ch[i].CurrentSamplePointIndex >= Samples[ChannelNote].SampleData.size())
			{
				ch[i].CurrentSamplePointIndex = Samples[ChannelNote].SampleData.size();
			}
			//cout << "\nPhase Accum: " << Phase[i] << " - Channel: " << i;
			ch[i].CurrentSamplePointIndex += (float)pow(2., ch[i].CurrentPlayedNote / 12.);
			*/
		}
		else
		{
			//ch[i].CurrentSamplePointIndex = 0;
		}
		Emu_APU.APU_Evaluate_Channel_Regs(ch);
	}
}

//Updates buffer size for the audio buffer
void SoundGenerator::SetBufferSize(int Length)
{
	Totalbuffer.resize(Length);
	Totalbuffer.resize(Length);
}

//Creates pitch table
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

//Debugs the audio flow to find errors and inconsistencies
void SoundGenerator::DEBUG_Open_File()
{
	AudioOutputFile.open("AudioLog.txt");
}

//Closes debug file
void SoundGenerator::DEBUG_Close_File()
{
	AudioOutputFile.close();
}

//Outputs the current audio buffer for the passed buffer to a text file
void SoundGenerator::DEBUG_Output_Audio_Buffer_Log(Sint16 AudioData[][2], int Frame, int BufferIndex, int BufferSize)
{
	AudioOutputFile << "Frame: " << Frame << " - Index: " << BufferIndex << " - Audio Data L: " << AudioData[BufferIndex][0] << " - Audio Data R: " << AudioData[BufferIndex][1] << " - Buffer Size: " << BufferSize << "\n";
}
