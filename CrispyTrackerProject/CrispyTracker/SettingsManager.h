#pragma once

#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string>
#include <unordered_map>

#if _WIN32
#include <Windows.h>
#include <direct.h>
#include <libloaderapi.h>
#endif // _WIN32

using namespace std;

class SettingsManager
{
public:

	enum NotationStyle
	{
		SharpStyle = 0,
		FlatStyle = 1,
		GermanStyle = 2,
	};

	enum BufferSize
	{
		Buf_512,
		Buf_1024,
		Buf_2048,
		Buf_4096,
		Buf_8192,
	};

	enum Resolutions
	{
		Res_3840x2160 = 0,
		Res_2560x1440 = 1,
		Res_1920x1080 = 2,
		Res_1280x720 = 3,
	};

	string FilePath = "";

	bool CheckSettingsFolder();
	bool CreateSettings();
	void CreateDefaultSettings();
	void CloseSettingsStream();
	void ReadSettingsFile();

	void SetNotation(int* n, bool f);
	void SetBuffer(int* b, bool f);
	void SetResolution(int* w, int* h, bool f);
private:

	const string FileLabels[8] = {
		"FontSize:",
		"Notation:",
		"FPS:",
		"Resolution:",
		"Buffer:",
		"DefaultTrackSize:",
		"CursorMovesAtStepCount:",
		"DeleteMovesAtStepCount:",
	};

	struct SettingsData
	{
		//This is where we store all the settings data
		
		int FontSize;					//Size of the font

		NotationStyle NStyle;			//NotationStyle

		int FPS;						//Frames per second
		
		Resolutions Res;				//Resolutions

		BufferSize Buf;					//BufferSize

		int DefaultTrackSize;			//Default track length

		int CursorMovesAtStepCount;		//Move cursor by the step count
		int DeleteMovesAtStepCount;		//Deleting moves the cursor at the step amount
	};

	fstream SettingsDatastream;
	string FileDest = "";

	//Here we determine the setting we want to change from the text in the settings folder
	unordered_map<string, int> SettingsDict;

public:
	SettingsData DefaultData;
	SettingsData CustomData;
};