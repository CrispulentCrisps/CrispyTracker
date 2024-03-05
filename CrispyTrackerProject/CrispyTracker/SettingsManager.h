#pragma once

#include <fstream>
#include <stdio.h>
#include <string>

#if _WIN32
#include <Windows.h>
#include <libloaderapi.h>
#endif // _WIN32

using namespace std;

class SettingsManager
{
private:
	string FilePath = "";

	void CheckSettingsFolder();
	struct SettingsData
	{
		//This is where we store all the settings data
		int FontSize;
		
		enum NotationStyle
		{
			SharpStyle,
			FlatStyle,
			GermanStyle,
		};

		int FPS;

		enum Resolutions
		{
			Res_3840x2160,
			Res_2560x1440,
			Res_1920x1080,
			Res_1280x720,
		};

		bool CursorMovesAtStepCount;
		bool DeleteMovesAtStepCount;
	};
public:

};