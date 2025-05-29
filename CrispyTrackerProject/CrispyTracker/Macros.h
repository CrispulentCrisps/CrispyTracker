#pragma once
#include <stdio.h>
#include <string>
//Macros here define the reserved positions in the tracker commands
#define NULL_COMMAND		256		//Empty =section in rows, denoted with .. or ...
#define RELEASE_COMMAND		257		//Puts the instrument in the release state
#define STOP_COMMAND		258		//Fully stops the audio, most likely done with KOF and setting the volume to 0

//Maximum column values
#define MAX_INSTRUMENT		127
#define MAX_VOLUME			127
#define MAX_EFFECT			255
#define MAX_EFFECT_VALUE	255
#define MAX_PITCH_IND		96
#define MAX_OCTAVE			7

#define BASE_PITCH_RATE		16000

//File handling
#define FILE_EXT			".ctf"
#define FILE_HEAD			0xDEADBEEF
#define DRIVER_PATH			"asm/Cobalt.bin"
#define VERSION_100			0x0100

//File errors
#define FILE_ERORR_01		"FILE CORRUPTED"
#define FILE_ERORR_02		"FILE NOT FOUND"

#define FILE_ERORR_03		"AUDIO FILE LESS THAN 16 SAMPLES LARGE"
#define FILE_ERORR_04		"SAMPLE TOO LARGE TO FIT INTO DSP MEMORY"

#define DRIVER_INSTPTR		(uint16_t)	0x01E0			//Pointer to [Instrument	] data
#define DRIVER_ORDERPTR		(uint16_t)	0x01E2			//Pointer to [Order			] data
#define DRIVER_SFXLISTPTR	(uint16_t)	0x01E4			//Pointer to [SFX Subtunes	] data
#define DRIVER_SFXPATPTR	(uint16_t)	0x01E6			//Pointer to [SFX Patterns	] data
#define DRIVER_SUBPTR		(uint16_t)	0x01E8			//Pointer to [Music Subtunes] data
#define DRIVER_PITCHPTR		(uint16_t)	0x01EA			//Pointer to [PitchTable	] data
#define DRIVER_LOAD_FLAG	(uint16_t)	0x01EF			//Flag to load addresses into stack pointers [0 for driver testing, 1 for final export]
#define DRIVER_CODE			(uint16_t)	0x0200			//Start of the driver data
#define DRIVER_END			(uint16_t)	0x0C00			//End of the driver data
#define DATA_START			(uint16_t)	0x0D00			//Where dynamic data starts for the driver to interpret

#define DRIVER_TICK_VAL		(uint16_t)	0x000B			//Tick timer for tracker
#define DRIVER_KON_STATE	(uint16_t)	0x000C			//KON State

#define DRIVER_FLAG_VAL		(uint16_t)	0x000E			//Handshake byte for communication in driver RAM

#define DRIVER_SINE_IND_VIB	(uint16_t)	0x0036			//Sine wave Vibrato index
#define DRIVER_SINE_IND_TRM	(uint16_t)	0x0046			//Sine wave Tremolando index
#define DRIVER_SINE_IND_PBR	(uint16_t)	0x0056			//Sine wave Panbrello indexx

#define DRIVER_ARP_VAL		(uint16_t)	0x0066			//Value for arpeggios
#define DRIVER_ARP_TIMER	(uint16_t)	0x0076			//Timer for arpeggios

#define DRIVER_VSLIDE_VAL	(uint16_t)	0x0086			//Volume Slide value
#define DRIVER_PORT_VAL		(uint16_t)	0x0096			//Portamento value
#define DRIVER_VIBR_VAL		(uint16_t)	0x00A6			//Vibrato value
#define DRIVER_TREM_VAL		(uint16_t)	0x00B6			//Tremolando value

#define DRIVER_TICK_THRESH	(uint16_t)	0x00DA			//Tick threshold
#define DRIVER_MASTER_VOL	(uint16_t)	0x00ED			//Master volume for track
#define DRIVER_ECHO_VOL		(uint16_t)	0x00EE			//Echo volume for track

#define DRIVER_INST			(uint16_t)	0x014B			//Instruments for music & SFX
#define DRIVER_PITCHES		(uint16_t)	0x015B			//Pitches for music & SFX
#define DRIVER_VOLUME		(uint16_t)	0x017B			//Volumes for music & SFX
#define DRIVER_STOP_FLAGS	(uint16_t)	0x019B			//Stop flags for each channel

#define DRIVER_ROM_ADDR		(uint32_t)	0x00010000		//Where driver starts in CPU memory

#define DRIVER_PORT0		(uint8_t)	0xF4			//APU-0 register
#define DRIVER_PORT1		(uint8_t)	0xF5			//APU-1 register
#define DRIVER_PORT2		(uint8_t)	0xF6			//APU-2 register
#define DRIVER_PORT3		(uint8_t)	0xF7			//APU-3 register

#define EXCOM_SIZE			0x04

#define DEFAULT_EMU_SPEED	1024000.f

#define MASTERVOL_L			(uint8_t)	0x0C
#define MASTERVOL_R			(uint8_t)	0x1C

#define PMON_REG			(uint8_t)	0x2D
#define NON_REG				(uint8_t)	0x3D
#define EON_REG				(uint8_t)	0x4D
#define KON_REG				(uint8_t)	0x4C
#define KOFF_REG			(uint8_t)	0x5C
#define FLG_REG				(uint8_t)	0x6C

#define CHANNEL_VOL_L		(uint8_t)	0x00
#define CHANNEL_VOL_R		(uint8_t)	0x01
#define CHANNEL_PIT_L		(uint8_t)	0x02
#define CHANNEL_PIT_H		(uint8_t)	0x03
#define CHANNEL_SCRN		(uint8_t)	0x04
#define CHANNEL_ADSR1		(uint8_t)	0x05
#define CHANNEL_ADSR2		(uint8_t)	0x06
#define CHANNEL_GAIN		(uint8_t)	0x07
#define CHANNEL_ENVX		(uint8_t)	0x08
#define CHANNEL_OUTX		(uint8_t)	0x09

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

enum EffectList {
	Arp = 0x00,
	PortUp,
	PortDown,
	PortTo,
	Vib,
	Trem,

	Pan = 0x08,
	Speed,
	VolSlide,
	Goto,
	Break,

	Panbr = 0x20,

	EDel = 0x30,
	EFeed,
	EVolL,
	EVolR,
	EFilt1,
	EFilt2,
	EFilt3,
	EFilt4,
	EFilt5,
	EFilt6,
	EFilt7,
	EFilt8,

	Flag = 0xC0,

	ArpSpeed = 0xE0,
	PortUpCtrl,
	PortDownCtrl,

	TrackVol = 0xE8,

	EndTune = 0xFF
};

typedef struct Command 
{
	ComType type;
	unsigned short val;
};

typedef struct Row {
	unsigned short note =			NULL_COMMAND;
	unsigned short octave =			NULL_COMMAND;
	unsigned short instrument =		NULL_COMMAND;
	unsigned short volume =			NULL_COMMAND;
	unsigned short effect =			NULL_COMMAND;
	unsigned short effectvalue =	NULL_COMMAND;
	unsigned short effect2 =		NULL_COMMAND;
	unsigned short effectvalue2 =	NULL_COMMAND;
};

//Used for tracking row values when writing sequence data
typedef struct RowState {
	bool IsPan;					//If the current row has a panning command
	short PanVal;				//Panning value
};

//Pattern specific entries for data writes
typedef struct PatternState {
	char SleepCount;			//Count how many rows a given channel has slept
	bool IsEmpty;				//Check if line has to be sleep command for channel	//Counter for how many lines to sleep
	bool LastEmpty;				//Check for if the state of last empty changed
	char lastvolume;			//Deduplication for volume commands
	char lastinst;				//Deduplication for instrument commands
};

typedef struct Effect {
	EffectList type;			//Type of effect on hand
	char val;					//Effect value
	char timerval;				//Effect sine timer [when applicable: $00, $04, $20, 
};

//Channel writes
typedef struct ChannelState {
	short vol;					//Current volume of a given channel, includes panning
	short pit;					//Current pitch of a channel
	char inst;					//Current instrument of a channel 
	//Available effects are
	//	$00	|	Arpeggio
	//	$01	|	Portamento up
	//	$02	|	Portamento down	
	//	$03	|	Portamento to
	//	$04	|	Vibrato
	//	$05	|	Tremolando
	//	$0A	|	Volume slide
	//	$20	|	Panbrello
	//	Each of these has to be tracked for the tune to make sense mid play
	Effect fx[8];				//Current effects state
};

enum ExportTypes {
	WAV,
	MP3,
	OPUS,
	VORBIS,
	FLAC,
	SPC,
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

const std::string OpcodeNames[256] = {
//	0x00				0x01				0x02				0x03				0x04				0x05				0x06				0x07				0x08				0x09				0x0A				0x0B				0x0C				0x0D				0x0E				0x0F
	"NOP			",	"TCALL 0		",	"SET1 aa.0		",	"BBS aa.0, rr	",	"OR A,aa		",	"OR A,aaaa		",	"OR A,(X)		",	"OR A,(aa+X)	",	"OR A,#nn		",	"OR aa,aa		",	"OR1 C,aaa.b	",	"ASL aa			",	"ASL aaaa		",	"PUSH PSW		",	"TSET1 aaaa		",	"BRK			",	//0x00
	"BPL rr			",	"TCALL 1		",	"CLR1 aa.0		",	"BBC aa.0, rr	",	"OR A,aa+X		",	"OR A,aaaa+X	",	"OR A,aaaa+Y	",	"OR A,(aa)+Y	",	"OR aa,#nn		",	"OR (X),(Y)		",	"DECW aa		",	"ASL aa+X		",	"ASL A			",	"DEC X			",	"CMP X,aaaa		",	"JMP (aaaa+X)	",	//0x10
	"CLRP			",	"TCALL 2		",	"SET1 aa.1		",	"BBS aa.1, rr	",	"AND A,aa		",	"AND A,aaaa		",	"AND A,(X)		",	"AND A,(aa+X)	",	"AND A,#nn		",	"AND aa,aa		",	"OR1 C,!aaa.b	",	"ROL aa			",	"ROL aaaa		",	"PUSH A			",	"CBNE aa,rr		",	"BRA rr			",	//0x20
	"BMI rr			",	"TCALL 3		",	"CLR1 aa.1		",	"BBC aa.1, rr	",	"AND A,aa+X		",	"AND A,aaaa+X	",	"AND A,aaaa+Y	",	"AND A,(aa)+Y	",	"AND aa,#nn		",	"AND (X),(Y)	",	"INCW aa		",	"ROL aa+X		",	"ROL A			",	"INC X			",	"CMP X,aa		",	"CALL aaaa		",	//0x30
	"SETP			",	"TCALL 4		",	"SET1 aa.2		",	"BBS aa.2, rr	",	"XOR A,aa		",	"XOR A,aaaa		",	"XOR A,(X)		",	"XOR A,(aa+X)	",	"XOR A,#nn		",	"XOR aa,aa		",	"AND1 C,aaa.b	",	"LSR aa			",	"LSR aaaa		",	"PUSH X			",	"TCLR1 aaaa		",	"PCALL aa		",	//0x40
	"BVC rr			",	"TCALL 5		",	"CLR1 aa.2		",	"BBC aa.2, rr	",	"XOR A,aa+X		",	"XOR A,aaaa+X	",	"XOR A,aaaa+Y	",	"XOR A,(aa)+Y	",	"XOR aa,#nn		",	"XOR (X),(Y)	",	"CMPW YA,aa		",	"LSR aa+X		",	"LSR A			",	"MOV X,A		",	"CMP Y,aaaa		",	"JMP aaaa		",	//0x50
	"CLRC			",	"TCALL 6		",	"SET1 aa.3		",	"BBS aa.3, rr	",	"CMP A,aa		",	"CMP A,aaaa		",	"CMP A,(X)		",	"CMP A,(aa+X)	",	"CMP A,#nn		",	"CMP aa,aa		",	"AND1 C,!aaa.b	",	"ROR aa			",	"ROR aaaa		",	"PUSH Y			",	"DBNZ aa,rr		",	"RET			",	//0x60
	"BVS rr			",	"TCALL 7		",	"CLR1 aa.3		",	"BBC aa.3, rr	",	"CMP A,aa+X		",	"CMP A,aaaa+X	",	"CMP A,aaaa+Y	",	"CMP A,(aa)+Y	",	"CMP aa,#nn		",	"CMP (X),(Y)	",	"ADDW YA,aa		",	"ROR aa+X		",	"ROR A			",	"MOV A,X		",	"CMP Y,aa		",	"RETI			",	//0x70
	"SETC			",	"TCALL 8		",	"SET1 aa.4		",	"BBS aa.4, rr	",	"ADC A,aa		",	"ADC A,aaaa		",	"ADC A,(X)		",	"ADC A,(aa+X)	",	"ADC A,#nn		",	"ADC aa,aa		",	"XOR1 C,aaa.b	",	"DEC aa			",	"DEC aaaa		",	"MOV Y,#nn		",	"POP PSW		",	"MOV aa,#nn		",	//0x80
	"BCC rr			",	"TCALL 9		",	"CLR1 aa.4		",	"BBC aa.4, rr	",	"ADC A,aa+X		",	"ADC A,aaaa+X	",	"ADC A,aaaa+Y	",	"ADC A,(aa)+Y	",	"ADC aa,#nn		",	"ADC (X),(Y)	",	"SUBW YA,aa		",	"DEC aa+X		",	"DEC A			",	"MOV X,SP		",	"DIV YA,X		",	"XCN A			",	//0x90
	"EI				",	"TCALL A		",	"SET1 aa.5		",	"BBS aa.5, rr	",	"SBC A,aa		",	"SBC A,aaaa		",	"SBC A,(X)		",	"SBC A,(aa+X)	",	"SBC A,#nn		",	"SBC aa,aa		",	"MOV1 C,aaa.b	",	"INC aa			",	"INC aaaa		",	"CMP Y,#nn		",	"POP A			",	"MOV (X)+,A		",	//0xA0
	"BCS rr			",	"TCALL B		",	"CLR1 aa.5		",	"BBC aa.5, rr	",	"SBC A,aa+X		",	"SBC A,aaaa+X	",	"SBC A,aaaa+Y	",	"SBC A,(aa)+Y	",	"SBC aa,#nn		",	"SBC (X),(Y)	",	"MOVW YA,aa		",	"INC aa+X		",	"INC A			",	"MOV SP,X		",	"DAS A			",	"MOV A,(X)+		",	//0xB0
	"DI				",	"TCALL C		",	"SET1 aa.6		",	"BBS aa.6, rr	",	"MOV aa,A		",	"MOV aaaa,A		",	"MOV (X),A		",	"MOV (aa+X),A	",	"CMP X,#nn		",	"MOV aaaa,X		",	"MOV1 aaaa.b,C	",	"MOV aa,Y		",	"MOV aaaa,Y		",	"MOV X,#nn		",	"POP X			",	"MUL YA			",	//0xC0
	"BNE rr			",	"TCALL D		",	"CLR1 aa.6		",	"BBC aa.6, rr	",	"MOV aa+X,A		",	"MOV aaaa+X,A	",	"MOV aaaa+Y,A	",	"MOV (aa)+Y,A	",	"MOV aa,X		",	"MOV aa+Y,X		",	"MOVW aa,YA		",	"MOV aa+X,Y		",	"DEC Y			",	"MOV A,Y		",	"CBNE aa+X,rr	",	"DAA A			",	//0xD0
	"CLRV			",	"TCALL E		",	"SET1 aa.7		",	"BBS aa.7, rr	",	"MOV A,aa		",	"MOV A,aaaa		",	"MOV A,(X)		",	"MOV A,(aa+X)	",	"MOV A,#nn		",	"MOV X,aaaa		",	"NOT1 aaa.b		",	"MOV Y,aa		",	"MOV Y,aaaa		",	"NOTC			",	"POP Y			",	"SLEEP			",	//0xE0
	"BEQ rr			",	"TCALL F		",	"CLR1 aa.7		",	"BBC aa.7, rr	",	"MOV A,aa+X		",	"MOV A,aaaa+X	",	"MOV A,aaaa+Y	",	"MOV A,(aa)+Y	",	"MOV X,aa		",	"MOV X,aa+Y		",	"MOV aa,bb		",	"MOV Y,aa+X		",	"INC Y			",	"MOV Y,A		",	"DBNZ Y,rr		",	"STOP			",	//0xF0
};