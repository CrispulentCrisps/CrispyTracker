#include "SettingsManager.h"

bool SettingsManager::CheckSettingsFolder()
{
#if _WIN32

	char pBuf[4096];
	int len = sizeof(pBuf);
	GetCurrentDirectoryA(len, pBuf);
	//int bytes = GetModuleFileNameA(0, pBuf, len);
	FilePath = pBuf;
	//FilePath.resize(FilePath.length() - 17);
	FilePath.append("\\Settings");
	printf(FilePath.c_str());
	if (_mkdir(FilePath.c_str()))
	{
		if (ERROR_ALREADY_EXISTS != GetLastError())
		{
			CreateSettings();
			printf("\nFOLDER ALREADY EXISTS:");
			return true;
		}
		else
		{
			CreateSettings();
			printf("\nFOLDER CREATED:");
			return true;
		}
	}
	else
	{
		printf("\ERROR, DIR UNABLE TO BE MADE:");
		return false;
	}

#endif // _WIN32
}

bool SettingsManager::CreateSettings()
{
	std::string FileDest = FilePath.append("\\Settings.txt");
	printf("\n");
	printf(FileDest.c_str());
	SettingsDatastream.open(FileDest, std::ios_base::out | std::ios_base::trunc);
	if (SettingsDatastream.is_open())
	{
		//Settings should be placed in order within the file
		printf("\n FILE IS OPEN");
		SettingsDatastream << "FontSize:"				<< DefaultData.FontSize					<< endl;
		SettingsDatastream << "Notation:"				<< DefaultData.NStyle					<< endl;
		SettingsDatastream << "FPS:"					<< DefaultData.FPS						<< endl;
		SettingsDatastream << "Resolution:"				<< DefaultData.Res						<< endl;
		SettingsDatastream << "Buffer:"					<< DefaultData.Buf						<< endl;
		SettingsDatastream << "DefaultTrackSize:"		<< DefaultData.DefaultTrackSize			<< endl;
		SettingsDatastream << "CursorMovesAtStepCount:"	<< DefaultData.CursorMovesAtStepCount	<< endl;
		SettingsDatastream << "DeleteMovesAtStepCount:"	<< DefaultData.DeleteMovesAtStepCount	<< endl;

		SettingsDatastream.close();
		return true;
	}
	else
	{
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


	CustomData.FontSize = DefaultData.FontSize;
	CustomData.NStyle = DefaultData.NStyle;
	CustomData.FPS = DefaultData.FPS;
	CustomData.Res = DefaultData.Res;
	CustomData.Buf = DefaultData.Buf;
	CustomData.DefaultTrackSize = DefaultData.DefaultTrackSize;
	CustomData.DeleteMovesAtStepCount = DefaultData.DeleteMovesAtStepCount;
	CustomData.CursorMovesAtStepCount = CustomData.CursorMovesAtStepCount;
}

void SettingsManager::CloseSettingsStream()
{
	SettingsDatastream.close();
}

void SettingsManager::SetNotation(int* n)
{
	int n_sur = 0;
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

	n = &n_sur;
}

void SettingsManager::SetBuffer(int* b)
{
	int b_sur = 0;
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
	b = &b_sur;
}

void SettingsManager::SetResolution(int* w, int* h)
{
	int w_sur = 0;
	int h_sur = 0;
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
	w = &w_sur;
	h = &h_sur;
}
