;-----------------------------------;
;   Macro's file for Cobalt Driver  ;
;-----------------------------------;

!CodeBuffer = $0C00                 ;Section of bytes to fill

struct ZP $00
.TempMemADDRL:              skip 1  ;General purpose addr LO
.TempMemADDRH:              skip 1  ;General purpose addr HI
.TempScratchMem:            skip 1  ;Temporary memory to screw with
.TempScratchMemH:           skip 1  ;Temporary memory to screw with
.R0                         skip 1  ;Scratch memory
.R1                         skip 1  ;Scratch memory
.R2                         skip 1  ;Scratch memory
.R3                         skip 1  ;Scratch memory
.R4                         skip 1  ;Scratch memory
.MulProductTemp:            skip 2  ;Temporary memory for signed multiplication
.TickThresh:                skip 1  ;Equivelant to track speed
.KONState:                  skip 1  ;Holds the current bitfield state of KON
.SFXRec                     skip 1  ;Byte for APU1 value expected to play SFX
.FlagVal                    skip 1  ;Communication between SPC700 and 65C816
.ChannelMask                skip 1  ;Bitfield to mask off channels

;Sequencing
.SequenceAddr:              skip 32 ;Array of sequence address pointers

;Special
.TempVolumeProcess:         skip 2  ;Used to manipulate a volume value without actually changing said volume
.TempPitchProcess:          skip 2  ;Holds the pitch
.CurrentChannel:            skip 1  ;Current channel we are working with
.InjectionChannel:          skip 1  ;Current channel we're injecting into

;Effects
.SineIndexVib:              skip 16 ;Array of current indexes for sine tables
.SineIndexTrem:             skip 16 ;Array of current indexes for sine tables
.SineIndexPanbr:            skip 16 ;Array of current indexes for sine tables
.ArpValue:                  skip 16 ;Array of values for the Arpeggio
.ArpTimer:                  skip 16 ;Array of timers for the Arpeggio
.VolSlideValue:             skip 16 ;Array of values for the Volume slide
.PortValue:                 skip 16 ;Array of values for the Portamento
.VibratoValue:              skip 16 ;Array of values for the Vibrato
.TremolandoValue:           skip 16 ;Array of values for the Tremolando
.PanbrelloValue:            skip 16 ;Array of values for the Panbrello
.ChannelPitchesOutput:      skip 2  ;Pitch written to current channel
.ChannelVolumeOutput:       skip 2  ;Volume written to current channel

;SFX Specific
.VCTickThresh:              skip 8  ;Tick thresholders for SFX
.VCOut:                     skip 8  ;8 bytes that determine which SFX is going through which channel. [0 = OFF, 1 = ON]

;General Tracker State
.FadeFlag                   skip 1  ;Fades out master volume
.FadeSpeed                  skip 1  ;Speed fade is applied
.TrackSettings:             skip 1  ;Holds the track settings [refer to DriverRequirements.txt]
.MasterVol                  skip 1  ;Master volume for track
.EchoVol                    skip 1  ;Master echo for track
.OutVol                     skip 1  ;Masters the MVOL and EVOL registers
endstruct

struct OP $0100
;Sequencing
.OrderPos:                  skip 9  ;Position we are within the orders table
.OrderPosGoto:              skip 9  ;Aim position for reading in patterns
.OrderChangeFlag:           skip 9  ;Flag for when we need to load in the next order sequence
.ChannelSleepCounter:       skip 16 ;Array of sleep counters
.ChannelInstrumentIndex:    skip 16 ;Array of Instrument indexes
.ChannelPitches:            skip 32 ;Array of pitch values in each channel
.ChannelVolume:             skip 32 ;Holds the channel master volume [goes across 16 bytes]
.StopFlag:                  skip 9  ;Flag to stop track from progressing
.MaxVolTarget               skip 1  ;Maximum volume to aim for when fading in
.RegPitchWrite              skip 32 ;Pitches to write to pitch registers
endstruct

InstPtr =                   $01E0
OrderPtr =                  $01E2
SfxListPtr =                $01E4
SfxPatPtr =                 $01E6
SubPtr =                    $01E8
PitchPtr =                  $01EA

;Commands
    ;Row commands
struct RC $FF00
.SetSpeed                   skip 1  ;Sets tick threshold for track  |   $00     
.Sleep                      skip 1  ;Sleeps for S amount of rows    |   $01     
.Goto                       skip 1  ;Break to new order             |   $02     
.Break                      skip 1  ;Goto next order                |   $03     
.PlayPitch                  skip 1  ;Plays absolute pitch value     |   $04     
.SetInstrument              skip 1  ;Set instrument index           |   $05     
.SetFlagValue               skip 1  ;Set FLG register               |   $06     
.EchoDelay                  skip 1  ;Set echo delay value           |   $07     
.EchoVolume                 skip 1  ;Set echo L/R volume            |   $08     
.EchoFeedback               skip 1  ;Set echo feedback value        |   $09     
.EchoCoeff                  skip 8  ;Set echo coeffecients          |   $0A-$11      
.ChannelVol                 skip 1  ;Set individual channel volume  |   $12     
.SetArp                     skip 1  ;Set Arpeggio effect value      |   $13     
.SetPort                    skip 1  ;Set Portamento effect value    |   $14     
.SetVibrato                 skip 1  ;Set Vibrato effect value       |   $15     
.SetTremo                   skip 1  ;Set Tremolando effect value    |   $16     
.SetVolSlide                skip 1  ;Set Volume Slide effect value  |   $17     
.SetPanbrello               skip 1  ;Set Panbrello effect value     |   $18     
.ReleaseNote                skip 1  ;Set KOFF for given channel     |   $19
.Stop                       skip 1  ;Set STOP flag for tune         |   $1A
.MasterVol                  skip 1  ;Set the master volume left     |   $1B
.NanBuff                    skip 4  ;Buffer for future commands     |   $1C-1F
.PlayNote                   skip 1  ;Play pitch from table          |   $20-FF
endstruct

;   Programmer Commands
struct ProCom $FE00
.PlayMusic                  skip 1  ;Play music track               |   $00
.PlaySfx                    skip 1  ;Play sound effect              |   $01
.SetMasterVol               skip 1  ;Set OutVol byte                |   $02
.SetSettings                skip 1  ;Set settings byte              |   $03
.SetDriverDiv               skip 1  ;Set timer divider              |   $04
.MuteChannel                skip 1  ;Mutes certain channels         |   $05
.Pause                      skip 1  ;Flips STOP flag for tune       |   $06
.FadeAudio                  skip 1  ;Enables fade routine           |   $07
.FadeMax                    skip 1  ;Set maximum volume to fade to  |   $08
.FadeSpeed                  skip 1  ;Set fade speed                 |   $09
.ResetAPU                   skip 1  ;Go to IPL rom and load SPC     |   $0A
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
db RC.PlayNote+<P>
endmacro

macro PlayPitch(P)      ;Plays absolute pitch value
db RC.PlayPitch
dw <P>
endmacro

macro ReleaseNote()      ;Sets release in ADSR
db RC.ReleaseNote
endmacro

    ;Instrument
    ;
    ;   Written to the CPU in order:
    ;       SCRN
    ;       ADSR1
    ;       ADSR2
    ;       GAIN
    ;       EFX state [PMON, NON, EON]
    ;       KON
    ;
macro WriteInstrument(AD1, AD2, G, ES, SI)
db <AD1>    ;ADSR1
db <AD2>    ;ADSR2
db <G>      ;Gain
db <ES>     ;Effect state
db <SI>     ;Sample index
endmacro

!InstWidth =    $05

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

macro SetDelayVolume(V)   ;Set Echo delay volume
db RC.EchoVolume
db <V>
endmacro

macro SetDelayFeedback(V)   ;Set Echo feedback
db RC.EchoFeedback
db <V>
endmacro

macro SetDelayCoefficient(C, V)   ;Set Echo delay coeffecients
db RC.EchoCoeff+<C>
db <V>
endmacro

    ;Volume
macro SetChannelVolume(L, R)   ;Set Channel Volume
db RC.ChannelVol
db <L>
db <R>
endmacro

macro SetMasterVolume(V)   ;Set Channel Volume
db RC.MasterVol
db <V>
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

Apu0 =      $F4
Apu1 =      $F5
Apu2 =      $F6
Apu3 =      $F7