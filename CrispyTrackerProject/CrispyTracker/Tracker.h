#pragma once
#ifndef DEBUG

#include <SDL_keyboard.h>
#include "Channel.h"

#include "imgui.h"
#include "Libraries/imgui/backends/imgui_impl_sdl2.h"

#include <iostream>
#include <vector>
#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <math.h>

using namespace std;

class Tracker
{
public:
	Tracker();
	~Tracker();

	SDL_Renderer* rend = NULL;
	SDL_Window* window = NULL;
	ImGuiContext* cont = NULL;
	Channel Channels[8];
	int TickLimit;
	int Length;
	int YPos;
	int Input;
	bool PlayingTrack;
	bool ShowMain = true;

	void Initialise(int StartLength);
	void Run();
	void CheckInput();
	void Render();
};

#endif // DEBUG