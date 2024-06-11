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
#define PANBRELLO		0x0E

//Echo settings
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

//Global effects
#define GLOABL_PAN_L	0xE8//Global L
#define GLOABL_PAN_R	0xE9//Global R
#define GLOBAL_VOL		0xEA//Global Volume

#define END				0xFF//Ends the track

//Descriptions
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

#define FLAG_0_DESC 		"0xC0 Flag Effect 0 [On or Off] Used for interfacing with the SNES"
#define FLAG_1_DESC 		"0xC1 Flag Effect 1 [On or Off] Used for interfacing with the SNES"
#define FLAG_2_DESC 		"0xC2 Flag Effect 2 [On or Off] Used for interfacing with the SNES"
#define FLAG_3_DESC 		"0xC3 Flag Effect 3 [On or Off] Used for interfacing with the SNES"
#define FLAG_4_DESC 		"0xC4 Flag Effect 4 [On or Off] Used for interfacing with the SNES"
#define FLAG_5_DESC 		"0xC5 Flag Effect 5 [On or Off] Used for interfacing with the SNES"
#define FLAG_6_DESC 		"0xC6 Flag Effect 6 [On or Off] Used for interfacing with the SNES"
#define FLAG_7_DESC 		"0xC7 Flag Effect 7 [On or Off] Used for interfacing with the SNES"
#define FLAG_8_DESC 		"0xC8 Flag Effect 8 [On or Off] Used for interfacing with the SNES"
#define FLAG_9_DESC 		"0xC9 Flag Effect 9 [On or Off] Used for interfacing with the SNES"
#define FLAG_A_DESC 		"0xCA Flag Effect A [On or Off] Used for interfacing with the SNES"
#define FLAG_B_DESC 		"0xCB Flag Effect B [On or Off] Used for interfacing with the SNES"
#define FLAG_C_DESC 		"0xCC Flag Effect C [On or Off] Used for interfacing with the SNES"
#define FLAG_D_DESC 		"0xCD Flag Effect D [On or Off] Used for interfacing with the SNES"
#define FLAG_E_DESC 		"0xCE Flag Effect E [On or Off] Used for interfacing with the SNES"
#define FLAG_F_DESC 		"0xCF Flag Effect F [On or Off] Used for interfacing with the SNES"

#define ARP_SPEED_DESC		"E0xy Arpeggio speed [x: semitone, y: speed] Slides the note pitch up by X semitones at Y speed"
#define PORT_UP_CTRL_DESC	"E1xy Portamento up [x: semitone, y: speed] Slides the note pitch up by X semitones at Y speed"
#define PORT_DOWN_CTRL_DESC	"E2xy Portamento down [x: semitone, y: speed] Slides the note pitch down by X semitones at Y speed"
#define GLOBAL_PAN_L_DESC	"E8xx Set Global Left Panning Volume [xx: l value] Sets the Global Left panning of the tune"
#define GLOBAL_PAN_R_DESC	"E9xx Set Global Right Panning Volume [xx: r value] Sets the Global Right panning of the tune"
#define GLOBAL_VOL_DESC		"EAxx Set Global Volume [xx: volume] Set's the Global Volume of the track"		

#define END_DESC			"FFxx End Tune [xx: end tune] Will end the tune no matter the value"

const int8_t SineTable[256] = {
0x00,0x03,0x06,0x09,0x0C,0x10,0x13,0x16,0x19,0x1C,
0x1F,0x22,0x25,0x28,0x2B,0x2E,0x31,0x33,0x36,0x39,
0x3C,0x3F,0x41,0x44,0x47,0x49,0x4C,0x4E,0x51,0x53,
0x55,0x58,0x5A,0x5C,0x5E,0x60,0x62,0x64,0x66,0x68,
0x6A,0x6B,0x6D,0x6F,0x70,0x71,0x73,0x74,0x75,0x76,
0x78,0x79,0x7A,0x7A,0x7B,0x7C,0x7D,0x7D,0x7E,0x7E,
0x7E,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7E,0x7E,
0x7E,0x7D,0x7D,0x7C,0x7B,0x7A,0x7A,0x79,0x78,0x76,
0x75,0x74,0x73,0x71,0x70,0x6F,0x6D,0x6B,0x6A,0x68,
0x66,0x64,0x62,0x60,0x5E,0x5C,0x5A,0x58,0x55,0x53,
0x51,0x4E,0x4C,0x49,0x47,0x44,0x41,0x3F,0x3C,0x39,
0x36,0x33,0x31,0x2E,0x2B,0x28,0x25,0x22,0x1F,0x1C,
0x19,0x16,0x13,0x10,0x0C,0x09,0x06,0x03,0x00,0xFD,
0xFA,0xF7,0xF4,0xF0,0xED,0xEA,0xE7,0xE4,0xE1,0xDE,
0xDB,0xD8,0xD5,0xD2,0xCF,0xCD,0xCA,0xC7,0xC4,0xC1,
0xBF,0xBC,0xB9,0xB7,0xB4,0xB2,0xAF,0xAD,0xAB,0xA8,
0xA6,0xA4,0xA2,0xA0,0x9E,0x9C,0x9A,0x98,0x96,0x95,
0x93,0x91,0x90,0x8F,0x8D,0x8C,0x8B,0x8A,0x88,0x87,
0x86,0x86,0x85,0x84,0x83,0x83,0x82,0x82,0x82,0x81,
0x81,0x81,0x81,0x81,0x81,0x81,0x82,0x82,0x82,0x83,
0x83,0x84,0x85,0x86,0x86,0x87,0x88,0x8A,0x8B,0x8C,
0x8D,0x8F,0x90,0x91,0x93,0x95,0x96,0x98,0x9A,0x9C,
0x9E,0xA0,0xA2,0xA4,0xA6,0xA8,0xAB,0xAD,0xAF,0xB2,
0xB4,0xB7,0xB9,0xBC,0xBF,0xC1,0xC4,0xC7,0xCA,0xCD,
0xCF,0xD2,0xD5,0xD8,0xDB,0xDE,0xE1,0xE4,0xE7,0xEA,
0xED,0xF0,0xF4,0xF7,0xFA,0xFD
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
	
	uint8_t	Arp_Value[8];		//Stores Arpeggio value
	int16_t	Port_Value[8];		//Stores portamento value
	int16_t Vibrato_Value[8];	//Stores vibrato value
	uint8_t Tremo_Value[8];		//Stores the value held in the tremolando effect
	uint8_t Panbrello_Value[8];	//Stores the value held in the tremolando effect

	int16_t Base_Note[8];		//Stores base note
	int16_t Base_Pit[8];		//Stores base pitch
	int8_t Base_Vol_L[8];		//Stores base volume for L
	int8_t Base_Vol_R[8];		//Stores base volume for R
	
	uint8_t Sine_Index_Vib[8];	//Stores the index of the position in the sine table, used for vibrato
	uint8_t Sine_Index_Trem[8];	//Stores the index of the position in the sine table, used for tremolando
	uint8_t Sine_Index_PnBr[8];	//Stores the index of the position in the sine table, used for panbrello
	
	uint8_t ArpCounter[8];		//Stores the counter for arpeggio's
	uint8_t ArpState[8];		//Stores the currently held note in the arp

	uint8_t Arp_Control = 6;	//Controls how many ticks to wait between arpeggio notes.
};