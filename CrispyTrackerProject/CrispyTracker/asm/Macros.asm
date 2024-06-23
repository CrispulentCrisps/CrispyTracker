;
;   Macro's file for Cobalt Driver
;

namespace COM

TickAccum =                 $00         ;Accumulator for the tick amount
TickThresh =                $00         ;Equivelant to track speed
SequencePos =               $02         ;Position within the sequence stream
KONState =                  $04         ;Holds the current bitfield state of KON
FlagVal =                   $05         ;Holds the noise value
;Commands

    ;Row commands
EndRow =    $00
SetSpeed =  $01

    ;Note play

macro PlayNote(P, C)    ;Plays note in note table
db $10+<C>
db <P>
endmacro

macro PlayPitch(P, C)   ;Plays absolute pitch value
db $18+<C>
dw <P>
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

macro SetDelayCoefficient(V, C)   ;Set Echo delay volume
db $34+<C>
db <V>
endmacro
    ;Effects commands

namespace off