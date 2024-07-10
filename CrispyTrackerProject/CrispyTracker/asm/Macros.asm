;
;   Macro's file for Cobalt Driver
;

namespace COM
TempMemADDRL =              $00         ;General purpose addr
TempMemADDRH =              $01         ;
TickAccum =                 $02         ;Accumulator for the tick amount
TickThresh =                $03         ;Equivelant to track speed
SequencePos =               $04         ;Position within the sequence stream [Goes across 2 bytes]
KONState =                  $06         ;Holds the current bitfield state of KON
FlagVal =                   $07         ;Holds the flag value
ChannelVol =                $08         ;Holds the channel master volume [goes across 8 bytes]

;Instruments
;ChInstrumentIndex =         $10         ;Holds the current instrument index in the channel [Goes across 8 bytes]

;Special
NoiseState =                $30
EchoState =                 $31
PModState =                 $32
TempORStore =               $38

;Effects
TriangleCounter =           $40
TriangleState =             $41
EffectChannel =             $42
ArpBitField =               $48
PortBitField =              $49
VibratoBitfield =           $4A
TremoBitField =             $4B
PanBrelloBitField =         $4C

;General Tracker State
TrackSpeed =                $80     ;Holds the track speed for the track
ChannelPitch =              $90     ;Hold the current channel's pitch in memory for effects processing

;Commands

    ;Row commands
EndRow =                    $00
SetSpeed =                  $01

    ;Note play
macro PlayNote(P, C)    ;Plays note in note table
db $10+<C>
db <P>
endmacro

macro PlayPitch(P, C)   ;Plays absolute pitch value
db $18+<C>
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

macro SetInstrument(C, I)
db $20+<C>
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

macro SetChannelVolume(C, V)   ;Set Channel Volume
db $41+<C>
db <V>
endmacro

macro MixChannel(C)

endmacro
    ;Effects commands

namespace off