#pragma once
#ifndef DEBUG

#include <SDL_keyboard.h>

#include "Sample.h"
#include "Instrument.h"
#include "Patterns.h"

#include "Libraries/imgui/imgui.h"
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

	int UNIVERSAL_WINDOW_FLAGS = ImGuiWindowFlags_AlwaysAutoResize;
	int TABLE_FLAGS = ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp;
	int TRACKER_AUDIO_BUFFER = 1024;
	int SPS = 41000;
	string VERSION = "version: 0.3";
	int AUDIO_FORMATS = SF_FORMAT_WAV;
	int FrameCount = 0;
	SDL_Event Event;
	SDL_Keycode Currentkey;
	bool IsPressed = false;
	bool EditingMode = true;
	SDL_Renderer* rend = NULL;
	SDL_Window* window = NULL;
	ImGuiContext* cont = NULL;
	ImGuiIO io;
	Channel Channels[8];
	vector<Patterns> pattern;
	int MAX_VALUE = 256;

	ImColor Default = IM_COL32(22, 22, 33, 255);
	ImColor H2Col = IM_COL32(66, 66, 88, 255);
	ImColor H1Col = IM_COL32(33, 33, 66, 255);
	ImColor CursorCol = IM_COL32(99, 99, 122, 255);
	
	SDL_KeyCode NoteInput[24] = 
	{
		//Lower octave
		SDLK_z, SDLK_s, SDLK_x, SDLK_d,
		SDLK_c, SDLK_v, SDLK_g, SDLK_b,
		SDLK_h, SDLK_n, SDLK_j, SDLK_m,
		//Higher octave
		SDLK_q , SDLK_2, SDLK_w, SDLK_3, 
		SDLK_e, SDLK_r, SDLK_5, SDLK_t, 
		SDLK_6, SDLK_y, SDLK_7, SDLK_u,
	};

	SDL_KeyCode VolInput[16] =
	{
		SDLK_0, SDLK_1, SDLK_2, SDLK_3, 
		SDLK_4, SDLK_5, SDLK_6, SDLK_7, 
		SDLK_8, SDLK_9, SDLK_a, SDLK_b, 
		SDLK_c, SDLK_d, SDLK_e, SDLK_f,
	};

	int TickLimit;
	int TrackLength = 64;
	int YPos;
	int Input;
	int Step = 1;
	int Octave = 4;
	int ChannelColumn = 0, ChannelRow = 0;
	bool PlayingTrack;
	int TextSize = 13;
	int BaseTempo = 150;
	int Highlight1 = 4;
	int Highlight2 = 16;
	int TempoDivider = 6;
	
	int CursorX = 0;			//Xpos of the channel cursor, not the mouse cursor
	int CursorY = 0;			//Ypos of the channel cursor, not the mouse cursor
	int CursorPos = 0;			//This is specifically for the individual elements in the effects chain
	bool HoverNote = false;
	bool HoverInst = false;
	bool HoverVolume = false;
	bool HoverEffect = false;
	bool HoverValue = false;	//In regards to effect values

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

	vector<Patterns> patterns[8];
	Patterns DefaultPattern;

	int VolumeScaleL = 127;	//(0-127)
	int VolumeScaleR = 127;	//(0-127)

	//Echo settings
	int Delay;			//(0-15)
	int Feedback;		//(0-127)
	int EchoVolL;		//(0-127)
	int EchoVolR;		//(0-127)
	int EchoFilter[8];	//(0-127) Must accumulate to 127 at most!!!!

	//Enums
	enum ChannelEditState {
		NOTE = 0,
		INSTR = 1,
		VOLUME = 2,
		EFFECT = 3,
		VALUE = 4,
	};

	int SongLength = 1;
	int Maxindex = 8;

	//Functions
	void Initialise(int StartLength);
	void Run(void);
	void CheckInput();
	void Render();

	void MenuBar();
	void Patterns_View();
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

	void ChannelInput(int CurPos, int x, int y);
	void LoadSample();
	void DownMix(SNDFILE* sndfile, SF_INFO sfinfo, Sint16 outputBuffer[]);
	void UpdatePatternIndex(int x, int y);
	void ChangePatternData(int x, int y, int i);
	void UpdateRows();
	void BRRConverter();

	string Authbuf;
	string Descbuf;
	string Output = " ";
	string FilePath = " ";
	string FileName = " ";
	string ADSRNames[4] = {
		"Direct (cut on release)",
		"Effective (linear decrease)",
		"Effective (exponential decrease)",
		"Delayed (write r on release)"
	};

	ImVec4 AttackColour = ImVec4(0, 1, 1, 1);
	ImVec4 DecayColour = ImVec4(0, 1, 0, 1);
	ImVec4 SustainColour = ImVec4(1, 1, 0, 1);
	ImVec4 ReleaseColour = ImVec4(1, .25, .25, 1);
};
#endif // DEBUG