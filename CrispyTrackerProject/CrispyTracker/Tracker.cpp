#include <windows.h>
#include <Audioclient.h>
#include <mmdeviceapi.h>
#include "Tracker.h"

void Tracker::Run()
{
	//Setting audio devices and clients up [while I act like I've any clue how this shit properly works ;w;]
	const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
	const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
	const IID IID_IAudioClient = __uuidof(IAudioClient);
	const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);


	bool running = true;
	while (running)
	{

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
