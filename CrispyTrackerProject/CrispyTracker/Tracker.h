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
#include "Libraries/ImGuiFileDialog-0.6.5/ImGuiFileDialog.h"
#include "Libraries/libsndfile/include/sndfile.h"

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

	int UNIVERSAL_WINDOW_FLAGS = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse;
	int TABLE_FLAGS = ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp;
	int TRACKER_AUDIO_BUFFER = 1024;
	int SPS = 41000;
	string VERSION = "version: 0.2";
	int AUDIO_FORMATS = SF_FORMAT_WAV;
	const Uint8* keystates = 0;
	SDL_Event event;
	bool IsPressed = false;
	SDL_Renderer* rend = NULL;
	SDL_Window* window = NULL;
	ImGuiContext* cont = NULL;
	ImGuiIO io;
	
	Channel Channels[8];
	vector<Patterns> pattern;

	ImColor Default = IM_COL32(22, 22, 33, 255);
	ImColor H2Col = IM_COL32(66, 66, 88, 255);
	ImColor H1Col = IM_COL32(33, 33, 66, 255);
	ImColor CursorCol = IM_COL32(99, 99, 122, 255);

	int TickLimit;
	int TrackLength = 64;
	int YPos;
	int Input;
	int Step = 1;
	int Octave = 4;
	int ChannelColumn = 0, ChannelRow = 0;
	bool PlayingTrack;
	int TextSize = 14;
	int BaseTempo = 150;
	int Highlight1 = 4;
	int Highlight2 = 16;
	int TempoDivider = 6;

	bool ShowSettings = false;
	
	int SelectedInst = 0;
	int SelectedSample = 0;
	
	bool ShowCredits = false;
	bool ShowInstrument = false;
	bool ShowSample = false;
	bool ShowEcho = false;
	bool LoadingSample = false;

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

	void LoadSample();
	void DownMix(SNDFILE* sndfile, SF_INFO sfinfo, Sint16 outputBuffer[]);
	void UpdateRows();
	void BRRConverter();
	string Authbuf;
	string Descbuf;
	string Output = " ";
	string FilePath = " ";
	string FileName = " ";
};
#endif // DEBUG