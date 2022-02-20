#include "Tracker.h"
#include "SoundGenerator.h"

#include <windows.h>
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

void Tracker::Run()
{
	bool PlayingTrack = false;
	bool WindowIsGood = true;
	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		WindowIsGood = false;
	}
	else
	{
		//Create window
		window = SDL_CreateWindow("CrispyTracker :]", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
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
		CheckInput();
		if (WindowIsGood) {
			//Get window surface
			screenSurface = SDL_GetWindowSurface(window);

			//Fill the surface white
			SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));

			//Update the surface
			SDL_UpdateWindowSurface(window);
		}
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
	if (SDL_PollEvent(&event))
	{
		if (event.type == SDL_KEYDOWN)
		{
			if (keystates[SDL_SCANCODE_Q])
			{
				SG.NoteIndex = TuninOff;
			}
			else if (keystates[SDL_SCANCODE_2])
			{
				SG.NoteIndex = TuninOff + 1;
			}
			else if (keystates[SDL_SCANCODE_W])
			{
				SG.NoteIndex = TuninOff + 2;
			}
			else if (keystates[SDL_SCANCODE_3])
			{
				SG.NoteIndex = TuninOff + 3;
			}
			else if (keystates[SDL_SCANCODE_E])
			{
				SG.NoteIndex = TuninOff + 4;
			}
			else if (keystates[SDL_SCANCODE_R])
			{
				SG.NoteIndex = TuninOff + 5;
			}
			else if (keystates[SDL_SCANCODE_5])
			{
				SG.NoteIndex = TuninOff + 6;
			}
			else if (keystates[SDL_SCANCODE_T])
			{
				SG.NoteIndex = TuninOff + 7;
			}
			else if (keystates[SDL_SCANCODE_6])
			{
				SG.NoteIndex = TuninOff + 8;
			}
			else if (keystates[SDL_SCANCODE_Y])
			{
				SG.NoteIndex = TuninOff + 9;
			}
			else if (keystates[SDL_SCANCODE_7])
			{
				SG.NoteIndex = TuninOff + 10;
			}
			else if (keystates[SDL_SCANCODE_U])
			{
				SG.NoteIndex = TuninOff + 11;
			}
			else if (keystates[SDL_SCANCODE_I])
			{
				SG.NoteIndex = TuninOff + 12;
			}
			SG.PlayAudioStream();
		}
	}
}
