#pragma once
#ifndef DEBUG	 

#include <SDL_keyboard.h>
#include "Channel.h"
#include "Sample.h"
#include "Instrument.h"
#include "Patterns.h"

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
	int TABLE_FLAGS = ImGuiTableFlags_Borders;

	SDL_Renderer* rend = NULL;
	SDL_Window* window = NULL;
	ImGuiContext* cont = NULL;
	ImGuiIO io;
	
	Channel Channels[8];
	vector<Patterns> pattern;

	int TickLimit;
	int TrackLength = 64;
	int YPos;
	int Input;
	int Step = 1;
	int Octave = 4;
	bool PlayingTrack;
	int TextSize = 16;

	bool ShowSettings = false;
	
	int SelectedInst = 0;
	int SelectedSample = 0;
	
	bool ShowInstrument = false;
	bool ShowSample = false;
	bool ShowEcho = false;

	int InstXPadding = 32;
	int InstYPadding = 72;

	vector<Instrument> inst;
	Instrument DefaultInst;

	vector<Sample> samples;
	Sample DefaultSample;

	int VolumeScaleL = 127;	//(0-127)
	int VolumeScaleR = 127;	//(0-127)

	//Echo settings
	int Delay;			//(0-15)
	int Feedback;		//(0-127)
	int EchoVolL;		//(0-127)
	int EchoVolR;		//(0-127)
	int EchoFilter[8];	//(0-127) Must accumulate to 127 at most!!!!

	//Functions
	void Initialise(int StartLength);
	void Run(void);
	void CheckInput();
	void Render();

	void MenuBar();
	void Patterns();
	void Instruments();
	void Instrument_View();
	void Channel_View();
	void Samples();
	void Sample_View();
	void Settings_View();
	void Misc_View();
	void Author_View();
	void EchoSettings();
	void Credits();
	void SetupInstr();

	string Authbuf;
	string Descbuf;
	string Output = " ";
};
#endif // DEBUG