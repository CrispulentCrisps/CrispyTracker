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
bool SoundGenerator::Update(float ElapsedTime, Channel* ch, vector<Sample>& Samples, int& YPos, vector<Instrument>& inst)
{
	if (ch[0].Tickcheck)
	{
		for (int i = 0; i < 8; i++)
		{
			//Emu_APU.APU_Grab_Channel_Status(&ch[i], &inst[ch[i].CurrentInstrument], YPos);
			ch[i].Tickcheck = false;
		}
		YPos++;
	}
	return true;
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
