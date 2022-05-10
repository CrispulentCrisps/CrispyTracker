#include "Tracker.h"

int main()
{
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    Tracker tracker;
    tracker.Run();
    CoUninitialize();
    return 0;
}