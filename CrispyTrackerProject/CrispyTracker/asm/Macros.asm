;-----------------------------------;
;   Macro's file for Cobalt Driver  ;
;-----------------------------------;

!CodeBuffer = $0A00                                 ;Section of bytes to fill

struct ZP $00
.TempMemADDRL:              skip 1                  ;General purpose addr LO
.TempMemADDRH:              skip 1                  ;General purpose addr HI
.TempScratchMem:            skip 1                  ;Temporary memory to screw with
.TempScratchMemH:           skip 1                  ;Temporary memory to screw with
.R0                         skip 1                  ;Scratch memory
.R1                         skip 1                  ;Scratch memory
.R2                         skip 1                  ;Scratch memory
.R3                         skip 1                  ;Scratch memory
.R4                         skip 1                  ;Scratch memory
.MulProductTemp:            skip 2                  ;Temporary memory for signed multiplication
.TickThresh:                skip 1                  ;Equivelant to track speed
.KONState:                  skip 1                  ;Holds the current bitfield state of KON
.FlagVal:                   skip 1                  ;Holds the flag value
.SFXRec                     skip 1                  ;Byte for APU1 value expected to play SFX

;Sequencing
.SequenceAddr:              skip 32                 ;Array of sequence address pointers

;Special
.TempVolumeProcess:         skip 2                  ;Used to manipulate a volume value without actually changing said volume
.TempPitchProcess:          skip 2                  ;Holds the pitch

;Effects
.CurrentChannel:            skip 1                  ;Current channel we are working with

.SineIndexVib:              skip 16                 ;Array of current indexes for sine tables
.SineIndexTrem:             skip 16                 ;Array of current indexes for sine tables
.SineIndexPanbr:            skip 16                 ;Array of current indexes for sine tables
.ArpValue:                  skip 16                 ;Array of values for the Arpeggio
.ArpTimer:                  skip 16                 ;Array of timers for the Arpeggio
.VolSlideValue:             skip 16                 ;Array of values for the Volume slide
.PortValue:                 skip 16                 ;Array of values for the Portamento
.VibratoValue:              skip 16                 ;Array of values for the Vibrato
.TremolandoValue:           skip 16                 ;Array of values for the Tremolando
.PanbrelloValue:            skip 16                 ;Array of values for the Panbrello

.ChannelPitchesOutput:      skip 2                  ;Pitch written to current channel
.ChannelVolumeOutput:       skip 2                  ;Volume written to current channel

;SFX Specific
.VCTick:                    skip 8                  ;Tick counters for the virtual channels
.VCTickThresh:              skip 8                  ;Tick thresholders for SFX
.VCOut:                     skip 4                  ;8 nibbles that determine which SFX is going through which channel. If a nibble is F we assume it's not outputting to any channel

;General Tracker State
.TrackSettings:             skip 1                  ;Holds the track settings [refer to DriverRequirements.txt]
.StopFlag                   skip 1                  ;Stops the track from progressing
endstruct

struct OP $0100
;Sequencing
.OrderPos:                  skip 9                  ;Position we are within the orders table
.OrderPosGoto:              skip 9                  ;Aim position for reading in patterns
.OrderChangeFlag:           skip 9                  ;Flag for when we need to load in the next order sequence
.ChannelSleepCounter:       skip 16                 ;Array of sleep counters
.ChannelInstrumentIndex:    skip 16                 ;Array of Instrument indexes
.ChannelPitches:            skip 32                 ;Array of pitch values in each channel
.ChannelVolume:             skip 32                 ;Holds the channel master volume [goes across 16 bytes]
endstruct

;Commands

    ;Row commands

struct RC $00
.SetSpeed       skip 1  ;Sets tick threshold for track  |   $00
.Sleep          skip 1  ;Sleeps for S amount of rows    |   $01   
.Goto           skip 1  ;Break to new order             |   $02
.Break          skip 1  ;Goto next order                |   $03
.PlayNote       skip 1  ;Plays note in note table       |   $04
.PlayPitch      skip 1  ;Plays absolute pitch value     |   $05
.SetInstrument  skip 1  ;Set instrument index           |   $06
.SetFlagValue   skip 1  ;Set FLG register               |   $07
.EchoDelay      skip 1  ;Set echo delay value           |   $08
.EchoVolume     skip 1  ;Set echo L/R volume            |   $09
.EchoFeedback   skip 1  ;Set echo feedback value        |   $0A
.EchoCoeff      skip 8  ;Set echo coeffecients          |   $0B-$12
.MasterVol      skip 1  ;Set Master L/R volumes         |   $13
.ChannelVol     skip 1  ;Set individual channel volume  |   $14
.SetArp         skip 1  ;Set Arpeggio effect value      |   $15
.SetPort        skip 1  ;Set Portamento effect value    |   $16
.SetVibrato     skip 1  ;Set Vibrato effect value       |   $17
.SetTremo       skip 1  ;Set Tremolando effect value    |   $18
.SetVolSlide    skip 1  ;Set Volume Slide effect value  |   $19
.SetPanbrello   skip 1  ;Set Panbrello effect value     |   $1A
.ReleaseNote    skip 1  ;Set KOFF for given channel     |   $1B
.Stop           skip 1  ;Set STOP flag for tune         |   $1C
endstruct

macro SetSpeed(S)       ;Sets tick threshold for track
db RC.SetSpeed
db <S>
endmacro

macro Sleep(S)          ;Sleeps for S amount of rows
db RC.Sleep
db <S>
endmacro

macro Goto(P)          ;Break to new order
db RC.Goto
db <P>
endmacro

macro Break()          ;Goto order P
db RC.Break
endmacro

    ;Note play
macro PlayNote(P)       ;Plays note in note table
db RC.PlayNote
db <P>
endmacro

macro PlayPitch(P)      ;Plays absolute pitch value
db RC.PlayPitch
dw <P>
endmacro

    ;Instrument
macro WriteInstrument(L, R, AD1, AD2, G, ES, SI, P)
db <L>      ;Left volume
db <R>      ;Right volume
db <AD1>    ;ADSR1
db <AD2>    ;ADSR2
db <G>      ;Gain
db <ES>     ;Effect state
db <SI>     ;Sample index
db <P>      ;Priority
endmacro

macro SetInstrument(I)
db RC.SetInstrument
db <I>
endmacro

    ;Special
macro SetFlag(V)       ;Set noise value in flag register
db RC.SetFlagValue
db <V>
endmacro

macro SetDelayTime(V)   ;Set Echo delay value
db RC.EchoDelay
db <V>
endmacro

macro SetDelayVolume(L, R)   ;Set Echo delay volume
db RC.EchoVolume
db <L>
db <R>
endmacro

macro SetDelayFeedback(V)   ;Set Echo delay volume
db RC.EchoFeedback
db <V>
endmacro

macro SetDelayCoefficient(C, V)   ;Set Echo delay coeffecients
db RC.EchoCoeff+<C>
db <V>
endmacro

    ;Volume
macro SetMasterVolume(L, R)   ;Set Master volume
db RC.MasterVol
db <L>
db <R>
endmacro

macro SetChannelVolume(L, R)   ;Set Channel Volume
db RC.ChannelVol
db <L>
db <R>
endmacro

    ;Effects commands
macro SetArp(V)
db RC.SetArp
db <V>
endmacro

macro SetPort(V)
db RC.SetPort
db <V>
endmacro

macro SetVib(V)
db RC.SetVibrato
db <V>
endmacro

macro SetTremo(V)
db RC.SetTremo
db <V>
endmacro

macro SetVolSlide(V)
db RC.SetVolSlide
db <V>
endmacro

macro SetPanbr(V)
db RC.SetPanbrello
db <V>
endmacro

macro Stop()
db RC.Stop
endmacro

macro VirtSleep(V)
db RC.VirtSleep
db <V>
endmacro

Apu0 =      $F4
Apu1 =      $F5
Apu2 =      $F6
Apu3 =      $F7