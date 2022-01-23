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
		CheckInput();
		if (Input == 'Q')
		{
			SG.NoteIndex = 64;
		}
		else if (Input == '2')
		{
			SG.NoteIndex = 65;
		}
		else if (Input == 'W')
		{
			SG.NoteIndex = 66;
		}
		else if (Input == '3')
		{
			SG.NoteIndex = 67;
		}
		else if (Input == 'E')
		{
			SG.NoteIndex = 68;
		}
		else if (Input == 'R')
		{
			SG.NoteIndex = 69;
		}
		else if (Input == '5')
		{
			SG.NoteIndex = 70;
		}
		else if (Input == 'T')
		{
			SG.NoteIndex = 71;
		}
		else if (Input == '6')
		{
			SG.NoteIndex = 72;
		}
		else if (Input == 'Y')
		{
			SG.NoteIndex = 73;
		}
		else if (Input == '7')
		{
			SG.NoteIndex = 74;
		}
		else if (Input == 'U')
		{
			SG.NoteIndex = 75;
		}
		else if (Input == 'I')
		{
			SG.NoteIndex = 76;
		}

		SG.PlayAudioStream();
	}
}

HANDLE Tracker::CheckInput()
{
	Input = _getche_nolock();
	return HANDLE();
}
