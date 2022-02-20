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
				break;
			case SDL_QUIT:
				running = false;
				break;
			case SDLK_q:
				SG.NoteIndex = TuninOff;
				break;
			case SDLK_2:
				SG.NoteIndex = TuninOff + 1;
				break;
			case SDLK_w:
				SG.NoteIndex = TuninOff + 2;
				break;
			case SDLK_3:
				SG.NoteIndex = TuninOff + 3;
				break;
			case SDLK_e:
				SG.NoteIndex = TuninOff + 4;
				break;
			case SDLK_r:
				SG.NoteIndex = TuninOff + 5;
				break;
			case SDLK_5:
				SG.NoteIndex = TuninOff + 6;
				break;
			case SDLK_t:
				SG.NoteIndex = TuninOff + 7;
				break;
			case SDLK_6:
				SG.NoteIndex = TuninOff + 8;
				break;
			case SDLK_y:
				SG.NoteIndex = TuninOff + 9;
				break;
			case SDLK_7:
				SG.NoteIndex = TuninOff + 10;
				break;
			case SDLK_u:
				SG.NoteIndex = TuninOff + 11;
				break;
			case SDLK_i:
				SG.NoteIndex = TuninOff + 12;
				break;
			}
			SG.PlayAudioStream();
			SDL_Delay(1);
			break;
		case SDL_QUIT:
			// shut down
			running = false;
			break;
		}
	}
}
