#pragma once
#ifndef Tracker

#include <SDL_keyboard.h>

#include "SettingsManager.h"
#include "Patterns.h"
#include "SoundGenerator.h"
#include "SnesAPUHandler.h"

#include "Libraries/imgui/backends/imgui_impl_opengl3_loader.h"
#include "Libraries/glfw-3.3.8/glfw-3.3.8/include/GLFW/glfw3.h"
#include "Libraries/imgui/backends/imgui_impl_glfw.h"
#include "Libraries/glfw-3.3.8/glfw-3.3.8/include/GLFW/glfw3native.h"
#include "Libraries/imgui/imgui.h"
#include "Libraries/imgui/backends/imgui_impl_opengl3.h"
#include "Libraries/ImGuiFileDialog-0.6.5/ImGuiFileDialog.h"
#include "Libraries/libsndfile/include/sndfile.h"
#include "Libraries/implot-0.16/implot.h"

#if !CT_UNIX
#include <mmdeviceapi.h>
#include <Audioclient.h>
#endif
#include <math.h>

using namespace std;
using namespace ImGui;

class Tracker
{
public:
	Tracker();
	~Tracker();

	SnesAPUHandler Emu_APU;

	int UNIVERSAL_WINDOW_FLAGS = ImGuiWindowFlags_AlwaysAutoResize;
	int TAB_FLAGS = ImGuiTabBarFlags_NoCloseWithMiddleMouseButton;
	int TAB_ITEM_FLAGS = ImGuiTabItemFlags_NoReorder;
	int TABLE_FLAGS = ImGuiTableFlags_SizingStretchProp;
	int IMPLOT_FLAGS = ImPlotFlags_NoFrame | ImPlotFlags_Crosshairs;
	int TRACKER_AUDIO_BUFFER = AUDIO_BUFFER;
	int SPS = 41000;
	string VERSION = "version: 0.4";
	int AUDIO_FORMATS = SF_FORMAT_WAV | SF_FORMAT_OGG | SF_FORMAT_MPEG_LAYER_III;
	int FrameCount = 0;
	int Event;
	int Currentkey;
	int CurrentMod;
	bool IsPressed = false;
	bool EditingMode = true;
	bool PlayingMode = false;
	float TickTimer = 0;//For when the tracker is running
	int IDOffset = 0;
	int FPS = 144;
	int MAX_FPS = 360;

	//Screen dimension constants
	int SCREEN_WIDTH = 1920;
	int SCREEN_HEIGHT = 1080;

	GLFWwindow* window = NULL;
	ImGuiContext* cont = NULL;
	ImGuiIO io;
	Channel Channels[8];
	SoundGenerator SG;
	vector<Patterns> pattern;
	SettingsManager SManager;
	int MAX_VALUE = 256;

	int NoteInput[24] = 
	{
		//Lower octave
		GLFW_KEY_Z, GLFW_KEY_S, GLFW_KEY_X, GLFW_KEY_D,
		GLFW_KEY_C, GLFW_KEY_V, GLFW_KEY_G, GLFW_KEY_B,
		GLFW_KEY_H, GLFW_KEY_N, GLFW_KEY_J, GLFW_KEY_M,
		//Higher octave
		GLFW_KEY_Q, GLFW_KEY_2, GLFW_KEY_W, GLFW_KEY_3, 
		GLFW_KEY_E, GLFW_KEY_R, GLFW_KEY_5, GLFW_KEY_T, 
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
	bool MoveOnDelete = false;
	bool MoveByStep = false;

	int TextSize = 12;
	int TextSizeLarge = TextSize*2;
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

	int SelectionBoxX1 = 0;
	int SelectionBoxSubX1 = 0;
	int SelectionBoxY1 = 0;

	int SelectionBoxX2 = 0;
	int SelectionBoxSubX2 = 0;
	int SelectionBoxY2 = 0;

	bool BoxSelected = false;

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
	int EchoFilter[8];	//(0-127) Must accumulate to 127 or -128 at most!!!!

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
	
	double ScrollValue();

	//Functions
	void Initialise(int StartLength);
	void Run();
	void CheckInput();
	void Render();

	void MenuBar();
	void CreditsWindow();
	void Patterns_View();
	void Instruments();
	void Instrument_View();
	void Channel_View();
	void Samples();
	void Sample_View();
	void Settings_View();
	void Misc_View();
	void Author_View();
	void Info_View();
	void EchoSettings();
	void SetupInstr();

	void RunTracker(); //For when the tracker is playing the tune
	void UpdateRows(); //Sending the audio data to the speakers for every element in the tracker row

	void ChannelInput(int CurPos, int x, int y);
	void LoadSample();
	void DownMix(SNDFILE* sndfile, SF_INFO sfinfo, Sint16 outputBuffer[]);
	void UpdatePatternIndex(int x, int y);
	void UpdateAllPatterns();
	void ChangePatternData(int x, int y);
	void UpdateSettings();
	void ResetSettings();

	string Authbuf = " ";
	string Descbuf = " ";
	string FilePath = " ";
	string FileName = " ";
	string ADSRNames[4] = {
		"Direct (cut on release)",
		"Effective (linear decrease)",
		"Effective (exponential decrease)",
		"Delayed (write r on release)"
	};

	string Credits = "Crispytracker: " + VERSION;//Is actually used for the title

	ImColor Default = IM_COL32(22, 22, 33, 255);
	ImColor H2Col = IM_COL32(66, 66, 88, 255);
	ImColor H1Col = IM_COL32(33, 33, 66, 255);
	ImColor CursorCol = IM_COL32(122, 122, 188, 255);

	ImColor Dark_Default = IM_COL32(22, 22, 33, 255);
	ImColor Dark_H2Col = IM_COL32(66, 66, 88, 255);
	ImColor Dark_H1Col = IM_COL32(33, 33, 66, 255);
	ImColor Dark_CursorCol = IM_COL32(122, 122, 188, 255);

	ImColor Editing_H2Col = IM_COL32(66, 88, 110, 255);
	ImColor Editing_H1Col = IM_COL32(33, 66, 88, 255);

	ImColor SelectionBoxCol = IM_COL32(66, 66, 128, 255);

	ImVec4 AttackColour = ImVec4(.44, .44, .88, 1);
	ImVec4 DecayColour = ImVec4(.22, .88, .22, 1);
	ImVec4 SustainColour = ImVec4(.88, .88, .22, 1);
	ImVec4 ReleaseColour = ImVec4(.77, .33, .33, 1);


	ImColor Graph_Colour_Line1 = IM_COL32(99, 99, 196, 255);
	ImColor Graph_Colour_Line2 = IM_COL32(66, 66, 128, 255);
	ImColor Graph_Colour_Line3 = IM_COL32(33, 33, 64, 255);
	
	int PlotLineWeight = 1;
	ImU32 colorDataRGB[3] = { Graph_Colour_Line1, Graph_Colour_Line2, Graph_Colour_Line3 };
	int PlotColours = 0;

	ImAxis x;
	ImAxis y;
};
#endif // Tracker
