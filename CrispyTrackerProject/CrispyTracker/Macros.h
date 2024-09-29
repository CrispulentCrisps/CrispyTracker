#pragma once

//Macros here define the reserved positions in the tracker commands
#define NULL_COMMAND 256		//Empty =section in rows, denoted with .. or ...
#define RELEASE_COMMAND 257		//Puts the instrument in the release state
#define STOP_COMMAND 258		//Fully stops the audio, most likely done with KOF and setting the volume to 0

//Maximum column values
#define MAX_INSTRUMENT 127
#define MAX_VOLUME 127
#define MAX_EFFECT 255
#define MAX_EFFECT_VALUE 255

//File handling
#define FILE_EXT ".ctf"
#define FILE_HEAD 0xDEADBEEF

//File errors
#define FILE_ERORR_01 "FILE CORRUPTED"
#define FILE_ERORR_02 "FILE NOT FOUND"

#define FILE_ERORR_03 "AUDIO FILE LESS THAN 16 SAMPLES LARGE"
#define FILE_ERORR_04 "SAMPLE TOO LARGE TO FIT INTO DSP MEMORY"

//Tracker command bytes for SPC export
enum Commands 
{
	Com_SetSpeed,
	Com_Sleep,
	Com_Goto,
	Com_Break,
	Com_PlayNote,
	Com_PlayPitch,
	Com_SetInstrument,
	Com_SetFlagValue,
	Com_EchoDelay,
	Com_EchoVolume,
	Com_EchoFeedback,
	Com_EchoCoef1,
	Com_EchoCoef2,
	Com_EchoCoef3,
	Com_EchoCoef4,
	Com_EchoCoef5,
	Com_EchoCoef6,
	Com_EchoCoef7,
	Com_EchoCoef8,
	Com_MasterVol,
	Com_ChannelVol,
	Com_SetArp,
	Com_SetPort,
	Com_SetVibrato,
	Com_SetTremo,
	Com_SetVolSlide,
	Com_SetPanbrello,
	Com_ReleaseNote,
	Com_VirtSpeed,
	Com_VirtBreak,
	Com_VirtStop,
};