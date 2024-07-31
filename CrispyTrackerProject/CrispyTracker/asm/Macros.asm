;
;   Macro's file for Cobalt Driver
;

namespace COM
TempMemADDRL =              $00         ;General purpose addr LO
TempMemADDRH =              $01         ;General purpose addr HI
TickAccum =                 $02         ;Accumulator for the tick amount
TickThresh =                $03         ;Equivelant to track speed
SequencePos =               $04         ;Position within the sequence stream [Goes across 2 bytes]
KONState =                  $06         ;Holds the current bitfield state of KON
FlagVal =                   $07         ;Holds the flag value
ChannelVol =                $08         ;Holds the channel master volume [goes across 8 bytes]

;Special
NoiseState =                $18
EchoState =                 $19
PModState =                 $1A
TempORStore =               $1B
TempVolumeProcess =         $1C
TempPitchProcess =          $1D         ;Goes across 2 bytes

TempTriangleSpeed =         $1F
;Effects
TriangleCounter =           $20
TriangleState =             $28
TriangleSignHolder =        $30
EffectChannel =             $31     ;Current channel we are processing effects on

ChannelArpValue =           $32     ;Array of values for the Arpeggio
ChannelPortValue =          $3A     ;Array of values for the Portamento
ChannelVibratoValue =       $4A     ;Array of values for the Vibrato
ChannelTremolandoValue =    $52     ;Array of values for the Tremolando
ChannelPanbrelloValue =     $5A     ;Array of values for the Panbrello
ChannelVolSlideValue =      $62     ;Array of values for the Volume slide

PortDir =                   $6A
ChannelPitches =            $70     ;Array of pitch values in each channel
ChannelPitchesOutput =      $80     ;Array of pitches written to for every new note
ChannelVolumeOutput =       $90     ;Array of pitches written to for every new note
ChannelInstrumentIndex =    $A0     ;Array of Instrument indexes

;General Tracker State
TrackSpeed =                $E0     ;Holds the track speed for the track
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

    ;Effects commands
macro SetEffectBitfield(C, V)
db $50+<C>
db <V>
endmacro

macro SetArpValue(C, V)
db $58+<C>
db <V>
endmacro

macro SetPort(C, V, D)
db $60+<C>
db <V>
db <D>
endmacro

macro SetVib(C, V)
db $68+<C>
db <V>
endmacro

macro SetTremo(C, V)
db $70+<C>
db <V>
endmacro

macro SetVolSlide(C, V)
db $78+<C>
db <V>
endmacro
namespace off