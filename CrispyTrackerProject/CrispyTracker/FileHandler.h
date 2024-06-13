#pragma once
#include "FileDef.h"

#if !CT_UNIX
#include <mmdeviceapi.h>
#include <Audioclient.h>
#endif
#include <math.h>
#include <stdio.h>

class FileHandler
{
public:
	Module mod;
	bool LoadModule(string path, string fielname);
	bool SaveModule(string path, string fielname);
};