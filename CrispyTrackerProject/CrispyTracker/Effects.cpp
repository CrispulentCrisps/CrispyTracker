#include "Effects.h"

const extern  std::string EffectDesc[48] = {
	"00xy Arpeggio				[x: offset 1, y: offset 2]		Semitone offset from base note",
	"01xx Portamento up			[xx: speed up]					Slides the pitch upwards",
	"02xx Portamento down		[xx: speed down]				Slides the pitch downwards",
	"03xx Portamento to			[xx: speed toward]				Slides the pitch to the desired note",
	"04xy Vibrato				[x: speed, y: depth]			Oscillates the pitch of the note",
	"05xy Tremolando			[x: speed, y: depth]			Oscillates the volume of the note",

	"08xy Panning				[x: left, y: right]				Sets the panning of the channel",
	"09xx Speed					[xx: speed]						Sets the speed of the track",
	"0Axy Volume Slide			[x: up, y: down]				Changes the instrument volume",
	"0Bxx Goto					[xx: set order position]		Sets position within orders",
	"0Cxx Retrigger				[xx: frames between triggers]	Plays the note the amount of frames specified",
	"0Dxx Break													Goes to next order",

	"20xy Panbrello				[x: speed, y: depth]			Oscillates the panning of the note",

	"30xx Echo Delay			[xx: echo value]				Sets the delay value of the echo [Note! Every echo value consumes 2048 bytes of Audio Ram, this may cause issues within the ROM export and may not be able to fit within the SPC Export!]",
	"31xx Echo Feedback			[xx: feedback]					Sets the Feedback of the echo. A value of 80 is equivelant to a feedback of 0",
	"32xx Echo L Volume			[xx: left value]				Sets the volume of the Echo Left channel. A value of 80 is equivelant to a feedback of 0",
	"33xx Echo R Volume			[xx: right value]				Sets the volume of the Echo Right channel. A value of 80 is equivelant to a feedback of 0",
	"34xx Echo Filter Value 1	[xx: value]						Sets the value for the Echo Filter [Note! the whole filter buffer can only be from -128 to 127 TOTAL]",
	"35xx Echo Filter Value 2	[xx: value]						Sets the value for the Echo Filter [Note! the whole filter buffer can only be from -128 to 127 TOTAL]",
	"36xx Echo Filter Value 3	[xx: value]						Sets the value for the Echo Filter [Note! the whole filter buffer can only be from -128 to 127 TOTAL]",
	"37xx Echo Filter Value 4	[xx: value]						Sets the value for the Echo Filter [Note! the whole filter buffer can only be from -128 to 127 TOTAL]",
	"38xx Echo Filter Value 5	[xx: value]						Sets the value for the Echo Filter [Note! the whole filter buffer can only be from -128 to 127 TOTAL]",
	"39xx Echo Filter Value 6	[xx: value]						Sets the value for the Echo Filter [Note! the whole filter buffer can only be from -128 to 127 TOTAL]",
	"3Axx Echo Filter Value 7	[xx: value]						Sets the value for the Echo Filter [Note! the whole filter buffer can only be from -128 to 127 TOTAL]",
	"3Bxx Echo Filter Value 8	[xx: value]						Sets the value for the Echo Filter [Note! the whole filter buffer can only be from -128 to 127 TOTAL]",

	"0xC0 Flag Effect			[$00-$FF]						Used for interfacing with the 65C816",

	"E0xy Arpeggio speed		[x: semitone, y: speed]			Slides the note pitch up by X semitones at Y speed",
	"E1xy Portamento up			[x: semitone, y: speed]			Slides the note pitch up by X semitones at Y speed",
	"E2xy Portamento down		[x: semitone, y: speed]			Slides the note pitch down by X semitones at Y speed",
	"E8xx Set Track Volume		[xx: value]						Sets the Global Volume of the tune",

	"FFxx End Tune				[xx: end tune]					Stop tune playback",

	"??? Effect Unknown ???",
};