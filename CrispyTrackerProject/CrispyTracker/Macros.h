#pragma once

//Macros here define the reserved positions in the tracker commands
#define NULL_COMMAND		256		//Empty =section in rows, denoted with .. or ...
#define RELEASE_COMMAND		257		//Puts the instrument in the release state
#define STOP_COMMAND		258		//Fully stops the audio, most likely done with KOF and setting the volume to 0

//Maximum column values
#define MAX_INSTRUMENT		127
#define MAX_VOLUME			127
#define MAX_EFFECT			255
#define MAX_EFFECT_VALUE	255

//File handling
#define FILE_EXT ".ctf"
#define FILE_HEAD 0xDEADBEEF
#define DRIVER_PATH	"asm/Cobalt.bin"
#define VERSION_100	0x0100

//File errors
#define FILE_ERORR_01 "FILE CORRUPTED"
#define FILE_ERORR_02 "FILE NOT FOUND"

#define FILE_ERORR_03 "AUDIO FILE LESS THAN 16 SAMPLES LARGE"
#define FILE_ERORR_04 "SAMPLE TOO LARGE TO FIT INTO DSP MEMORY"

#define DRIVER_INSTPTR		(uint16_t)	0x01E0			//Pointer to [Instrument	] data
#define DRIVER_ORDERPTR		(uint16_t)	0x01E2			//Pointer to [Order			] data
#define DRIVER_SFXLISTPTR	(uint16_t)	0x01E4			//Pointer to [SFX Subtunes	] data
#define DRIVER_SFXPATPTR	(uint16_t)	0x01E6			//Pointer to [SFX Patterns	] data
#define DRIVER_SUBPTR		(uint16_t)	0x01E8			//Pointer to [Music Subtunes] data
#define DRIVER_PITCHPTR		(uint16_t)	0x01EA			//Pointer to [PitchTable	] data
#define DRIVER_CODE			(uint16_t)	0x0200			//Start of the driver data
#define DATA_START			(uint16_t)	0x0C00			//Where dynamic data starts for the driver to interpret

#define DRIVER_ROM_ADDR		(uint32_t)	0x010000		//Where driver starts in CPU memory

//Tracker command bytes for SPC export
enum ComType
{
	com_SetSpeed,			//Sets tick threshold for track | $00		
	com_Sleep,				//Sleeps for S amount of rows	| $01		
	com_Goto,				//Break to new order			| $02		
	com_Break,				//Goto next order				| $03		
	com_PlayPitch,			//Plays absolute pitch value	| $04		
	com_SetInstrument,		//Set instrument index			| $05		
	com_SetFlagValue,		//Set FLG register				| $06		
	com_EchoDelay,			//Set echo delay value			| $07		
	com_EchoVolume,			//Set echo L / R volume			| $08		
	com_EchoFeedback,		//Set echo feedback value		| $09		
	com_EchoCoeff,			//Set echo coeffecients			| $0A - $11	
	com_ChannelVol = 0x12,	//Set individual channel volume | $12		
	com_SetArp,				//Set Arpeggio effect value		| $13		
	com_SetPort,			//Set Portamento effect value	| $14		
	com_SetVibrato,			//Set Vibrato effect value		| $15		
	com_SetTremo,			//Set Tremolando effect value	| $16		
	com_SetVolSlide,		//Set Volume Slide effect value | $17		
	com_SetPanbrello,		//Set Panbrello effect value	| $18		
	com_ReleaseNote,		//Set KOFF for given channel	| $19
	com_Stop,				//Set STOP flag for tune		| $1A
	com_MasterVol,			//Set the master volume			| $1B		
	com_PlayNote,			//Play pitch from table         | $20-FF
};

enum ProComType {
	PC_PlayMusic,			//Play music track				| $00
	PC_PlaySfx,				//Play sound effect				| $01
	PC_SetMasterVol,		//Set OutVol byte				| $02
	PC_SetSettings,			//Set settings byte				| $03
	PC_SetDriverDiv,		//Set timer divider				| $04
	PC_MuteChannel,			//Mutes certain channels		| $05
	PC_Pause,				//Flips STOP flag for tune		| $06
	PC_FadeAudio,			//Enables fade routine			| $07
	PC_FadeMax,				//Set maximum volume to fade to | $08
	PC_FadeSpeed,			//Set fade speed				| $09
	PC_ResetAPU,			//Go to IPL rom and load SPC	| $0A
};

typedef struct Command 
{
	ComType type;
	short val;
};

typedef struct Row {
	unsigned short note = NULL_COMMAND;
	unsigned short octave = NULL_COMMAND;
	unsigned short instrument = NULL_COMMAND;
	unsigned short volume = NULL_COMMAND;
	unsigned short effect = NULL_COMMAND;
	unsigned short effectvalue = NULL_COMMAND;
	unsigned short effect2 = NULL_COMMAND;
	unsigned short effectvalue2 = NULL_COMMAND;
};

enum ExportTypes {
	WAV,
	MP3,
	OPUS,
	VORBIS,
	FLAC,
	SPC,
	ASM,
};

enum ExportSign
{
	UNSIGNED,
	SIGNED,
};

enum ExportDepth
{
	EIGHT,
	SIXTEEN,
	TWENTYFOUR,
	THIRTYTWO,
	SIXTYFOUR,
};

//Assuming it's an export for audio
enum ExportQuality {
	KHZ_8,
	KHZ_11,
	KHZ_16,
	KHZ_22,
	KHZ_24,
	KHZ_32,
	KHZ_44,
	KHZ_48,
};