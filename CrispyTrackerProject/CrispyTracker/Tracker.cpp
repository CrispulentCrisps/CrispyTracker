#include "Tracker.h"
#include "SoundGenerator.h"

#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <math.h>

//Universal variables here
SoundGenerator SG(1, 56, 1);
bool running = true;

//Screen dimension constants
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

//The window we'll be rendering to
SDL_Window* window = NULL;

//The surface contained by the window
SDL_Surface* screenSurface = NULL;

SDL_AudioSpec want, have;
SDL_AudioDeviceID dev;


void MyAudioCallback(void* userdata, Uint8* stream, int len)
{
	DWORD    flags;
	static_cast<SoundGenerator*>(userdata)->LoadData(len / 8, stream, &flags);
}

void Tracker::Run()
{
	SDL_memset(&want, 0, sizeof(want)); /* or SDL_zero(want) */
	want.freq = 48000;
	want.format = AUDIO_F32;
	want.channels = 2;
	want.samples = 4096;
	want.callback = MyAudioCallback;  // you wrote this function elsewhere.
	want.userdata = &SG;
	dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
	const char* err = SDL_GetError();

	bool PlayingTrack = false;
	bool WindowIsGood = true;
	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		WindowIsGood = false;
	}
	else
	{
		//Create window
		window = SDL_CreateWindow("CrispyTracker", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (window == NULL)
		{
			printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
			WindowIsGood = false;
		}
		else
		{
			WindowIsGood = true;
		}
	}

	while (running) {
		if (WindowIsGood) {
			//Get window surface
			screenSurface = SDL_GetWindowSurface(window);

			//Fill the surface white
			SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 22, 22, 22));

			//Update the surface
			SDL_UpdateWindowSurface(window);
			printf("WINDOW UPDATING \n");
		}
		CheckInput();
	}
	//Destroy window
	SDL_DestroyWindow(window);

	//Quit SDL subsystems
	SDL_Quit();
}

void Tracker::CheckInput()
{
	int TuninOff = 48;
	SDL_Event event;
	const Uint8* keystates = SDL_GetKeyboardState(NULL);
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym)
			{
			default:
				return;
				break;
			case SDL_QUIT:
				running = false;
				break;
			case SDLK_q:
				SG.NoteIndex = TuninOff;
				printf("PRESSED Q");
				SG.PlayingNoise == true;
				break;
			case SDLK_2:
				SG.NoteIndex = TuninOff + 1;
				printf("PRESSED 1");
				SG.PlayingNoise == true;
				break;
			case SDLK_w:
				SG.NoteIndex = TuninOff + 2;
				printf("PRESSED W");
				SG.PlayingNoise == true;
				break;
			case SDLK_3:
				SG.NoteIndex = TuninOff + 3;
				printf("PRESSED 3");
				SG.PlayingNoise == true;
				break;
			case SDLK_e:
				SG.NoteIndex = TuninOff + 4;
				printf("PRESSED E");
				SG.PlayingNoise == true;
				break;
			case SDLK_r:
				SG.NoteIndex = TuninOff + 5;
				printf("PRESSED R");
				SG.PlayingNoise == true;
				break;
			case SDLK_5:
				SG.NoteIndex = TuninOff + 6;
				printf("PRESSED 5");
				SG.PlayingNoise == true;
				break;
			case SDLK_t:
				SG.NoteIndex = TuninOff + 7;
				printf("PRESSED T");
				SG.PlayingNoise == true;
				break;
			case SDLK_6:
				SG.NoteIndex = TuninOff + 8;
				printf("PRESSED 6");
				SG.PlayingNoise == true;
				break;
			case SDLK_y:
				SG.NoteIndex = TuninOff + 9;
				printf("PRESSED Y");
				SG.PlayingNoise == true;
				break;
			case SDLK_7:
				SG.NoteIndex = TuninOff + 10;
				printf("PRESSED 7");
				SG.PlayingNoise == true;
				break;
			case SDLK_u:
				SG.NoteIndex = TuninOff + 11;
				printf("PRESSED U");
				SG.PlayingNoise == true;
				break;
			case SDLK_i:
				SG.NoteIndex = TuninOff + 12;
				printf("PRESSED I");
				SG.PlayingNoise == true;
				break;
			}

			{
				SDL_PauseAudioDevice(dev, 0);
				SDL_Delay(1000);
				SDL_CloseAudioDevice(dev);
			}

			break;
		case SDL_QUIT:
			running = false;
			break;
		}
		SG.PlayingNoise = false;
	}
}
