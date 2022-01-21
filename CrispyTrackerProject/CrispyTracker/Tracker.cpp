#include "Tracker.h"
#include "SoundGenerator.h"
#include <windows.h>
#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <math.h>



void Tracker::Run()
{
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
	SoundGenerator SG(1, 56, 1);
	bool running = true;
	PlayingTrack = false;

	while (running)
	{
		//the 0x8000 is a check for if the key has a greater value than that number
		if (GetKeyState('q') & 0x8000)
		{
			SG.NoteIndex = 64;
		}
		else if (GetKeyState('2') & 0x8000)
		{
			SG.NoteIndex = 65;
		}
		else if (GetKeyState('q') & 0x8000)
		{
			SG.NoteIndex = 66;
		}
		else if (GetKeyState('3') & 0x8000)
		{
			SG.NoteIndex = 67;
		}
		else if (GetKeyState('e') & 0x8000)
		{
			SG.NoteIndex = 68;
		}
		else if (GetKeyState('r') & 0x8000)
		{
			SG.NoteIndex = 69;
		}
		else if (GetKeyState('5') & 0x8000)
		{
			SG.NoteIndex = 70;
		}
		else if (GetKeyState('t') & 0x8000)
		{
			SG.NoteIndex = 71;
		}
		else if (GetKeyState('6') & 0x8000)
		{
			SG.NoteIndex = 72;
		}
		else if (GetKeyState('y') & 0x8000)
		{
			SG.NoteIndex = 73;
		}
		else if (GetKeyState('7') & 0x8000)
		{
			SG.NoteIndex = 74;
		}
		else if (GetKeyState('u') & 0x8000)
		{
			SG.NoteIndex = 75;
		}
		else if (GetKeyState('i') & 0x8000)
		{
			SG.NoteIndex = 76;
		}

		SG.PlayAudioStream();
		CoUninitialize();
	}
}

void Tracker::CheckNotes(Channel Channels[])
{
}

void Tracker::TickAlong(Channel Channels[], int tick)
{
	if (tick > TickLimit)
	{
		for (int i = 0; i < sizeof(Channels); i++)
		{

		}
	}
}
