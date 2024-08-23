;-----------------------------------;
;   Macro's file for Cobalt Driver  ;
;-----------------------------------;

struct ZP $00
.TempMemADDRL:              skip 1                  ;General purpose addr LO
.TempMemADDRH:              skip 1                  ;General purpose addr HI
.TempScratchMem:            skip 1                  ;Temporary memory to screw with
.TempScratchMemH:           skip 1                  ;Temporary memory to screw with
.TickThresh:                skip 1                  ;Equivelant to track speed
.KONState:                  skip 1                  ;Holds the current bitfield state of KON
.FlagVal:                   skip 1                  ;Holds the flag value

;Special
.TempVolumeProcess:         skip 2                  ;Used to manipulate a volume value without actually changing said volume
.TempPitchProcess:          skip 2                  ;Holds the pitch

;Effects
.CurrentChannel:            skip 1                  ;Current channel we are working with

.ChannelArpValue:           skip 8                  ;Array of values for the Arpeggio
.ChannelPortValue:          skip 8                  ;Array of values for the Portamento
.ChannelVibratoValue:       skip 8                  ;Array of values for the Vibrato
.ChannelTremolandoValue:    skip 8                  ;Array of values for the Tremolando
.ChannelPanbrelloValue:     skip 8                  ;Array of values for the Panbrello
.ChannelVolSlideValue:      skip 8                  ;Array of values for the Volume slide

.PortDir:                   skip 1
.MulProductTemp:            skip 1
.ChannelPitches:            skip 16                 ;Array of pitch values in each channel
.ChannelPitchesOutput:      skip 16                 ;Array of pitches written to for every new note
.ChannelVolume:             skip 16                 ;Holds the channel master volume [goes across 16 bytes]
.ChannelVolumeOutput:       skip 16                 ;Array of pitches written to for every new note
.ChannelInstrumentIndex:    skip 8                  ;Array of Instrument indexes

.SequenceAddr:              skip 16                 ;Array of sequence address pointers
.OrderPos:                  skip 1                  ;Position we are within the orders table
.OrderPosGoto:              skip 1                  ;Aim position for reading in patterns
.OrderChangeFlag:           skip 1                  ;Flag for when we need to load in the next order sequence
.ChannelSleepCounter:       skip 8                  ;Array of sleep counters

;General Tracker State
.TrackSettings:             skip 1                  ;Holds the track settings [refer to DriverRequirements.txt]

endstruct
;Commands

    ;Row commands

struct RC $00
.SetSpeed       skip 1  ;Sets tick threshold for track
.Sleep          skip 1  ;Sleeps for S amount of rows
.Goto           skip 1  ;Break to new order
.Break          skip 1  ;Goto order
.PlayNote       skip 1  ;Plays note in note table
.PlayPitch      skip 1  ;Plays absolute pitch value
.SetInstrument  skip 1  ;
.SetFlagValue   skip 1  ;
.EchoDelay      skip 1  ;
.EchoVolume     skip 1  ;
.EchoFeedback   skip 1  ;
.EchoCoeff      skip 1  ;
.MasterVol      skip 1  ;
.ChannelVol     skip 1  ;
.SetArp         skip 1  ;
.SetPort        skip 1  ;
.SetVibrato     skip 1  ;
.SetTremo       skip 1  ;
.SetVolSlide    skip 1  ;
.SetPanbrello   skip 1  ;
.Stop           skip 1  ;
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
macro SetArpValue(V)
db RC.SetArp
db <V>
endmacro

macro SetPort(V, D)
db RC.SetPort
db <V>
db <D>
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

macro SetPabr(V)
db RC.SetPanbrello
db <V>
endmacro

macro Stop()
db RC.Stop
endmacro