#pragma once
#ifndef DEBUG

#include <SDL_keyboard.h>
#include "Channel.h"
#include "Instrument.h"

#include "imgui.h"
#include "Libraries/imgui/backends/imgui_impl_sdl2.h"
#include "Libraries/imgui/backends/imgui_impl_sdlrenderer2.h"

#include <iostream>
#include <vector>
#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <math.h>

using namespace std;
using namespace ImGui;

class Tracker
{
public:
	Tracker();
	~Tracker();

	int UNIVERSAL_WINDOW_FLAGS = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;
	int TABLE_FLAGS = 0;

	SDL_Renderer* rend = NULL;
	SDL_Window* window = NULL;
	ImGuiContext* cont = NULL;	
	ImGuiIO io;
	Channel Channels[8];
	int TickLimit;
	int Length;
	int YPos;
	int Input;
	bool PlayingTrack;
	int TextSize = 16;

	bool ShowSettings = false;
	
	int SelectedInst;
	bool ShowInstrument = false;
	int InstXPadding = 32;
	int InstYPadding = 72;

	vector<Instrument> inst;
	Instrument DefaultInst;

	void Initialise(int StartLength);
	void Run(void);
	void CheckInput();
	void Render();

	void MenuBar();
	void Patterns();
	void Instruments();
	void Instrument_View();
	void Samples();
	void Sample_View();
	void Settings_View();
	void Credits();
};

#endif // DEBUG