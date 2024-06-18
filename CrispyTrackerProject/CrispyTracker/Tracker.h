//	_____________________________
//	|		Crispytracker		|
//	|	   The SNES tracker		|
//	|		 of all time		|
//	|___________________________|
// 
//Apologies for the anarchistic coding, I've no idea what I'm doing lmao
//

#pragma once
#ifndef Tracker

#include "SettingsManager.h"
#include "SoundGenerator.h"
#include "FileHandler.h"

#include <SDL_keyboard.h>

#include "Libraries/imgui/backends/imgui_impl_opengl3_loader.h"
#include "Libraries/glfw-3.3.8/glfw-3.3.8/include/GLFW/glfw3.h"
#include "Libraries/imgui/backends/imgui_impl_glfw.h"
#include "Libraries/glfw-3.3.8/glfw-3.3.8/include/GLFW/glfw3native.h"
#include "Libraries/imgui/imgui.h"
#include "Libraries/imgui/backends/imgui_impl_opengl3.h"
#include "Libraries/ImGuiFileDialog-0.6.5/ImGuiFileDialog.h"
#include "Libraries/libsndfile/include/sndfile.h"
#include "Libraries/implot-0.16/implot.h"
#include "Libraries/implot-0.16/implot_internal.h"

#if !CT_UNIX
#include <mmdeviceapi.h>
#include <Audioclient.h>
#endif
#include <math.h>
#include <stdio.h>

using namespace std;
using namespace ImGui;

class Tracker
{
public:
	Tracker();
	~Tracker();

	int UNIVERSAL_WINDOW_FLAGS = ImGuiWindowFlags_AlwaysAutoResize;
	int TAB_FLAGS = ImGuiTabBarFlags_NoCloseWithMiddleMouseButton;
	int TAB_ITEM_FLAGS = ImGuiTabItemFlags_NoReorder;
	int TABLE_FLAGS = ImGuiTableFlags_SizingStretchSame;
	int IMPLOT_FLAGS = ImPlotFlags_NoFrame | ImPlotFlags_Crosshairs;
	int SPS = 44100;
	string VERSION = "version: 0.7";
	string Fontpath = "fonts/Manaspace.ttf";
	int AUDIO_FORMATS = SF_FORMAT_WAV | SF_FORMAT_OGG | SF_FORMAT_MPEG_LAYER_III | SF_FORMAT_RAW;
	int FrameCount = 0;
	int Event;
	int CurrentKey;
	int CurrentMod;
	bool IsPressed = false;
	bool EditingMode = true;
	bool PlayingMode = false;
	bool AudioStopped = false;
	float TickTimer = 0;//For when the tracker is running
	int IDOffset = 0;
	int FPS = 144;
	int MAX_FPS = 360;
	float TimeHolder = 0;
	float DeltaTimeHolder = 0;
	int MaxTune = 0;
	int CurrentTune = 0;

	//Screen dimension constants
	int SCREEN_WIDTH = 1920;
	int SCREEN_HEIGHT = 1080;

	GLFWwindow* window = NULL;
	GLFWmonitor* monitor = NULL;
	ImGuiContext* cont = NULL;
	ImGuiIO io;
	Channel Channels[8];
	SoundGenerator SG;
	vector<Patterns> pattern;
	SettingsManager SManager;
	int MAX_VALUE = NULL_COMMAND;

	FileHandler filehandler = FileHandler();
	bool SavingFile = false;
	bool LoadingFile = false;

#pragma region Keyboard Controls
	//Inputting notes
	const int NoteInput[24] = 
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

	//Hexadecimal Inputs
	const int VolInput[16] =
	{
		GLFW_KEY_0, GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, 
		GLFW_KEY_4, GLFW_KEY_5, GLFW_KEY_6, GLFW_KEY_7, 
		GLFW_KEY_8, GLFW_KEY_9, GLFW_KEY_A, GLFW_KEY_B, 
		GLFW_KEY_C, GLFW_KEY_D, GLFW_KEY_E, GLFW_KEY_F,
	};
	
	const int ArrowInput[4] =
	{
		GLFW_KEY_LEFT, GLFW_KEY_RIGHT, GLFW_KEY_UP, GLFW_KEY_DOWN,
	};
#pragma endregion

	//Regional
	int SelectedRegion = 0;
	string RegionNames[2] = { "PAL/SECAM", "NTSC" };
	Region reg;
	//

	//Tracker Controls
	int TrackLength = 64;
	int YPos;
	int Input;
	int Step = 1;
	int Octave = 4;
	int ChannelColumn = 0;
	int ChannelRow = 0;
	bool PlayingTrack;
	bool MoveOnDelete = false;
	bool MoveByStep = false;
	//

	//Fonts/Text
	ImFont* font;
	ImFont* Largefont;
	int TextSize = 12;
	int TextSizeLarge = TextSize * 2;
	//

	//TrackerSpeed
	int BaseTempo = 150;
	int Speed1 = 6;
	int TickCounter = 0;
	int TempoDivider = 1;
	//

	//Track visuals
	int Highlight1 = 4;
	int Highlight2 = 16;
	//
	
	//Key Timings
	float MinKeyTime = 0.02f;
	float KeyTimer = 0;
	float InitKeyTime = 0.3f;
	float InitTimer = 0;
	bool InitPressed = false;
	//

	bool ShowEmuDebug = false;
	int MemCols = 32;

	//Tracker Cursor
	int CursorX = 0;			//Xpos of the channel cursor, not the mouse cursor
	int CursorY = 0;			//Ypos of the channel cursor, not the mouse cursor
	int CursorPos = 0;			//This is specifically for the individual elements in the effects chain
	int PatternIndex = 0;		//This is for the currently scelected index
	//

	//Selection box functionality
	int SelectionBoxX1 = 0;
	int SelectionBoxSubX1 = 0;
	int SelectionBoxY1 = 0;
	int SelectionBoxX2 = 0;
	int SelectionBoxSubX2 = 0;
	int SelectionBoxY2 = 0;
	bool BoxSelected = false;

	//
	bool ShowSettings = false;

	//Selected items
	int SelectedInst = 0;
	int SelectedSample = 0;
	int SelectedPattern = 0;

	//Window States
	bool ShowCredits = false;
	bool ShowInstrument = false;
	bool ShowSample = false;
	bool ShowEcho = false;
	bool LoadingSample = false;

	//Visual padding
	int InstXPadding = 32;
	int InstYPadding = 72;

	bool FontUpdate = false;

	//Instruments
	vector<Instrument> inst;
	Instrument DefaultInst;
	
	//Samples
	vector<Sample> samples;
	Sample DefaultSample;

	//This is the order of patterns
	vector<Patterns> orders[8];//Display

	//This is the patterns that is Stored so that when a pattern index is removed the data is preserved
	vector<Patterns> StoragePatterns;//Storage
	Patterns DefaultPattern;

	int VolumeScale = 127;	//(0-127)

	//Echo settings
	int Delay = 0;		//(0-15)
	int Feedback = 64;	//(0-127)
	int EchoVol = 127;	//(0-127)
	int EchoFilter[8];	//(-128-127) Must accumulate to 127 or -128 at most/least respectively!!!!

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

	int InputCount = 0;//Used for the amount of times a value has been input into the volume, effects or value
	
	enum ExportTypes {
		WAV = 0,
		MP3 = 1,
		OGG = 2,
		SPC = 3,
		ASM = 4,
	};

	enum ExportSign
	{
		UNSIGNED = 0,
		SIGNED = 1,
	};

	enum ExportDepth
	{
		EIGHT = 0,
		SIXTEEN = 1,
	};

	//Assuming it's an export for audio
	enum ExportQuality {
		KHZ_8 = 0,
		KHZ_11 = 1,
		KHZ_16 = 2,
		KHZ_22 = 3,
		KHZ_24 = 4,
		KHZ_32 = 5,
		KHZ_44 = 6,
		KHZ_48 = 7,
	};

	ExportTypes E_Type;
	ExportSign E_Sign;
	ExportDepth E_Depth;
	ExportQuality E_Quality;
	int SelectedExportType = 2;
	int SelectedTechnicalType = 0;
	int SelectedSignType = 1;
	int SelectedDepthType = 1;
	int SelectedQualityType = 7;
	bool ShowExport = false;
	
	//Error handling
	bool ShowError = false;
	string ErrorMessage;
	//

	bool ShowDSPDebugger = false;
	bool ShowTrackerDebugger = false;

	string EffectText = "";

	double ScrollValue();//Returns the position scrolled down the tracker

	//Functions
	void Initialise(int StartLength);
	void Run();
	void CheckUpdatables();//for updating things outside of "new frames"
	void CheckInput();
	void Render();

	//Rendering
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
	void Speed_View();
	void Info_View();
	void EchoSettings();
	void SetupInstr();
	void UpdateFont();
	void Export_View();
	void EffectsText(int effect);
	//

	void RunTracker(); //For when the tracker is playing the tune
	void UpdateRows(); //Sending the audio data to the speakers for every element in the tracker row
	void UpdateTicks(); //Updating any tick based effects
	void UpdateAudioBuffer();

	void ChannelInput(int CurPos, int x, int y);
	void LoadSample();
	void DownMix(SNDFILE* sndfile, SF_INFO sfinfo, Sint16 outputBuffer[]);
	void UpdatePatternIndex(int x, int y);
	void UpdateAllPatterns();

	void ChangePatternData(int x, int y);
	void UpdateSettings(int w);
	void ResetSettings();

	void UpdateModule();
	void SaveModuleAs();
	void LoadModuleAs();
	void ApplyLoad();

	void ErrorWindow();
	void DSPDebugWindow();
	void TrackerDebug();

	string FilePath = "";
	string FileName = "";

	char songbuf[256] = { 0 };
	char authbuf[256] = { 0 };
	char descbuf[256] = { 0 };

	string ADSRNames[4] = {
		"Direct (cut on release)",
		"Effective (linear decrease)",
		"Effective (exponential decrease)",
		"Delayed (write r on release)"
	};

	string Credits = "Crispytracker: " + VERSION;//Is actually used for the title
	
	//Visuals
	ImColor WindowBG = IM_COL32(11, 11, 22, 255);
	ImColor Default = IM_COL32(22, 22, 44, 255);
	ImColor H2Col = IM_COL32(66, 66, 88, 255);
	ImColor H1Col = IM_COL32(44, 44, 66, 255);
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

	ImColor ReservedColour = IM_COL32(255, 255, 255, 255);
	ImColor InstColour = IM_COL32(88, 88, 196, 255);
	ImColor SampColour = IM_COL32(144, 144, 255, 255);
	ImColor EchoColour = IM_COL32(44, 44, 128, 255);
	
	int PlotLineWeight = 1;
	ImU32 colorDataRGB[3] = { Graph_Colour_Line1, Graph_Colour_Line2, Graph_Colour_Line3 };
	int PlotColours = 0;

	ImAxis x;
	ImAxis y;
};	
#endif // Tracker
