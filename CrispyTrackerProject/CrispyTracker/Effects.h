#pragma once

short Effect_Flags[8]; //Storing effect flags as a bit field, one for each channel
//Currently this would store 16 possible effects at once
//Should be more than enough

#define ARPEGGIO		0x00
#define PORT_UP			0x01
#define PORT_DOWN		0x02
#define PORT_TO			0x03
#define VIBRATO			0x04
#define TREMOLANDO		0x05

#define PANNING			0x08
#define SPEED			0x09
#define VOLSLIDE		0x0A
#define GOTO			0x0B
#define RETRIGGER		0x0C
#define BREAK			0x0D

#define ADSR_ENV		0x10
#define ADSR_TYP		0x11
#define ADSR_ATK		0x12
#define ADSR_DEC		0x13
#define ADSR_DC2		0x14
#define ADSR_SUS		0x15
#define ADSR_REL		0x16
#define GAIN			0x17
#define INVL			0x18
#define NOISESET		0x19
#define NOISEFREQ		0x1A
#define PTCHMOD			0x1B
#define ECHO			0x1C

#define PANBRLLO		0x20

#define ECHO_DEL		0x30
#define ECHO_FDB		0x31
#define ECHO_L			0x32
#define ECHO_R			0x33
#define ECHO_FIL1		0x34
#define ECHO_FIL2		0x35
#define ECHO_FIL3		0x36
#define ECHO_FIL4		0x37
#define ECHO_FIL5		0x38
#define ECHO_FIL6		0x39
#define ECHO_FIL7		0x3A
#define ECHO_FIL8		0x3B

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

#define PANL			0xE8//Global L
#define PANR			0xE9//Global R
#define GLOBAL_VOL		0xEA//Global Volume

#define END				0xFF

#define ARPEGGIO_DESC	"00xy Arpeggio [x: offset 1, y: offset 2] Semitone offset from base note"	
#define PORT_UP_DESC	"01xx Portamento up [xx: speed up] Slides the pitch upwards"	
#define PORT_DOWN_DESC	"02xx Portamento up [xx: speed down] Slides the pitch downwards"
#define PORT_TO_DESC	"03xx Portamento to [xx: speed toward] Slides the pitch to the desired note"	
#define VIBRATO_DESC	"04xy Vibrato [x: speed, y: depth] Oscillates the pitch of the note"
#define TREMOLANDO_DESC	"05xy Tremolando [x: speed, y: depth] Oscillates the volume of the note"

#define PANNING_DESC	"08xy Panning [x: left, y: right] Oscillates the pitch of the note"	
#define SPEED_DESC		"09xx Speed [xx: speed] Sets the speed of the track"
#define VOLSLIDE_DESC	"0Axy Volume Slide [x: up, y: down] Changes the instrument volume"
#define GOTO_DESC		"0Bxx Goto [xx: set order position] Sets position within orders"
#define RETRIGGER_DESC	"0Cxx Retrigger [xx: frames between triggers] Plays the note the amount of frames specified"
#define BREAK_DESC		"0Dxx Break. Goes to next order"
						
#define ADSR_ENV_DESC	"10xx Envelope Used [0 for off, 1 for on] Tells the instrument if it uses the ADSR envelope"
#define ADSR_TYP_DESC	"11xx Envelope Used [xx: type] Dictates which ADSR type to use"
#define ADSR_ATK_DESC	"12xx Set Attack [xx: value] Sets the attack value"
#define ADSR_DEC_DESC	"13xx Set Decay [xx: value] Sets the decay value"
#define ADSR_DC2_DESC	"14xx Set Decay 2 [xx: value] Sets the second decay value"
#define ADSR_SUS_DESC	"15xx Set Sustain [xx: value] Sets the sustain value"
#define ADSR_REL_DESC	"16xx Set Release [xx: value] Sets the release value"
#define GAIN_DESC		"17xx Set Gain [xx: value] Sets the gain value"
#define INVL_DESC		"18xy Invert Left [x: Inv L, y: Inv R,] Inverts audio on the specific channel"
#define NOISESET_DESC	"19xx Set Noise [0: off, 1: on] Sets audio generator to play"
#define NOISEFREQ_DESC	"1Axx Set Noise Frequency [xx: ] Sets the frequency for the noise generator"
#define PTCHMOD_DESC	"1Bxx Set Pitch Modulation [0: off, 1: on] Enables/Disables the Pitch Mod"
#define ECHO_DESC		"1Cxx Set Echo [0: off, 1: on] Enables/Disables the Echo"
						
#define PANBRLLO_DESC	"20xy Panbrello [x: speed, y: depth] Oscillates the panning of the note"

#define ECHO_DEL_DESC	"30xx Echo Delay [xx: echo value] sets the delay value of the echo [Note! Every echo value consumes 2048 bytes of Audio Ram, this may cause issues within the ROM export and may not be able to fit within the SPC Export!]"
#define ECHO_FDB_DESC	"31xx Echo Feedback [xx: feedback] Sets the Feedback of the echo. A value of 80 is equivelant to a feedback of 0"
#define ECHO_L_DESC		"32xx Echo L Volume [xx: left value] Sets the volume of the Echo Left channel. A value of 80 is equivelant to a feedback of 0"
#define ECHO_R_DESC		"33xx Echo R Volume [xx: right value] Sets the volume of the Echo Right channel. A value of 80 is equivelant to a feedback of 0"
#define ECHO_FIL1_DESC	"34xx Echo Filter Value 1 [xx: value] Sets the value for the Echo Filter [Note! the whole filter buffer can only be from -128 to 127 TOTAL]"
#define ECHO_FIL2_DESC	"35xx Echo Filter Value 2 [xx: value] Sets the value for the Echo Filter [Note! the whole filter buffer can only be from -128 to 127 TOTAL]"
#define ECHO_FIL3_DESC	"36xx Echo Filter Value 3 [xx: value] Sets the value for the Echo Filter [Note! the whole filter buffer can only be from -128 to 127 TOTAL]"
#define ECHO_FIL4_DESC	"37xx Echo Filter Value 4 [xx: value] Sets the value for the Echo Filter [Note! the whole filter buffer can only be from -128 to 127 TOTAL]"
#define ECHO_FIL5_DESC	"38xx Echo Filter Value 5 [xx: value] Sets the value for the Echo Filter [Note! the whole filter buffer can only be from -128 to 127 TOTAL]"
#define ECHO_FIL6_DESC	"39xx Echo Filter Value 6 [xx: value] Sets the value for the Echo Filter [Note! the whole filter buffer can only be from -128 to 127 TOTAL]"
#define ECHO_FIL7_DESC	"3Axx Echo Filter Value 7 [xx: value] Sets the value for the Echo Filter [Note! the whole filter buffer can only be from -128 to 127 TOTAL]"
#define ECHO_FIL8_DESC	"3Bxx Echo Filter Value 8 [xx: value] Sets the value for the Echo Filter [Note! the whole filter buffer can only be from -128 to 127 TOTAL]"

#define FLAG_0_DESC =	"Flag Effect 0 [On or Off] Used for interfacing with the SNES"
#define FLAG_1_DESC =	"Flag Effect 1 [On or Off] Used for interfacing with the SNES"
#define FLAG_2_DESC =	"Flag Effect 2 [On or Off] Used for interfacing with the SNES"
#define FLAG_3_DESC =	"Flag Effect 3 [On or Off] Used for interfacing with the SNES"
#define FLAG_4_DESC =	"Flag Effect 4 [On or Off] Used for interfacing with the SNES"
#define FLAG_5_DESC =	"Flag Effect 5 [On or Off] Used for interfacing with the SNES"
#define FLAG_6_DESC =	"Flag Effect 6 [On or Off] Used for interfacing with the SNES"
#define FLAG_7_DESC =	"Flag Effect 7 [On or Off] Used for interfacing with the SNES"
#define FLAG_8_DESC =	"Flag Effect 8 [On or Off] Used for interfacing with the SNES"
#define FLAG_9_DESC =	"Flag Effect 9 [On or Off] Used for interfacing with the SNES"
#define FLAG_A_DESC =	"Flag Effect A [On or Off] Used for interfacing with the SNES"
#define FLAG_B_DESC =	"Flag Effect B [On or Off] Used for interfacing with the SNES"
#define FLAG_C_DESC =	"Flag Effect C [On or Off] Used for interfacing with the SNES"
#define FLAG_D_DESC =	"Flag Effect D [On or Off] Used for interfacing with the SNES"
#define FLAG_E_DESC =	"Flag Effect E [On or Off] Used for interfacing with the SNES"
#define FLAG_F_DESC =	"Flag Effect F [On or Off] Used for interfacing with the SNES"

#define PANL_DESC		"E8xx Set Global Left Panning Volume [xx: l value] Sets the Global Left panning of the tune"
#define PANR_DESC		"E9xx Set Global Right Panning Volume [xx: r value] Sets the Global Right panning of the tune"
#define GLOBAL_VOL_DESC	"EAxx Set Global Volume [xx: volume] Set's the Global Volume of the track"		

#define END_DESC		"FFxx End Tune [xx: end tune] Will end the tune no matter the value"