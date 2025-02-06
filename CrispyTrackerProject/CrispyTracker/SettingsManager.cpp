#include "SettingsManager.h"

bool SettingsManager::CheckSettingsFolder()
{
	cout << "\n" << GetLastError();
	for (int x = 0; x < sizeof(FileLabels)/sizeof(FileLabels[0]); x++)
	{
		SettingsDict.insert(make_pair(FileLabels[x], x));
	}

#if _WIN32
	char pBuf[8192];
	int len = sizeof(pBuf);
	GetCurrentDirectoryA(len, pBuf);
	//int bytes = GetModuleFileNameA(0, pBuf, len);
	FilePath = pBuf;
	//FilePath.resize(FilePath.length() - 17);
	printf(FilePath.c_str());
	FileDest = FilePath.append("\\Settings.txt");
	printf("\n");
	printf(FileDest.c_str());
	return true;
#endif // _WIN32
}

bool SettingsManager::CreateSettings()
{
	ifstream FileCheck;
	SettingsDatastream.open(FileDest, ios_base::out | ios_base::trunc);
	if (SettingsDatastream.is_open())
	{
		//Settings should be placed in order within the file
		printf("\n FILE IS OPEN");
		SettingsDatastream << FileLabels[0] << DefaultData.FontSize << "\n";
		SettingsDatastream << FileLabels[1] << DefaultData.NStyle << "\n";
		SettingsDatastream << FileLabels[2] << DefaultData.FPS << "\n";
		SettingsDatastream << FileLabels[3] << DefaultData.Res << "\n";
		SettingsDatastream << FileLabels[4] << DefaultData.Buf << "\n";
		SettingsDatastream << FileLabels[5] << DefaultData.DefaultTrackSize << "\n";
		SettingsDatastream << FileLabels[6] << DefaultData.CursorMovesAtStepCount << "\n";
		SettingsDatastream << FileLabels[7] << DefaultData.DeleteMovesAtStepCount << "\n";

		SettingsDatastream.close();
		return true;
	}
	else
	{
		SettingsDatastream.close();
		printf("\nERROR: FILE COULD NOT BE CREATED");
		return false;
	}
}

void SettingsManager::CreateDefaultSettings()
{
	DefaultData.FontSize = 12;
	DefaultData.NStyle = SharpStyle;
	DefaultData.FPS = 60;
	DefaultData.Res = Res_1920x1080;
	DefaultData.Buf = Buf_1024;
	DefaultData.DefaultTrackSize = 64;
	DefaultData.DeleteMovesAtStepCount = 0;
	DefaultData.CursorMovesAtStepCount = 0;
	
	CustomData = DefaultData;
}

void SettingsManager::CloseSettingsStream()
{
	SettingsDatastream.close();
}
//Opens the settings file and reads the contents of it
void SettingsManager::ReadSettingsFile()
{
	int BreakCounter = 0;//Used to avoid infinite while loops
	ifstream Input(FileDest);

	for (int x = 0; x < 8; x++)
	{
		string CurrentLine = "\n";
		while (getline(Input, CurrentLine) && BreakCounter < 99)
		{
			int CurrentValue = CurrentLine.find(':')+1;
			if (CurrentValue != string::npos)
			{
				int InputValue = atoi(CurrentLine.substr(CurrentValue).c_str());
				
				CurrentLine.resize(CurrentValue);
				
				if (SettingsDict.find(CurrentLine) != SettingsDict.end())//Assuming we found the element
				{
					int CurrentSetting = SettingsDict.at(CurrentLine);
					cout << "\nCurrent Line: " << CurrentLine;
					cout << "\nCurrent Input: " << InputValue;
					switch (CurrentSetting)
					{
					case 0:
						DefaultData.FontSize = InputValue;
						break;
					case 1:
						DefaultData.NStyle = SetNotation(InputValue);
						break;
					case 2:
						DefaultData.FPS = InputValue;
						break;
					case 3:
						DefaultData.Res = SetResolution(InputValue);
						break;
					case 4:
						DefaultData.Buf = SetBuffer(InputValue);
						break;
					case 5:
						DefaultData.DefaultTrackSize = InputValue;
						break;
					case 6:
						DefaultData.CursorMovesAtStepCount = InputValue;
						break;
					case 7:
						DefaultData.DeleteMovesAtStepCount = InputValue;
						break;
					}
				}
				else
				{
					cout << "\nERROR: SETTING " << CurrentLine << " CANNOT BE FOUND\n";
				}
			}
			BreakCounter++;
			//printf(CurrentLine.c_str());
		}
	}
	SetCustomDataToDefault();
	Input.close();
}

void SettingsManager::SetDefaultDataToCustom()
{
	DefaultData = CustomData;
}

void SettingsManager::SetCustomDataToDefault()
{
	CustomData = DefaultData;
}

void SettingsManager::GetNotation(int& n, bool f)
{
	int n_sur = 0;
	if (f)
	{
		switch (CustomData.NStyle)
		{
		case 0:
			n_sur = 0;
			break;
		case 1:
			n_sur = 1;
			break;
		case 2:
			n_sur = 2;
			break;
		default:
			n_sur = 0;
			break;
		}
	}
	else
	{
		switch (DefaultData.NStyle)
		{
		case 0:
			n_sur = 0;
			break;
		case 1:
			n_sur = 1;
			break;
		case 2:
			n_sur = 2;
			break;
		default:
			n_sur = 0;
			break;
		}

	}
	n = n_sur;
}

void SettingsManager::GetBuffer(int& b, bool f)
{
	int b_sur = 0;
	if (f)
	{
		switch (CustomData.Buf)
		{
		case Buf_512:
			b_sur = 512;
			break;
		case Buf_1024:
			b_sur = 1024;
			break;
		case Buf_2048:
			b_sur = 2048;
			break;
		case Buf_4096:
			b_sur = 4096;
			break;
		case Buf_8192:
			b_sur = 8192;
			break;
		default:
			b_sur = 1024;
			break;
		}
	}
	else
	{
		switch (DefaultData.Buf)
		{
		case Buf_512:
			b_sur = 512;
			break;
		case Buf_1024:
			b_sur = 1024;
			break;
		case Buf_2048:
			b_sur = 2048;
			break;
		case Buf_4096:
			b_sur = 4096;
			break;
		case Buf_8192:
			b_sur = 8192;
			break;
		default:
			b_sur = 1024;
			break;
		}
	}
	b = b_sur;
}

void SettingsManager::GetResolution(int& w, int& h, bool f)
{
	int w_sur = 0;
	int h_sur = 0;
	if (f)
	{
		switch (CustomData.Res)
		{
		case Res_3840x2160:
			w_sur = 3840;
			h_sur = 2160;
			break;
		case Res_2560x1440:
			w_sur = 2560;
			h_sur = 1440;
			break;
		case Res_1920x1080:
			w_sur = 1920;
			h_sur = 1080;
			break;
		case Res_1280x720:
			w_sur = 1280;
			h_sur = 720;
			break;
		default:
			w_sur = 1920;
			h_sur = 1080;
			break;
		}
	}
	else
	{
		switch (DefaultData.Res)
		{
		case Res_3840x2160:
			w_sur = 3840;
			h_sur = 2160;
			break;
		case Res_2560x1440:
			w_sur = 2560;
			h_sur = 1440;
			break;
		case Res_1920x1080:
			w_sur = 1920;
			h_sur = 1080;
			break;
		case Res_1280x720:
			w_sur = 1280;
			h_sur = 720;
			break;
		default:
			w_sur = 1920;
			h_sur = 1080;
			break;
		}
	}
	w = w_sur;
	h = h_sur;
}

SettingsManager::NotationStyle SettingsManager::SetNotation(int index)
{
	switch (index)
	{
	default:
		return SharpStyle;
		break;
	case 0:
		return SharpStyle;
		break;
	case 1:
		return FlatStyle;
		break;
	case 2:
		return GermanStyle;
		break;
	}
}

SettingsManager::Resolutions SettingsManager::SetResolution(int index)
{
	switch (index)
	{
	default:
		return Res_1920x1080;
		break;
	case 0:
		return Res_3840x2160;
		break;
	case 1:
		return Res_2560x1440;
		break;
	case 2:
		return Res_1920x1080;
		break;
	case 3:
		return Res_1280x720;
		break;
	}
}

SettingsManager::BufferSize SettingsManager::SetBuffer(int index)
{
	switch (index)
	{
	default:
		return Buf_1024;
		break;
	case 0:
		return Buf_512;
		break;
	case 1:
		return Buf_1024;
		break;
	case 2:
		return Buf_2048;
		break;
	case 3:
		return Buf_4096;
		break;
	case 4:
		return Buf_8192;
		break;
	}
}
