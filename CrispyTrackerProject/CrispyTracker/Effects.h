#pragma once

short Effect_Flags[8]; //Storing effect flags as a bit field, one for each channel
//Currently this would store 16 possible effects at once
//Should be more than enough

#define ARPEGGIO	0x00
#define PORT_UP		0x01
#define PORT_DOWN	0x02
#define PORT_TO		0x03
#define VIBRATO		0x04
#define TREMOLANDO	0x05

#define PANNING		0x08
#define SPEED		0x09
#define VOLSLIDE	0x0A
#define GOTO		0x0B
#define RETRIGGER	0x0C
#define BREAK		0x0D

#define ADSR_ACT	0x10
#define ADSR_TYP	0x11
#define ADSR_ATK	0x12
#define ADSR_DEC	0x13
#define ADSR_DC2	0x14
#define ADSR_SUS	0x15
#define ADSR_REL	0x16
#define GAIN		0x17
#define INVL		0x18
#define INVR		0x19
#define NOISESET	0x1A
#define NFREQ		0x1B
#define PTCHMOD		0x1C
#define ECHO		0x1D

#define PANBRLLO	0x20

#define PANL		0xE8
#define PANR		0xE9

#define END			0xFF

#define ARPEGGIO_DESC	"0xy Arpeggio [x: offset 1, y: offset 2] Semitone offset from base note"	
#define PORT_UP_DESC	"1xx Portamento up [xx: speed up] Slides the pitch upwards"	
#define PORT_DOWN_DESC	"2xx Portamento up [xx: speed down] Slides the pitch downwards"
#define PORT_TO_DESC	"3xx Portamento to [xx: speed toward] Slides the pitch to the desired note"	
#define VIBRATO_DESC	"4xy Vibrato [x: speed, y: depth] Oscillates the pitch of the note"
#define TREMOLANDO_DESC	"5xy Vibrato [x: speed, y: depth] Oscillates the volume of the note"

#define PANNING_DESC	"8xy Panning [x: left, y: right] Oscillates the pitch of the note"	
#define SPEED_DESC		
#define VOLSLIDE_DESC	
#define GOTO_DESC		
#define RETRIGGER_DESC		
#define BREAK_DESC		
						
#define ADSR_ACT_DESC	
#define ADSR_TYP_DESC	
#define ADSR_ATK_DESC	
#define ADSR_DEC_DESC	
#define ADSR_DC2_DESC	
#define ADSR_SUS_DESC	
#define ADSR_REL_DESC	
#define GAIN_DESC		
#define INVL_DESC		
#define INVR_DESC		
#define NOISESET_DESC	
#define NFREQ_DESC		
#define PTCHMOD_DESC		
#define ECHO_DESC		
						
#define PANBRLLO_DESC	
					
#define PANL_DESC			
#define PANR_DESC			

#define END_DESC		