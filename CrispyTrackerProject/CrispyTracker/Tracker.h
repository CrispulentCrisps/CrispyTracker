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

	SDL_Renderer* rend = NULL;
	SDL_Window* window = NULL;
	ImGuiContext* cont = NULL;
	vector<Channel> Channels;
	int TickLimit;
	int Length;
	int YPos;
	int Input;
	bool PlayingTrack;
	
	void Initialise(int StartAmount, int StartLength);
	void Run();
	void CheckInput();
	void Render();
};

#endif // DEBUG