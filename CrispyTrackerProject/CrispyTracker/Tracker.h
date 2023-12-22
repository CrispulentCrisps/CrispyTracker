#pragma once
#ifndef DEBUG

#include <SDL_keyboard.h>

#include "Sample.h"
#include "Instrument.h"
#include "Patterns.h"

#include "Libraries/imgui/backends/imgui_impl_opengl3_loader.h"
#include "Libraries/glfw-3.3.8/glfw-3.3.8/include/GLFW/glfw3.h"
#include "Libraries/glfw-3.3.8/glfw-3.3.8/include/GLFW/glfw3native.h"
#include "Libraries/imgui/backends/imgui_impl_glfw.h"
#include "Libraries/imgui/imgui.h"
#include "Libraries/imgui/backends/imgui_impl_sdl2.h"
#include "Libraries/imgui/backends/imgui_impl_opengl3.h"
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
	int TABLE_FLAGS = ImGuiTableFlags_SizingStretchProp;
	int TRACKER_AUDIO_BUFFER = 1024;
	int SPS = 41000;
	string VERSION = "version: 0.3";
	int AUDIO_FORMATS = SF_FORMAT_WAV;
	int FrameCount = 0;
	int Event;
	int Currentkey;
	bool IsPressed = false;
	bool EditingMode = true;
	bool PlayingMode = false;
	float TickTimer = 0;//For when the tracker is running
	int IDOffset = 0;
	
	GLFWwindow* window = NULL;
	ImGuiContext* cont = NULL;
	ImGuiIO io;
	Channel Channels[8];
	vector<Patterns> pattern;
	int MAX_VALUE = 256;

	
	
	ImColor Default = IM_COL32(22, 22, 33, 255);
	ImColor H2Col = IM_COL32(66, 66, 88, 255);
	ImColor H1Col = IM_COL32(33, 33, 66, 255);
	ImColor CursorCol = IM_COL32(122, 122, 188, 255);
	
	ImColor Editing_H2Col = IM_COL32(66, 88, 110, 255);
	ImColor Editing_H1Col = IM_COL32(33, 66, 88, 255);
	
	
	int NoteInput[24] = 
	{
		//Lower octave
		GLFW_KEY_Z, GLFW_KEY_S, GLFW_KEY_X, GLFW_KEY_D,
		GLFW_KEY_C, GLFW_KEY_V, GLFW_KEY_G, GLFW_KEY_B,
		GLFW_KEY_H, GLFW_KEY_N, GLFW_KEY_J, GLFW_KEY_M,
		//Higher octave
		GLFW_KEY_Q , GLFW_KEY_2, GLFW_KEY_W, GLFW_KEY_3, 
		GLFW_KEY_3, GLFW_KEY_R, GLFW_KEY_5, GLFW_KEY_T, 
		GLFW_KEY_6, GLFW_KEY_Y, GLFW_KEY_7, GLFW_KEY_U,
	};

	int VolInput[16] =
	{
		GLFW_KEY_0, GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, 
		GLFW_KEY_4, GLFW_KEY_5, GLFW_KEY_6, GLFW_KEY_7, 
		GLFW_KEY_8, GLFW_KEY_9, GLFW_KEY_A, GLFW_KEY_B, 
		GLFW_KEY_C, GLFW_KEY_D, GLFW_KEY_E, GLFW_KEY_F,
	};//Should really be called Hex Input but I can't be bothered :]
	
	int ArrowInput[4] =
	{
		GLFW_KEY_LEFT, GLFW_KEY_RIGHT, GLFW_KEY_UP, GLFW_KEY_DOWN,
	};
	
	
	int TrackLength = 64;
	int YPos;
	int Input;
	int Step = 1;
	int Octave = 4;
	int ChannelColumn = 0, ChannelRow = 0;
	bool PlayingTrack;



	const int TextSize = 11;
	const int TextSizeLarge = TextSize*2;
	int BaseTempo = 150;
	int Speed1 = 6;
	int TickCounter = 0;
	int Highlight1 = 4;
	int Highlight2 = 16;
	int TempoDivider = 1;
	


	float MinKeyTime = 0.02f;
	float KeyTimer = 0;

	float InitKeyTime = 0.5f;
	float InitTimer = 0;
	bool InitPressed = false;



	ImFont* font;
	ImFont* Largefont;



	int CursorX = 0;			//Xpos of the channel cursor, not the mouse cursor
	int CursorY = 0;			//Ypos of the channel cursor, not the mouse cursor
	int CursorPos = 0;			//This is specifically for the individual elements in the effects chain
	int PatternIndex = 0;		//This is for the currently scelected index



	bool ShowSettings = false;
	


	int SelectedInst = 0;
	int SelectedSample = 0;
	int SelectedPattern = 0;
	


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



	//This is the patterns that is Displayed with
	vector<Patterns> patterns[8];//Display

	//This is the patterns that is Stored so that when a pattern index is removed the data is preserved
	vector<Patterns> StoragePatterns;//Storage
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
		NOTE =		0,
		INSTR =		1,
		VOLUME =	2,
		EFFECT =	3,
		VALUE =		4,
	};

	int SongLength = 1;
	int Maxindex = 8;

	
	
	//Functions
	void Initialise(int StartLength);
	void Run();
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
	void SetupInstr();



	void RunTracker(); //For when the tracker is playing the tune
	void UpdateRows(); //Sending the audio data to the speakers for every element in the tracker row


	void ChannelInput(int CurPos, int x, int y);
	void LoadSample();
	void DownMix(SNDFILE* sndfile, SF_INFO sfinfo, Sint16 outputBuffer[]);
	void UpdatePatternIndex(int x, int y);
	void ChangePatternData(int x, int y, int i);
	void BRRConverter();



	string Authbuf;
	string Descbuf;
	string FilePath = " ";
	string FileName = " ";
	string ADSRNames[4] = {
		"Direct (cut on release)",
		"Effective (linear decrease)",
		"Effective (exponential decrease)",
		"Delayed (write r on release)"
	};

	string Credits = "Crispytracker   |   Tracker Code: Crisps, Alexmush, Euly  |   Optimisation Help: Myself806   |   Emulator Code: SPC Player   |   Driver Code: Kulor (hopefully)";



	ImVec4 AttackColour = ImVec4(0, 1, 1, 1);
	ImVec4 DecayColour = ImVec4(0, 1, 0, 1);
	ImVec4 SustainColour = ImVec4(1, 1, 0, 1);
	ImVec4 ReleaseColour = ImVec4(1, .33, .33, 1);
};
#endif // DEBUG