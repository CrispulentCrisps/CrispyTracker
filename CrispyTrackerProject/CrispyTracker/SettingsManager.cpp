#include "SettingsManager.h"

void SettingsManager::CheckSettingsFolder()
{
#if _WIN32

	char pBuf[4096];
	size_t len = sizeof(pBuf);
	int bytes = GetModuleFileNameA(NULL, pBuf, len);
	if (bytes != -1)
	{
		FilePath = pBuf;
		if (CreateDirectoryA(FilePath.c_str(), NULL))
		{
			ofstream SettingsDatastream;

			SettingsDatastream.open("Settings.txt");

			SettingsDatastream.close();
		}
		else if (ERROR_ALREADY_EXISTS == GetLastError())
		{
			printf("\nERROR: DIRECTORY CANNOT BE CREATED DUE TO PATH ALREADY EXISTING\n");
		}
	}
	
#endif // WIN64
}