;-----------------------------------;
;   Macro's file for Cobalt Driver  ;
;-----------------------------------;

struct ZP $00
.TempMemADDRL:              skip 1                  ;General purpose addr LO
.TempMemADDRH:              skip 1                  ;General purpose addr HI
.TempScratchMem:            skip 1                  ;Temporary memory to screw with
.TickThresh:                skip 1                  ;Equivelant to track speed
.KONState:                  skip 1                  ;Holds the current bitfield state of KON
.FlagVal:                   skip 1                  ;Holds the flag value
.ChannelVol:                skip 16                 ;Holds the channel master volume [goes across 16 bytes]

;Special
.NoiseState:                skip 1                  ;Bitfield for the NON  register
.EchoState:                 skip 1                  ;Bitfield for the EON  register
.PModState:                 skip 1                  ;Bitfield for the PMON register
.TempVolumeProcess:         skip 2                  ;Used to manipulate a volume value without actually changing said volume
.TempPitchProcess:          skip 2                  ;Holds the pitch

;Effects
.CurrentChannel:            skip 1                  ;Current channel we are working with
.SineTableDir:              skip 1                  ;Holds the state of the sine table inversion [YX inversion]

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
.ChannelVolumeOutput:       skip 16                 ;Array of pitches written to for every new note
.ChannelInstrumentIndex:    skip 8                  ;Array of Instrument indexes
.ChannelPatternIndex:       skip 8                  ;Array of Pattern indexes

.SequenceAddr:              skip 16                 ;Array of sequence address pointers
.OrderPos:                  skip 1                  ;Position we are within the orders table
.OrderChangeFlag:           skip 1                  ;Flag for when we need to load in the next order sequence
.ChannelSleepCounter:       skip 8                  ;Array of sleep counters

;General Tracker State
.TrackSettings:             skip 1                  ;Holds the track settings [refer to DriverRequirements.txt]

endstruct
;Commands

    ;Row commands
EndRow =                    $00

macro SetSpeed(S)       ;Sets tick threshold for track
db $01
db <S>
endmacro

macro Sleep(S)          ;Sleeps for S amount of rows
db $02
db <S>
endmacro

macro Goto(P)          ;Break to new order
db $03
db <P>
endmacro

macro Break()          ;Goto order P
db $04
endmacro

    ;Note play
macro PlayNote(P)       ;Plays note in note table
db $10
db <P>
endmacro

macro PlayPitch(P)      ;Plays absolute pitch value
db $11
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
db $20
db <I>
endmacro

    ;Special
macro SetNoise(V)       ;Set noise value in flag register
db $30
db <V>
endmacro

macro SetDelayTime(V)   ;Set Echo delay value
db $31
db <V>
endmacro

macro SetDelayVolume(L, R)   ;Set Echo delay volume
db $32
db <L>
db <R>
endmacro

macro SetDelayFeedback(V)   ;Set Echo delay volume
db $33
db <V>
endmacro

macro SetDelayCoefficient(C, V)   ;Set Echo delay coeffecients
db $34+<C>
db <V>
endmacro

    ;Volume
macro SetMasterVolume(L, R)   ;Set Master volume
db $40
db <L>
db <R>
endmacro

macro SetChannelVolume(L, R)   ;Set Channel Volume
db $41
db <L>
db <R>
endmacro

    ;Effects commands
macro SetArpValue(V)
db $50
db <V>
endmacro

macro SetPort(V, D)
db $51
db <V>
db <D>
endmacro

macro SetVib(V)
db $52
db <V>
endmacro

macro SetTremo(V)
db $53
db <V>
endmacro

macro SetVolSlide(V)
db $54
db <V>
endmacro

macro SetPabr(V)
db $55
db <V>
endmacro

macro Stop()
db $FF
endmacro