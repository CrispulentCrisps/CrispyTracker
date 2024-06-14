#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "FileDef.h"
#include "miniz.h"

#if !CT_UNIX
#include <mmdeviceapi.h>
#include <Audioclient.h>
#endif
#include <assert.h>
#include <math.h>
#include <stdio.h>

class FileHandler
{
public:
	Module mod;
	bool LoadModule(string path);
	bool SaveModule(string path);
	bool Compress = true;
};