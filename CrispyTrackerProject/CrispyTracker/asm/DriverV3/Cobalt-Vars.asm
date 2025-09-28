incsrc "../spc700.asm"
incsrc "../dsp.asm"

!TRACKER =    $01

!CodeBuffer = $0C00                 ;Section of bytes to fill

!ChannelCount = 8

struct ZP $0000
.Ptr0           skip 2                  ;General purpose pointer
.Ptr1           skip 2                  ;General purpose pointer
.Ptr2           skip 2                  ;General purpose pointer
.Ptr3           skip 2                  ;General purpose pointer
.R0             skip 1                  ;Scratch memory
.R1             skip 1                  ;Scratch memory
.R2             skip 1                  ;Scratch memory
.R3             skip 1                  ;Scratch memory
.R4             skip 1                  ;Scratch memory
.R5             skip 1                  ;Scratch memory
.R6             skip 1                  ;Scratch memory
.R7             skip 1                  ;Scratch memory

.ChSeq          skip 2*!ChannelCount    ;Pointer for current channel data
.ChOrder        skip 2*!ChannelCount    ;Pointer for current channel position in order table
.ChVol          skip 2*!ChannelCount    ;Volume of current channel
.ChPitch        skip 2*!ChannelCount    ;Pitch of current channel
.ChInst         skip !ChannelCount      ;Current channel instrument
.ChTimer        skip !ChannelCount      ;Current channel timer
.ChVibVal       skip !ChannelCount      ;Channel vibrato value [xy, x - depth, y - speed]
.ChVibPhase     skip !ChannelCount      ;Channel vibrato sine phase
.ChPortVal      skip !ChannelCount      ;Channel portamento
.ChVolSlideVal  skip !ChannelCount      ;Channel volume slide [wait 0x frames before volume change]
.ChVolSlideCnt  skip !ChannelCount      ;Timer to wait N frames before the volume slide

.ChIndex        skip 1                  ;Current channel index
.KON            skip 1                  ;KON state
.KOFF           skip 1                  ;KOFF state
.EON            skip 1                  ;Echo status
.NON            skip 1                  ;Noise status
.PMON           skip 1                  ;Pitch modulation status
.MusSpeed1      skip 1                  ;Delay time for music track
.MusSpeed2      skip 1                  ;Delay time for music track
.MusSpeedSel    skip 1                  ;Which speed to use for music
.MusTimer       skip 1                  ;Timer for frames to wait before next row

.PauseFlag      skip 1                  ;Flag to pause music

.OutPitch       skip 2                  ;Output pitch in effects routine
.OutVol         skip 2                  ;Output volume in effects routine

endstruct

assert sizeof(ZP) < $F0

InstTable =     $01EC                   ;Instrument table pointer
TuneStart =     $00F8                   ;Pointer to start of tune in memory

;
;   Header information for a given tune
;       Will be placed at the start of a given file, intended to be loaded before the start of order memory
;
struct Header $FFF0
.EchoDelay      skip 1                  ;Echo delay value
.EchoFeedback   skip 1                  ;Echo feecback
.EchoVol        skip 2                  ;Echo volume
.EchoCoeff      skip 8                  ;Echo coeffecients
.InitSpeed      skip 2                  ;Initial track speed
endstruct

;Music commands
!COM_SLEEP  =       $00     ;Sets a channel to wait N timer passes
!COM_JUMP  =        $01     ;Jumps a channel's order position to a new position
!COM_BREAK =        $02     ;Advance order by 1 position
!COM_INST =         $03     ;Sets the instrument values for a given channel
!COM_PITCH =        $04     ;Plays an absolute pitch
!COM_VOLUME =       $05     ;Sets the volume of a given channel
!COM_SPEED1 =       $06     ;Sets the frame wait for every sleep decrement
!COM_SPEED2 =       $07     ;Sets the frame wait for every sleep decrement
!COM_VIBRATO =      $08     ;Sets the vibrato for a given channel
!COM_PORTAMENTO =   $09     ;Sets the portamento for a given channel
!COM_VOLSLIDE =     $0A     ;Sets the volume slide for a given channel

!COM_NOTE =         $20     ;Any value $20 and above is interpreted as a note based off of note tables

!PRoCom_NULL =      $00     ;Null value, any value that's 0 is ignored in the communication routine
!PRoCom_LOAD =      $01     ;Sets the IPL loader up
!PRoCom_PLAYSFX =   $02     ;Play SFX out of a specific channel
!PRoCom_FADETUNE =  $03     ;Play SFX out of a specific channel
!PRoCom_SETMASTER = $04     ;Sets the master volume for the     

macro WriteCom(Com)
    db <Com>
endmacro

macro WriteComByte(Com, Val)
    db <Com>
    db <Val>
endmacro

macro WriteComWord(Com, Val)
    db <Com>
    dw <Val>
endmacro

;
;   Flags: ---- -PNE
;                ||Echo
;                |Noise
;                Pitch mod
;
!InstSize = $05
macro WriteInstrument(Srcn, ADSR1, ADSR2, GAIN, Flags)
    db <Srcn>
    db <ADSR1>
    db <ADSR2>
    db <GAIN>
    db <Flags>
endmacro

macro ReadSeqVal()
    mov.b A, ZP.ChIndex
    asl A
    mov.b X, A
    mov.b A, (ZP.ChSeq+X)
    inc.b ZP.ChSeq+X
    bne +
    inc.b ZP.ChSeq+1+X
    +
endmacro