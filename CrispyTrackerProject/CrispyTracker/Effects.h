#pragma once
#include <iostream>

#define ARPEGGIO		0x00
#define PORT_UP			0x01
#define PORT_DOWN		0x02
#define PORT_TO			0x03
#define VIBRATO			0x04

#define TREMOLANDO		0x07
#define PANNING			0x08
#define SPEED			0x09
#define VOLSLIDE		0x0A
#define GOTO			0x0B
#define RETRIGGER		0x0C
#define BREAK			0x0D

//Instrument settings
#define ADSR_ENV		0x10
#define ADSR_TYP		0x11
#define ADSR_ATK		0x12
#define ADSR_DEC		0x13
#define ADSR_DC2		0x14
#define ADSR_SUS		0x15
#define ADSR_REL		0x16
#define GAIN			0x17
#define INVL			0x18
#define INVR			0x19
#define NOISE_SET		0x1A
#define PITCHMOD		0x1B
#define ECHO			0x1C

#define PANBRELLO		0x20

#define ECHO_DEL		0x30
#define ECHO_FDB		0x31
#define ECHO_L			0x32
#define ECHO_R			0x33
#define ECHO_FIL_1		0x34
#define ECHO_FIL_2		0x35
#define ECHO_FIL_3		0x36
#define ECHO_FIL_4		0x37
#define ECHO_FIL_5		0x38
#define ECHO_FIL_6		0x39
#define ECHO_FIL_7		0x3A
#define ECHO_FIL_8		0x3B

//Flags for music-code interchange
#define FLAG_0			0xC0
#define FLAG_1			0xC1
#define FLAG_2			0xC2
#define FLAG_3			0xC3
#define FLAG_4			0xC4
#define FLAG_5			0xC5
#define FLAG_6			0xC6
#define FLAG_7			0xC7
#define FLAG_8			0xC8
#define FLAG_9			0xC9
#define FLAG_A			0xCA
#define FLAG_B			0xCB
#define FLAG_C			0xCC
#define FLAG_D			0xCD
#define FLAG_E			0xCE
#define FLAG_F			0xCF

//Extended effects
#define ARP_SPEED		0xE0//Controls arpeggio speed
#define PORT_UP_CTRL	0xE1//Portamento up controlled
#define PORT_DOWN_CTRL	0xE2//Portamento down controlled

#define GLOABL_PAN_L	0xE8//Global L
#define GLOABL_PAN_R	0xE9//Global R
#define GLOBAL_VOL		0xEA//Global Volume

#define END				0xFF//Ends the track

#define ARPEGGIO_DESC		"00xy Arpeggio [x: offset 1, y: offset 2] Semitone offset from base note"	
#define PORT_UP_DESC		"01xx Portamento up [xx: speed up] Slides the pitch upwards"	
#define PORT_DOWN_DESC		"02xx Portamento up [xx: speed down] Slides the pitch downwards"
#define PORT_TO_DESC		"03xx Portamento to [xx: speed toward] Slides the pitch to the desired note"	
#define VIBRATO_DESC		"04xy Vibrato [x: speed, y: depth] Oscillates the pitch of the note"
#define TREMOLANDO_DESC		"05xy Tremolando [x: speed, y: depth] Oscillates the volume of the note"

#define PANNING_DESC		"08xy Panning [x: left, y: right] Sets the panning of the channel"	
#define SPEED_DESC			"09xx Speed [xx: speed] Sets the speed of the track"
#define VOLSLIDE_DESC		"0Axy Volume Slide [x: up, y: down] Changes the instrument volume"
#define GOTO_DESC			"0Bxx Goto [xx: set order position] Sets position within orders"
#define RETRIGGER_DESC		"0Cxx Retrigger [xx: frames between triggers] Plays the note the amount of frames specified"
#define BREAK_DESC			"0Dxx Break. Goes to next order"
							
#define ADSR_ENV_DESC		"10xx Envelope Used [0 for off, 1 for on] Tells the instrument if it uses the ADSR envelope"
#define ADSR_TYP_DESC		"11xx Envelope Used [xx: type] Dictates which ADSR type to use"
#define ADSR_ATK_DESC		"12xx Set Attack [xx: value] Sets the attack value"
#define ADSR_DEC_DESC		"13xx Set Decay [xx: value] Sets the decay value"
#define ADSR_DC2_DESC		"14xx Set Decay 2 [xx: value] Sets the second decay value"
#define ADSR_SUS_DESC		"15xx Set Sustain [xx: value] Sets the sustain value"
#define ADSR_REL_DESC		"16xx Set Release [xx: value] Sets the release value"
#define GAIN_DESC			"17xx Set Gain [xx: value] Sets the gain value"
#define INVL_DESC			"18xy Invert Left [x: Inv L, y: Inv R,] Inverts audio on the specific channel"
#define NOISESET_DESC		"19xx Set Noise [0: off, 1: on] Sets audio generator to play"
#define NOISEFREQ_DESC		"1Axx Set Noise Frequency [xx: ] Sets the frequency for the noise generator"
#define PITCHMOD_DESC		"1Bxx Set Pitch Modulation [0: off, 1: on] Enables/Disables the Pitch Mod"
#define ECHO_DESC			"1Cxx Set Echo [0: off, 1: on] Enables/Disables the Echo"
							
#define PANBRLLO_DESC		"20xy Panbrello [x: speed, y: depth] Oscillates the panning of the note"

#define ECHO_DEL_DESC		"30xx Echo Delay [xx: echo value] sets the delay value of the echo [Note! Every echo value consumes 2048 bytes of Audio Ram, this may cause issues within the ROM export and may not be able to fit within the SPC Export!]"
#define ECHO_FDB_DESC		"31xx Echo Feedback [xx: feedback] Sets the Feedback of the echo. A value of 80 is equivelant to a feedback of 0"
#define ECHO_L_DESC			"32xx Echo L Volume [xx: left value] Sets the volume of the Echo Left channel. A value of 80 is equivelant to a feedback of 0"
#define ECHO_R_DESC			"33xx Echo R Volume [xx: right value] Sets the volume of the Echo Right channel. A value of 80 is equivelant to a feedback of 0"
#define ECHO_FIL1_DESC		"34xx Echo Filter Value 1 [xx: value] Sets the value for the Echo Filter [Note! the whole filter buffer can only be from -128 to 127 TOTAL]"
#define ECHO_FIL2_DESC		"35xx Echo Filter Value 2 [xx: value] Sets the value for the Echo Filter [Note! the whole filter buffer can only be from -128 to 127 TOTAL]"
#define ECHO_FIL3_DESC		"36xx Echo Filter Value 3 [xx: value] Sets the value for the Echo Filter [Note! the whole filter buffer can only be from -128 to 127 TOTAL]"
#define ECHO_FIL4_DESC		"37xx Echo Filter Value 4 [xx: value] Sets the value for the Echo Filter [Note! the whole filter buffer can only be from -128 to 127 TOTAL]"
#define ECHO_FIL5_DESC		"38xx Echo Filter Value 5 [xx: value] Sets the value for the Echo Filter [Note! the whole filter buffer can only be from -128 to 127 TOTAL]"
#define ECHO_FIL6_DESC		"39xx Echo Filter Value 6 [xx: value] Sets the value for the Echo Filter [Note! the whole filter buffer can only be from -128 to 127 TOTAL]"
#define ECHO_FIL7_DESC		"3Axx Echo Filter Value 7 [xx: value] Sets the value for the Echo Filter [Note! the whole filter buffer can only be from -128 to 127 TOTAL]"
#define ECHO_FIL8_DESC		"3Bxx Echo Filter Value 8 [xx: value] Sets the value for the Echo Filter [Note! the whole filter buffer can only be from -128 to 127 TOTAL]"

#define FLAG_0_DESC =		"Flag Effect 0 [On or Off] Used for interfacing with the SNES"
#define FLAG_1_DESC =		"Flag Effect 1 [On or Off] Used for interfacing with the SNES"
#define FLAG_2_DESC =		"Flag Effect 2 [On or Off] Used for interfacing with the SNES"
#define FLAG_3_DESC =		"Flag Effect 3 [On or Off] Used for interfacing with the SNES"
#define FLAG_4_DESC =		"Flag Effect 4 [On or Off] Used for interfacing with the SNES"
#define FLAG_5_DESC =		"Flag Effect 5 [On or Off] Used for interfacing with the SNES"
#define FLAG_6_DESC =		"Flag Effect 6 [On or Off] Used for interfacing with the SNES"
#define FLAG_7_DESC =		"Flag Effect 7 [On or Off] Used for interfacing with the SNES"
#define FLAG_8_DESC =		"Flag Effect 8 [On or Off] Used for interfacing with the SNES"
#define FLAG_9_DESC =		"Flag Effect 9 [On or Off] Used for interfacing with the SNES"
#define FLAG_A_DESC =		"Flag Effect A [On or Off] Used for interfacing with the SNES"
#define FLAG_B_DESC =		"Flag Effect B [On or Off] Used for interfacing with the SNES"
#define FLAG_C_DESC =		"Flag Effect C [On or Off] Used for interfacing with the SNES"
#define FLAG_D_DESC =		"Flag Effect D [On or Off] Used for interfacing with the SNES"
#define FLAG_E_DESC =		"Flag Effect E [On or Off] Used for interfacing with the SNES"
#define FLAG_F_DESC =		"Flag Effect F [On or Off] Used for interfacing with the SNES"

#define PORT_UP_CTRL_DESC	"E1xy Portamento up [x: semitone, y: speed] Slides the note pitch up by X semitones at Y speed"
#define PORT_DOWN_CTRL_DESC	"E2xy Portamento down [x: semitone, y: speed] Slides the note pitch down by X semitones at Y speed"
#define GLOBAL_PAN_L_DESC	"E8xx Set Global Left Panning Volume [xx: l value] Sets the Global Left panning of the tune"
#define GLOBAL_PAN_R_DESC	"E9xx Set Global Right Panning Volume [xx: r value] Sets the Global Right panning of the tune"
#define GLOBAL_VOL_DESC		"EAxx Set Global Volume [xx: volume] Set's the Global Volume of the track"		

#define END_DESC			"FFxx End Tune [xx: end tune] Will end the tune no matter the value"

const int16_t SineTable[256] = {
	 0,     6,    13,    19,    25,    31,    37,
	44,    50,    56,    62,    68,    74,    80,
	86,    92,    98,   103,   109,   115,   120,
   126,   131,   136,   142,   147,   152,   157,
   162,   167,   171,   176,   180,   185,   189,
   193,   197,   201,   205,   208,   212,   215,
   219,   222,   225,   228,   231,   233,   236,
   238,   240,   242,   244,   246,   247,   249,
   250,   251,   252,   253,   254,   254,   255,
   255,   255,   255,   255,   254,   254,   253,
   252,   251,   250,   249,   247,   246,   244,
   242,   240,   238,   236,   233,   231,   228,
   225,   222,   219,   215,   212,   208,   205,
   201,   197,   193,   189,   185,   180,   176,
   171,   167,   162,   157,   152,   147,   142,
   136,   131,   126,   120,   115,   109,   103,
	98,    92,    86,    80,    74,    68,    62,
	56,    50,    44,    37,    31,    25,    19,
	13,     6,     0,    -6,   -13,   -19,   -25,
   -31,   -37,   -44,   -50,   -56,   -62,   -68,
   -74,   -80,   -86,   -92,   -98,  -103,  -109,
  -115,  -120,  -126,  -131,  -136,  -142,  -147,
  -152,  -157,  -162,  -167,  -171,  -176,  -180,
  -185,  -189,  -193,  -197,  -201,  -205,  -208,
  -212,  -215,  -219,  -222,  -225,  -228,  -231,
  -233,  -236,  -238,  -240,  -242,  -244,  -246,
  -247,  -249,  -250,  -251,  -252,  -253,  -254,
  -254,  -255,  -255,  -255,  -255,  -255,  -254,
  -254,  -253,  -252,  -251,  -250,  -249,  -247,
  -246,  -244,  -242,  -240,  -238,  -236,  -233,
  -231,  -228,  -225,  -222,  -219,  -215,  -212,
  -208,  -205,  -201,  -197,  -193,  -189,  -185,
  -180,  -176,  -171,  -167,  -162,  -157,  -152,
  -147,  -142,  -136,  -131,  -126,  -120,  -115,
  -109,  -103,   -98,   -92,   -86,   -80,   -74,
   -68,   -62,   -56,   -50,   -44,   -37,   -31,
   -25,   -19,   -13,    -6 
};

enum EffectFlags {
	arp_flag = 1,				//Flag for arpeggio
	port_flag = 2,				//Flag for portamento down
	vibrato_flag = 4,			//Flag for vibrato
	tremo_flag = 8,				//Flag for tremolando
	panbrello_flag = 16,		//Flag for panbrello
	volslide_flag = 32,			//Flag for volume sliding
	port_to_flag = 64,			//Flag of portamento to note
	port_ctrl_flag = 128,		//Flag for controlled portamento, shared between up and down
};

class EffectsHandler {
public:
	uint8_t Effect_Flags[8];	//Storing effect flags as a bit field, one for each channel: Currently this would store 16 possible effects at once
	uint16_t SPC_Flags[16];		//Stores the current value for the flag effects
	
	int16_t	Port_Value[8];		//Stores portamento value
	int16_t Vibrato_Value[8];	//Stores vibrato value
	int16_t Base_Pit[8];		//Stores base pitch
	int8_t Base_Vol_L[8];		//Stores base volume for L
	int8_t Base_Vol_R[8];		//Stores base volume for R
	uint8_t Sine_Index[8];		//Stores the index of the position in the sine table
	uint8_t Tremo_Value[8];		//Stores the value held in the tremolando effect
};