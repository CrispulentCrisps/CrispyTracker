;
;   Macro's file for Cobalt Driver
;

namespace COM

TickAccum =     $00         ;Accumulator for the tick amount
TickThresh =    $00         ;Equivelant to track speed
SequencePos =   $02         ;Position within the sequence stream

;Commands

    ;Row commands
EndRow =    $00
SetSpeed =  $01

macro PlayNote(P, C)    ;Plays note in note table
db $10+<C>
db <P>
endmacro

macro PlayPitch(P, C)   ;Plays absolute pitch value
db $18+<C>
dw <P>
endmacro

    ;Effects commands

namespace off