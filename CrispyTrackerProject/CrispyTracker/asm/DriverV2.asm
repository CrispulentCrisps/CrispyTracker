;|==============================|
;|                              |
;|  Cobalt Audio Diver Program  |
;|                              |
;|          Written by          |
;|            Crisps            |
;|                              |
;|          08/06/2024          |
;|                              |
;|==============================|

incsrc "Macros.asm"
incsrc "spc700.asm"
incsrc "dsp.asm"

arch spc700     ;Set architecture to SPC-700

org $C10000     ;Go to bank C1000

ROM_Engine_Start:

base $0200              ;Set audio driver code to $0200 [closest to the start of memory we can get away with]

DriverStart:            ;Start of the driver

mov ZP.TempMemADDRL, #Engine_End&$FF
mov ZP.TempMemADDRH, #Engine_End>>8
mov X, #0
mov A, #0
.MemClearLoop:
db $C7, $00                     ;Equivelant to mov (ZP.TempMemADDRL+X), A; reason is the ASAR doesn't put this line in code, instead putting in C5 00 00
incw ZP.TempMemADDRL
bne .MemClearLoop

mov X, #$FF
mov SP, X
setp
mov SP, X
clrp
mov X, #$00
mov Y, #$00
mov A, #$00
-
mov $0100+Y, A
dec Y
bne -

mov ZP.TrackSettings, #$01     ;Set the track settings

;Audio register reset routine
mov A, #0
-
mov Y, #$00                         ;Shove into reset value
mov X, #$0F
mov SPC_RegADDR, A                  ;Shove address in
Inner:
mov SPC_RegData, Y                  ;Reset volume left
inc SPC_RegADDR                     ;Increment address value
dec X                               ;Decrement the loop counter
bpl Inner
clrc                                ;Clear carry
adc A, #$10                         ;Add 16
bvc -

mov SPC_Control, #$01               ;Set control bit to enable Timer 0
mov SPC_Timer1, #$20                ;Divide timer to run at ~250hz
setp
mov A, #$01
mov Y, #$08
-
mov OP.OrderChangeFlag+Y, A         ;Set change flag at start to load initial pattern data for both SFX and music
dec Y
bpl -
clrp
mov ZP.VCOut,   #$FF                ;Reset the VCOut state to F to prevent channel injection
mov ZP.VCOut+1, #$FF                ;Reset the VCOut state to F to prevent channel injection
mov ZP.VCOut+2, #$FF                ;Reset the VCOut state to F to prevent channel injection
mov ZP.VCOut+3, #$FF                ;Reset the VCOut state to F to prevent channel injection

mov Apu0, #$00
mov Apu1, #$00
mov Apu2, #$00
mov Apu3, #$00
mov ZP.SFXRec, #$01

%spc_write(DSP_FLG, $00)
%spc_write(DSP_MVOL_L, $7F)
%spc_write(DSP_MVOL_R, $7F)
%spc_write(DSP_EVOL_L, $00)
%spc_write(DSP_EVOL_R, $00)
%spc_write(DSP_ESA, $C0)
%spc_write(DSP_EDL, $00)
%spc_write(DSP_EFB, $20)
%spc_write(DSP_DIR, $0C)
%spc_write(DSP_PMON, $00)
%spc_write(DSP_EON, $00)
%spc_write(DSP_NON, $00)

mov A, #(PitchTable)&$FF
mov PitchPtr, A
mov A, #(PitchTable>>8)&$FF
mov PitchPtr+1, A

mov A, #(InstrumentMemory)&$FF
mov InstPtr, A
mov A, #(InstrumentMemory>>8)&$FF
mov InstPtr+1, A

mov A, #OrderTable&$FF
mov OrderPtr, A
mov A, #(OrderTable>>8)&$FF
mov OrderPtr+1, A

mov A, #SfxTable&$FF
mov SfxPatPtr, A
mov A, #(SfxTable>>8)&$FF
mov SfxPatPtr+1, A

mov A, #SFXList&$FF
mov SfxListPtr, A
mov A, #(SFXList>>8)&$FF
mov SfxListPtr+1, A

mov A, #SubtuneList&$FF
mov SubPtr, A
mov A, #(SubtuneList>>8)&$FF
mov SubPtr+1, A

DriverLoop:                         ;Main driver loop
    mov X, #0                       ;Reset counter
    
    .TickIncrement:
    mov Y, SPC_Count1               ;Check counter
    beq .TickIncrement              ;If the timer is set to 0
    
    call RecieveSub
    call ProcessEffects

    mov A, ZP.StopFlag              ;Stop track from progressing
    bne .TickIncrement

    inc X                           ;Increment our counter
    cmp X, ZP.TickThresh            ;Check if the counter has reached the 
    bmi .TickIncrement              ;Go back to the tick incrementer if the counter is not
                                    ;Assuming we've hit the threshold
    setp
    cmp OP.OrderChangeFlag, #1      ;Check the order flag with the 0th bit
    bne +                           ;Skip if the carry flag isn't set
    mov X, #$07
    mov Y, #$00
    -                                   ;Sleep Clear Loop
    mov.b OP.ChannelSleepCounter+X, Y   ;Clear sleep counter
    dec X                               ;Decrement counter
    bpl -
    clrp
    mov A, OrderPtr
    mov ZP.R0, A
    mov A, OrderPtr+1
    mov ZP.R1, A
    setp
    mov Y, OP.OrderPos
    clrp
    mov ZP.R2, Y
    call ReadPatterns
    setp
    mov OP.OrderChangeFlag, #$00
    +
    clrp
    
    mov ZP.CurrentChannel,#$7       ;Increment channel index
    .ChannelLoop:                   ;Main channel loop
    mov X, ZP.CurrentChannel
    setp
    mov.b Y, OP.ChannelSleepCounter+X
    clrp
    bne .SkipRow                    ;Check if the sleep counter != 0
    call ReadRows
    bra .SkipDec
    .SkipRow:                       ;Sleep counter routine
    setp
    dec OP.ChannelSleepCounter+X
    clrp
    .SkipDec:
    dec ZP.CurrentChannel
    bpl .ChannelLoop
    setp
    cmp OP.OrderChangeFlag, #1      ;Order change
    bne +
    mov A, OP.OrderPosGoto
    mov OP.OrderPos, A
    +
    clrp
    call HandleSFX                  ;Handle SFX interpreting
    jmp DriverLoop


ProcessEffects:
    push X
    mov A, ZP.UpdateSwitch
    inc A
    and A, #$01
    mov ZP.UpdateSwitch, A

    mov ZP.CurrentChannel, #$07
    mov A, ZP.UpdateSwitch
    bne +
    mov ZP.CurrentChannel, #$0F
    +
    .EffectsLoop:
    mov ZP.TempPitchProcess,    #0
    mov ZP.TempPitchProcess+1,  #0
    mov ZP.TempVolumeProcess,   #0
    mov ZP.TempVolumeProcess+1, #0
    mov A, ZP.CurrentChannel        ;Grab channel index
    asl A                           ;Double since we are working with 2 bytes
    mov ZP.TempScratchMemH, A       ;Return

    ;-------------------;
    ;   Volume Slide    ;
    ;-------------------;
    mov A, ZP.CurrentChannel
    asl A
    mov X, A
    mov A, ZP.VolSlideValue+X
    mov ZP.R0, A
    and A, #$0F
    mov ZP.R1, A
    beq .Up
    ;Dec volume
    setp
    mov A, OP.ChannelVolume+X
    clrp
    setc
    sbc A, ZP.R1
    setp
    mov OP.ChannelVolume+X, A
    clrp
    inc X
    setp
    mov A, OP.ChannelVolume+X
    clrp
    setc
    sbc A, ZP.R1
    setp
    mov OP.ChannelVolume+X, A
    clrp
    bra .SkipVSlide
    .Up:
    ;Inc volume
    mov A, ZP.R0
    xcn
    mov ZP.R1, A
    setp
    mov A, OP.ChannelVolume+X
    clrc
    clrp
    adc A, ZP.R1
    setp
    mov OP.ChannelVolume+X, A
    clrp
    inc X
    setp
    mov A, OP.ChannelVolume+X
    clrp
    clrc
    adc A, ZP.R1
    setp
    mov OP.ChannelVolume+X, A
    clrp
    .SkipVSlide:

    ;---------------;
    ;   Portamento  ;
    ;---------------;
    mov X, ZP.CurrentChannel
    mov A, ZP.PortValue+X
    mov ZP.TempMemADDRL, A
    mov ZP.TempMemADDRH, #0
    mov1 C, ZP.TempMemADDRL.7
    bcc +
    mov ZP.TempMemADDRH, #$FF
    +
    mov X, ZP.TempScratchMemH
    setp
    mov.b Y, OP.ChannelPitches+1+X
    mov A, OP.ChannelPitches+X
    clrp
    addw YA, ZP.TempMemADDRL
    setp
    mov.b OP.ChannelPitches+1+X, Y
    mov OP.ChannelPitches+X, A  
    clrp
    
    ;--------------------;
    ;       Vibrato      ;
    ;--------------------;
    mov X, ZP.CurrentChannel
    mov A, ZP.VibratoValue+X
    and A, #$F0
    xcn A
    asl A
    mov ZP.TempScratchMem, A    ;Hold speed in temporary memory
    mov A, ZP.SineIndexVib+X    ;Grab sine index
    adc A, ZP.TempScratchMem    ;Increment sine index by speed value
    mov ZP.SineIndexVib+X, A
    mov Y, A                    ;Grab sine index value
    mov A, ZP.VibratoValue+X
    and A, #$0F                 ;Grab depth
    call GetSineValue           ;Call sine function
    call SignedMul              ;multiply
    mov ZP.R0, A
    mov ZP.R1, Y
    mov A, ZP.TempPitchProcess
    mov Y, ZP.TempPitchProcess+1
    addw YA, ZP.R0
    movw ZP.TempPitchProcess, YA
    
    ;----------------;
    ;   Tremolando   ;
    ;----------------;
    mov X, ZP.CurrentChannel
    mov A, ZP.TremolandoValue+X
    and A, #$F0
    xcn A
    mov ZP.TempScratchMem, A        ;Hold speed in temporary memory
    mov A, ZP.SineIndexTrem+X       ;Grab sine index
    adc A, ZP.TempScratchMem        ;Increment sine index by speed value
    mov ZP.SineIndexTrem+X, A
    mov Y, A                        ;Grab sine index value
    mov A, ZP.TremolandoValue+X
    and A, #$0F                     ;Grab depth
    call GetSineValue               ;Call sine function
    call SignedMul                  ;Multiply sine influence
    mov ZP.R0, A                    ;Shove into temp memory
    mov ZP.R1, Y
    lsr ZP.R1                       ;Div by 2
    ror ZP.R0                       ;Rotate by 2
    lsr ZP.R1                       ;Div by 2
    ror ZP.R0                       ;Rotate by 2
    lsr ZP.R1                       ;Div by 2
    ror ZP.R0                       ;Rotate by 2
    lsr ZP.R1                       ;Div by 2
    ror ZP.R0                       ;Rotate by 2
    mov A, ZP.TempVolumeProcess     ;Grab current volume process
    clrc
    adc A, ZP.R0
    mov ZP.TempVolumeProcess, A     ;Apply
    mov A, ZP.TempVolumeProcess+1   ;Grab current volume process
    clrc
    adc A, ZP.R0
    mov ZP.TempVolumeProcess+1, A   ;Apply

    ;----------------;
    ;    Panbrello   ;
    ;----------------;
    mov X, ZP.CurrentChannel
    mov A, ZP.PanbrelloValue+X
    and A, #$F0
    xcn A
    mov ZP.TempScratchMem, A        ;Hold speed in temporary memory
    mov A, ZP.SineIndexPanbr+X      ;Grab sine index
    adc A, ZP.TempScratchMem        ;Increment sine index by speed value
    mov ZP.SineIndexPanbr+X, A
    mov Y, A                        ;Grab sine index value
    mov A, ZP.PanbrelloValue+X
    and A, #$0F                     ;Grab depth
    call GetSineValue               ;Call sine function
    call SignedMul                  ;Multiply sine influence
    mov ZP.R0, A                    ;Shove into temp memory
    mov ZP.R1, Y
    lsr ZP.R1                       ;Div by 2
    ror ZP.R0                       ;Rotate by 2
    lsr ZP.R1                       ;Div by 2
    ror ZP.R0                       ;Rotate by 2
    lsr ZP.R1                       ;Div by 2
    ror ZP.R0                       ;Rotate by 2
    lsr ZP.R1                       ;Div by 2
    ror ZP.R0                       ;Rotate by 2
    mov A, ZP.TempVolumeProcess     ;Grab current volume process
    clrc
    adc A, ZP.R0
    mov ZP.TempVolumeProcess, A     ;Apply
    mov A, ZP.TempVolumeProcess+1   ;Grab current volume process
    setc
    sbc A, ZP.R0
    mov ZP.TempVolumeProcess+1, A   ;Apply

    ;Check SFX output to inject into hardware output
    mov Y, #$00
    .SFXLoop:
    mov ZP.R0, #$18
    -
    mov A, ZP.VCOut+Y
    and A, #$08
    beq .InjectSFX
    dec ZP.R0
    dec ZP.R0
    bne -
    inc Y

    ;Prevent doing channel mixing if working Virtual channels
    mov A, ZP.CurrentChannel
    and A, #$08
    beq +
    jmp .SkipInst
    +

    bra .EndInject
    ;A SFX has been triggered and the output will be injected into the 
    .InjectSFX:
    mov A, X
    clrc
    adc A, #$08
    mov X, A    
    .EndInject
    ;---------------------------;
    ;       Mixing & Output     ;
    ;---------------------------;
    mov X, ZP.TempScratchMemH
    setp
    mov.b A, OP.ChannelVolume+X
    clrp
    mov ZP.ChannelVolumeOutput, A                     ;Shove into volume output
    
    mov X, ZP.TempScratchMemH
    setp
    mov.b A, OP.ChannelVolume+1+X
    clrp
    mov ZP.ChannelVolumeOutput+1, A                     ;Shove into volume output

    ;Add Volume offset from effects
    ;L
    mov A, ZP.ChannelVolumeOutput
    adc A, ZP.TempVolumeProcess
    bvc .SkipLCorr                                      ;Overflow detection
    bmi .NegativeL                                      ;Check negative
    mov A, #$80                                         ;Clamp output
    bra .SkipLCorr
    .NegativeL:
    mov A, #$7F                                         ;Clamp output
    .SkipLCorr:
    mov ZP.ChannelVolumeOutput, A
    ;R
    mov A, ZP.ChannelVolumeOutput+1
    adc A, ZP.TempVolumeProcess+1
    bvc .SkipRCorr                                      ;Overflow detection
    bmi .NegativeR                                      ;Check negative
    mov A, #$80                                         ;Clamp output
    bra .SkipRCorr
    .NegativeR:
    mov A, #$7F                                         ;Clamp output
    .SkipRCorr:
    mov ZP.ChannelVolumeOutput+1, A

    ;Force output to mono
    mov C, ZP.TrackSettings.0
    bcc .SkipMono
    clr1 ZP.ChannelVolumeOutput.7
    clr1 ZP.ChannelVolumeOutput+1.7
    .SkipMono:

    ;Apply Volume
    mov X, ZP.TempScratchMemH                           ;Grab Premult channel index
    mov Y, ZP.ChannelVolumeOutput                       ;Grab L output volume
    mov A, ZP.CurrentChannel                            ;Grab current channel
    xcn A                                               ;Swap nibbles to get correct channel addr
    mov SPC_RegADDR, A                                  ;Shove channel addr
    mov SPC_RegData, Y                                  ;Shove data in
    inc SPC_RegADDR                                     ;Inc address to get R volume
    mov Y, ZP.ChannelVolumeOutput+1                     ;Grab R output volume
    mov SPC_RegData, Y                                  ;Shove data in
    
    ;Channel pitch application
    mov A, ZP.CurrentChannel                            ;Grab current channel
    xcn A                                               ;XCN for correct register addr
    mov ZP.TempMemADDRL, A                              ;Store multiplied index
    or A, #$02                                          ; + 2
    mov SPC_RegADDR, A                                  ;Shove into addr
    
    mov X, ZP.TempScratchMemH                           ;Grab Premult channel index
    setp
    mov A, OP.ChannelPitches+X                          ;Grab current channel's lo pitch
    clrp
    adc A, ZP.TempPitchProcess                          ;Add lo byte to pitch offset
    mov ZP.ChannelPitchesOutput, A                      ;Put into output
    setp
    mov A, OP.ChannelPitches+1+X                        ;Grab current channel's hi pitch
    clrp
    adc A, ZP.TempPitchProcess+1                        ;Add hi byte to pitch offset
    setp
    mov ZP.ChannelPitchesOutput+1, A                    ;Put into output

    mov A, ZP.ChannelPitchesOutput                      ;Grab lo pitch
    mov SPC_RegData, A                                  ;Return
    inc SPC_RegADDR                                     ;Increment address
    mov A, ZP.ChannelPitchesOutput+1                    ;Grab hi pitch
    mov SPC_RegData, A                                  ;Return
    clrp
    ;Jump over if working Virtual channels
    .SkipInst:
    
    mov A, ZP.UpdateSwitch
    bne .MusicFX
    ;SFX loop
    dec ZP.CurrentChannel
    mov A, ZP.CurrentChannel
    cmp A, #$07
    beq .BreakLoop
    jmp .EffectsLoop
    ;Music loop
    .MusicFX:
    dec ZP.CurrentChannel
    bmi .BreakLoop
    jmp .EffectsLoop
    .BreakLoop:
    pop X
    ret

    ;
    ;   Clobberlist:
    ;       ZP.R0       Aim table lo
    ;       ZP.R1       Aim table hi
    ;       ZP.R2       Order position index
    ;       ZP.R3       Hardware/Virtual channel flag
    ;
ReadPatterns:
    mov ZP.R3, #$00
    mov X, ZP.R2
    mov A, X
    and A, #$08
    beq +
    mov ZP.R3, #$01
    mov A, ZP.R2
    and A, #$07
    setp
    mov A, OP.OrderPos+1+X                     ;Grab the current order position [sfx]
    bra .OrderRead
    +
    setp
    mov A, OP.OrderPos                         ;Grab the current order position [music]
    .OrderRead:
    clrp
    xcn A                                      ;Mult by 16
    mov ZP.TempMemADDRH, A                     ;Shove into hi zp
    mov ZP.TempMemADDRL, A                     ;Shove into lo zp
    and ZP.TempMemADDRH, #$0F                  ;Get lo nibble
    and ZP.TempMemADDRL, #$F0                  ;Get hi nibble
    mov A, ZP.R0                               ;Shove lo table addr into A
    mov Y, ZP.R1                               ;Shove hi table addr into Y
    addw YA, ZP.TempMemADDRL                   ;Add offset to YA
    movw ZP.TempMemADDRL, YA                   ;Return address to memory
    ;Fill address pointers for music and SFX
    mov A, ZP.R3
    bne .SFXAddr
    mov Y, #$0F                                ;Set Y up for loop
    -                                          ;Loop point
    mov A, (ZP.TempMemADDRL)+Y                 ;Indirectly shove addr value into A
    mov ZP.SequenceAddr+Y, A                   ;Copy value to sequence addr 
    dec Y                                      ;Decrement loop counter
    bpl -                                      ;Loop
    ret

    .SFXAddr:
    mov Y, #$0F                                ;Set Y up for loop
    -                                          ;Loop point
    mov A, (ZP.TempMemADDRL)+Y                 ;Indirectly shove addr value into A
    mov ZP.SequenceAddr+$10+Y, A               ;Copy value to sequence addr
    dec Y                                      ;Decrement loop counter
    bpl -                                      ;Loop
    ret

HandleSFX:
    ;Handle timers
    mov ZP.CurrentChannel, #$0F
    .SfxLoop:
    mov X, ZP.CurrentChannel
    ;Check if our tick counter has reached 0
    setp
    mov A, OP.ChannelSleepCounter+X
    bne .DecTick
    ;Read pattern if the counter is currently 0
    clrp
    mov A, ZP.CurrentChannel
    and A, #$07
    mov X, A
    setp
    mov A, OP.OrderChangeFlag+1+X
    beq .SkipPatternRead
    ;Erase flag
    mov A, #$00
    mov OP.OrderChangeFlag+1+X, A
    ;Setup pattern read
    clrp
    mov A, ZP.CurrentChannel
    asl A
    mov X, A
    mov A, SfxPatPtr
    mov Y, SfxPatPtr+1
    mov ZP.R0, A
    mov ZP.R1, Y
    setp
    mov A, OP.OrderPos-7+X
    clrp
    mov X, ZP.CurrentChannel
    mov ZP.R2, X
    call ReadPatterns
    mov A, ZP.VCTickThresh-8+X
    mov A, OP.ChannelSleepCounter+X
    .SkipPatternRead:
    ;If tick timer == 0 then we decrement our sleep counter
    mov X, ZP.CurrentChannel
    setp
    mov A, OP.ChannelSleepCounter+X
    bne .ApplySleep
    clrp    
    call ReadRows
    .ApplySleep:
    mov OP.ChannelSleepCounter+X, A
    .SkipSleep:
    .DecTick:
    dec OP.ChannelSleepCounter+X
    clrp
    dec ZP.CurrentChannel
    mov X, ZP.CurrentChannel
    cmp X, #$07
    bne .SfxLoop
    mov ZP.CurrentChannel, #$00
    ret

ReadRows:
    call GrabCommand
    mov ZP.R0, A
    setc
    sbc A, #RC.PlayNote&$FF
    bcc +
    jmp (Row_PlayNote)
    +
    mov A, ZP.R0
    asl A                               ;Mult by 2 to prevent reading in the wrong address byte
    mov X, A                            ;Shove into X for the jump table
    jmp (.RowJumpTable+X)               ;Goto jumptable + command index

.RowJumpTable:
    dw Row_SetSpeed
    dw Row_Sleep
    dw Row_Goto
    dw Row_Break
    dw Row_PlayPitch
    dw Row_SetInstrument
    dw Row_SetFlag
    dw Row_SetDelay
    dw Row_SetDelayVolume
    dw Row_SetDelayFeedback
    dw Row_SetDelayCoeff
    dw Row_SetDelayCoeff
    dw Row_SetDelayCoeff
    dw Row_SetDelayCoeff
    dw Row_SetDelayCoeff
    dw Row_SetDelayCoeff
    dw Row_SetDelayCoeff
    dw Row_SetDelayCoeff
    dw Row_SetChannelVolume
    dw Row_SetArp
    dw Row_SetPort
    dw Row_SetVib
    dw Row_SetTrem
    dw Row_SetVolSlide
    dw Row_SetPanbr
    dw Row_NoteRelease
    dw Row_Stop
    dw Row_PlayNote
    
Row_SetSpeed:
    clrp
    mov A, ZP.CurrentChannel
    and A, #$08
    bne +
    ;Music
    call GrabCommand
    mov ZP.TickThresh, A
    bra ReadRows
    +
    mov A, ZP.CurrentChannel
    and A, #$07
    mov Y, A
    call GrabCommand
    mov ZP.VCTickThresh+Y, A
    bra ReadRows

Row_Sleep:
    call GrabCommand
    mov X, ZP.CurrentChannel
    setp
    mov OP.ChannelSleepCounter+X, A
    clrp
    ret                             ;ret is exiting out of the read rows subroutine

Row_Goto:
    call GrabCommand
    call GetGotoInd
    setp
    mov OP.OrderPosGoto, A          ;Return pos
    mov A, #$01
    mov OP.OrderChangeFlag+X, A     ;Set the order change flag
    clrp
    ret

Row_Break:
    setp
    mov OP.OrderPosGoto, OP.OrderPos
    inc OP.OrderPosGoto
    mov A, #$01
    call GetGotoInd
    mov OP.OrderChangeFlag+X, A         ;Set the order change flag
    clrp
    ret                                 ;Break out of ReadRows

Row_PlayNote:
    mov A, ZP.R0                    ;Grab pitch index
    setc
    sbc A, #(RC.PlayNote)&$FF       ;Offset index by PlayNote
    asl A
    mov Y, A
    setp
    mov A, (PitchPtr)+Y             ;Grab lo pitch in table
    mov OP.ChannelPitches+X, A
    inc Y
    mov A, (PitchPtr)+Y          ;Grab hi pitch in table
    mov OP.ChannelPitches+1+X, A
    clrp
    bra Row_PlayPitch_SetKOn

Row_PlayPitch:
    ;Pitch application
    call GrabCommand                ;Grab lo byte of the pitch
    mov OP.ChannelPitches+X, A      ;Shove lo byte into pitch
    call GrabCommand                ;Grab hi byte of the pitch
    mov OP.ChannelPitches+1+X, A    ;Shove hi byte into pitch

.SetKOn:
    mov A, ZP.CurrentChannel        ;Check if handling SFX or Music
    and A, #$08
    bne .GotoSFX
    ;Music
    ;KON State
    mov X, ZP.CurrentChannel            ;Grab current channel
    mov A, BitmaskTable+X               ;Get bitmask index via X
    mov ZP.TempScratchMem, A            ;Shove into scratch memory for ORing
    mov SPC_RegADDR, #DSP_KON           ;Shove KON addr in
    or SPC_RegData, ZP.TempScratchMem   ;OR into A
    ;Mask in lo nibble in VCOUT
    mov ZP.R0, #$F0
    and A, #$01
    beq .MSXHi
    ;Mask in hi nibble in VCOUT
    mov ZP.R0, #$0F
    .MSXHi:
    mov A, ZP.CurrentChannel        ;Check if handling SFX or Music
    lsr A
    mov X, A
    mov A, ZP.VCOut+X
    or A, ZP.R0                     ;Mask in corresponding nibble
    mov ZP.VCOut+X, A
    jmp ReadRows
    .GotoSFX:
    ;SFX
    ;Mask out lo nibble in VCOUT
    mov ZP.R0, #$F0
    and A, #$01
    beq .SFXHi
    ;Mask out hi nibble in VCOUT
    mov ZP.R0, #$0F
    .SFXHi:
    mov A, ZP.CurrentChannel        ;Check if handling SFX or Music
    and A, #$07
    lsr A
    mov X, A
    mov A, ZP.VCOut+X
    and A, ZP.R0                    ;Mask out corresponding nibble
    mov ZP.VCOut+X, A
    jmp ReadRows

Row_SetInstrument:
    mov A, InstPtr
    mov Y, InstPtr+1
    clrp
    mov ZP.TempMemADDRL, A
    mov ZP.TempMemADDRH, Y
    call GrabCommand
    mov X, ZP.CurrentChannel
    setp
    mov OP.ChannelInstrumentIndex+X, A
    clrp
    mov Y, #!InstWidth
    mul YA
    addw YA, ZP.TempMemADDRL
    movw ZP.TempMemADDRL, YA

    mov A, ZP.CurrentChannel
    and A, #$08
    beq +
    jmp ReadRows
    +

    ;SCRN
    mov A, ZP.CurrentChannel            ;Grab current channel
    xcn A                               ;XCN to get correct channel in memory
    or A, #$04                          ;Add 4 for correct nibble
    mov SPC_RegADDR, A                  ;SCRN
    mov X, #0                           ;Reset X
    mov A, (ZP.TempMemADDRL+X)          ;Shove value into A
    mov SPC_RegData, A                  ;Apply
    
    ;ADSR1
    incw ZP.TempMemADDRL                ;Increment
    mov A, (ZP.TempMemADDRL+X)          ;Grab Value
    inc SPC_RegADDR                     ;Inc address
    mov SPC_RegData, A                  ;Apply
    
    ;ADSR2
    incw ZP.TempMemADDRL                ;Increment
    mov A, (ZP.TempMemADDRL+X)          ;Grab Value
    inc SPC_RegADDR                     ;Inc address
    mov SPC_RegData, A                  ;Apply
    
    ;GAIN
    incw ZP.TempMemADDRL                ;Increment
    mov A, (ZP.TempMemADDRL+X)          ;Grab Value
    inc SPC_RegADDR                     ;Inc address
    mov SPC_RegData, A                  ;Apply

    ;PMON, EON, NON, Channel ON
    incw ZP.TempMemADDRL                ;Increment
    mov X, ZP.CurrentChannel            ;Grab current channel
    mov A, BitmaskTable+X               ;Grab bitfield
    eor A, #$FF                         ;Invert current bit
    mov ZP.TempScratchMem, A            ;Shove into temp memory
    mov SPC_RegADDR, #DSP_PMON          ;Shove PMON into regaddr
    mov X, #$00                         ;Reset X
    mov ZP.TempScratchMemH, #$01        ;Shove 1 into temp memory
    mov Y, #$03                         ;Initialise loop counter

    .RestartLoop:
    and SPC_RegData, ZP.TempScratchMem  ;AND the current bitfield with
    mov A, (ZP.TempMemADDRL+X)          ;Grab effects state
    and A, ZP.TempScratchMemH           ;AND effects state and current comparison bit
    beq .SkipAppl                       
    eor ZP.TempScratchMem, #$FF         ;Undo inversion in scratch memory
    or SPC_RegData, ZP.TempScratchMem   ;Combine the cleared bit with the comparison bit
    eor ZP.TempScratchMem, #$FF         ;Reinvert the channel bit
    .SkipAppl:
    asl ZP.TempScratchMemH              ;LShift comparison bit
    adc SPC_RegADDR, #$10               ;Add on $10 to the address to get to other bitfields
    dec Y                               ;Dec loop counter
    bne .RestartLoop
    jmp ReadRows

Row_SetFlag:
    call GrabCommand                    ;Grab value
    mov SPC_RegADDR, #DSP_FLG           ;Get correct addr
    mov SPC_RegData, A                  ;Apply
    jmp ReadRows

Row_SetDelay:
    call GrabCommand                ;Grab delay value
    mov SPC_RegADDR, #DSP_EDL
    mov SPC_RegData, A
    jmp ReadRows

Row_SetDelayVolume:
    call GrabCommand                ;Grab delay value
    mov SPC_RegADDR, #DSP_EVOL_L
    mov SPC_RegData, A
    call GrabCommand                ;Grab delay value
    mov SPC_RegADDR, #DSP_EVOL_R
    mov SPC_RegData, A
    jmp ReadRows

Row_SetDelayFeedback:
    call GrabCommand                ;Grab delay value
    mov SPC_RegADDR, #DSP_EFB
    mov SPC_RegData, A
    jmp ReadRows

Row_SetDelayCoeff:
    mov A, X
    lsr A                       ;Divide A by 2 since it's premultiplied for the jump table
    setc
    sbc A, #(RC.EchoCoeff)&$FF  ;Grab current coeffecient
    xcn
    clrc
    adc A, #$0F
    mov SPC_RegADDR, A
    call GrabCommand                ;Grab coeff value
    mov SPC_RegData, A              ;Apply
    jmp ReadRows

Row_SetChannelVolume:
    call GrabCommand                ;Grab L volume
    setp
    mov OP.ChannelVolume+X, A       ;Apply
    clrp
    call GrabCommand                ;Grab R volume
    setp
    mov OP.ChannelVolume+1+X, A     ;Apply
    clrp
    jmp ReadRows

Row_SetArp:
    call GrabCommand
    mov X, ZP.CurrentChannel
    mov ZP.ArpValue+X, A
    jmp ReadRows

Row_SetPort:
    call GrabCommand
    mov X, ZP.CurrentChannel
    mov ZP.PortValue+X, A
    jmp ReadRows

Row_SetVib:
    call GrabCommand
    mov X, ZP.CurrentChannel
    mov ZP.VibratoValue+X, A
    jmp ReadRows

Row_SetTrem:
    call GrabCommand
    mov X, ZP.CurrentChannel
    mov ZP.TremolandoValue+X, A
    jmp ReadRows

Row_SetVolSlide:
    call GrabCommand
    mov X, ZP.CurrentChannel
    mov ZP.VolSlideValue+X, A
    jmp ReadRows

Row_SetPanbr:
    call GrabCommand
    mov X, ZP.CurrentChannel
    mov ZP.PanbrelloValue+X, A
    jmp ReadRows

Row_NoteRelease:
    ;KOF State
    mov X, ZP.CurrentChannel            ;Grab current channel
    mov A, BitmaskTable+X               ;Get bitmask index via X
    mov ZP.TempScratchMem, A            ;Shove into scratch memory for ORing
    mov SPC_RegADDR, #DSP_KOF           ;Shove KOF addr in
    or SPC_RegData, ZP.TempScratchMem   ;OR into A
    ret

Row_Stop:
    mov ZP.StopFlag, #$01
    ret

        ;----------------------------;
        ;       Command Grabber      ;
        ;----------------------------;
        ;
        ;   Input:
        ;       ZP.CurrentChannel
        ;
        ;   Output:
        ;       A: Next byte from command list
        ;       X: Current channel * 2
        ;       
        ;   Clobber list:
        ;       A: for the command value
        ;       X: command indexing
        ;
GrabCommand:
    mov A, ZP.CurrentChannel            ;Grab current channel
    asl A                               ;Mult by 2 to get the correct offset into the sequence pointers
    mov X, A
    mov A, (ZP.SequenceAddr+X)          ;Grab command from the current channel's command stream
    inc ZP.SequenceAddr+X               ;Increment the channel's lo sequence pointer
    bne +                               ;Check for 0 if we've overflown
    inc ZP.SequenceAddr+1+X             ;Increment hi byte
    +
    ret


        ;---------------------------;
        ;       Get GOTO Index      ;
        ;---------------------------;
        ;
        ;   Input:
        ;       ZP.CurrentChannel
        ;
        ;   Output:
        ;       X: Goto Index 
        ;           if (ZP.CurrentChannel < 8) return 0
        ;           else return (ZP.CurrentChannel&0x08)+1
        ;       
        ;   Clobber list:
        ;       X: command indexing
        ;
        ;
GetGotoInd:
    push P
    clrp
    push A                          ;Store away A value if need be
    mov X, #$00                     ;Assume music track flag index
    mov A, ZP.CurrentChannel
    and A, #$08
    beq +
    mov A, ZP.CurrentChannel        ;Grab current channel
    and A, #$07                     ;Get channel equivelant
    inc A                           ;Increment since first flag is for the music track
    mov X, A                        ;Move into X
    +
    pop A
    pop P
    ret

        ;----------------------------;
        ;       Sine Generator       ;
        ;----------------------------;
        ;
        ;   Input:
        ;       Y [As sine table index, 0:255]
        ;
        ;   Output:
        ;       Y [As the sine value, -128:127]
        ;
        ;   Clobber list:
        ;       Y
        ;       TempScratchMem
        ;       TempMemADDRL
        ;
GetSineValue:
    push A
    mov ZP.TempScratchMem, Y        ;Shove the index into scratch mem
    mov A, Y                        ;Shove index into A for modulo
    and A, #$3F                     ;Remove last 2 bits to clamp to 0-63
    mov ZP.TempMemADDRL, A          ;Shove value into temporary memory
    mov1 C, ZP.TempScratchMem.6     ;Shove 6th bit into carry to determine the X flip
    bcc +                           ;Jump over if (> 64 & < 128) | (> 192 & < 256)
    mov A, #$3F                     ;Shove subtraction into A
    setc
    sbc A, ZP.TempMemADDRL          ;Subtract
    +
    mov Y, A
    mov A, SineTable+Y              ;Add on the sine table value
    mov1 C, ZP.TempScratchMem.7     ;Shove 7th bit into carry to determine the Y flip
    bcc +                           ;Jump over if < 128
    eor A, #$FF                     ;Invert values
    inc A
    +
    mov Y, A                        ;Return value to Y
    pop A                           ;Grab original A value
    ret

    ;-----------------------;
    ; Signed Multiplication ;
    ;-----------------------;
    ;
    ;   Input:
    ;       Y [-128, 127]
    ;       A [-128, 127]
    ;
    ;   Output:
    ;       YA [As signed value]
    ;
    ;   Clobber list:
    ;       Y
    ;       A
    ;       X
    ;       MulProductTemp
    ;
    ;        [Provided by AArt1256]
    ;
SignedMul:
    ;Save L and H temp memory as we use it outside of this function
    mov X, ZP.TempMemADDRL
    push X
    mov X, ZP.TempMemADDRH
    push X
    
    mov ZP.TempMemADDRL, A
    mov ZP.TempMemADDRH, Y
    mul YA

    ; save product into temporary memory
    mov ZP.MulProductTemp, A
    mov ZP.MulProductTemp+1, Y

    ; check for MSB
    mov A, ZP.TempMemADDRL
    bpl .skip_mul1
    setc
    mov A, ZP.MulProductTemp+1
    sbc A, ZP.TempMemADDRH
    mov ZP.MulProductTemp+1, A
    .skip_mul1:

    ; check for MSB
    mov A, ZP.TempMemADDRH
    bpl .skip_mul2
    setc
    mov A, ZP.MulProductTemp+1
    sbc A, ZP.TempMemADDRL
    mov ZP.MulProductTemp+1, A
    .skip_mul2:

    mov A, ZP.MulProductTemp
    mov Y, ZP.MulProductTemp+1

    ;Restore L and H temp memory
    pop X
    mov ZP.TempMemADDRH, X
    pop X
    mov ZP.TempMemADDRL, X
    ret

    ;------------------------------;
    ;   Recieve Subtunes from CPU  ;
    ;------------------------------;
    ;
    ;   Input:
    ;       Apu0    Subtune index to play
    ;       Apu2    
    ;
    ;   Output:
    ;       Parse into Subtune table if Apu0 != 0
    ;
    ;   Clobberlist
    ;       APU01
    ;       ZP.R0   \
    ;       ZP.R1    |  Addr pointers
    ;       ZP.R2    |
    ;       ZP.R3   /
    ;
    ;                                   TODO:   Fix up code here to work for both subtunes _and_ SFX
RecieveSub:
    push A
    push X
    push Y
    mov A, Apu1                         ;Check SEND byte
    cmp A, ZP.SFXRec
    bne .SkipSFXCheck                   ;if SFXRec != SFXRec before then new a new subtune will be selected
    inc ZP.FlagVal
    mov X, #$00                         ;Setup music ptr
    mov A, SubPtr
    mov Y, SubPtr+1
    mov ZP.R0, A
    mov ZP.R1, Y
    mov A, Apu2                         ;if APU2 != 0 then we just assume it's playing a SFX, otherwise we assume
    beq +
    mov A, SfxListPtr
    mov Y, SfxListPtr+1
    mov ZP.R0, A             ;Setup SFX ptr
    mov ZP.R1, Y
    mov X, #$00                         ;Set X to 0 for some pointer shenanigans
    mov A, (ZP.R0+X)
    mov ZP.R2, A
    incw ZP.R0
    mov A, (ZP.R0+X)                    ;Write pointer to subtune pointer list
    mov ZP.R3, A
    mov ZP.R0, ZP.R2                    ;Return pointer address to ZP.R0 so we can write the correct SFX indices
    mov ZP.R1, ZP.R3
    mov X, #$10                         ;Offset address by $10 bytes
    +

    ;Store away SFX address to the correct sequence addr
    mov ZP.R2, #$0F
    mov Y, #$00
    .AddrLoop:
    mov A, (ZP.R0)+Y
    mov ZP.SequenceAddr+X, A
    inc X
    incw ZP.R0
    mov A, (ZP.R0)+Y
    mov ZP.SequenceAddr+X, A
    inc X
    incw ZP.R0
    dec ZP.R2
    bpl .AddrLoop

    ;Clear sleep timers
    mov A, X
    setc
    lsr A
    sbc A, #$08
    mov X, A
    mov Y, #$07
    setp
    -
    mov A, #$00
    mov OP.ChannelSleepCounter+X, A
    mov A, #$01
    mov OP.OrderChangeFlag+1+Y, A
    dec X
    dec Y
    bpl -
    clrp
    ;Increment recieve flag
    inc ZP.SFXRec
    mov Apu1, ZP.SFXRec
    ;Skip SFX if Apu0 == 0
    .SkipSFXCheck:
    pop Y
    pop X
    pop A
    ret

CheckProgrammerControls:
    push A
    push X
    push Y

    pop Y
    pop X
    pop A
    ret

BitmaskTable:   ;General bitmask table
    db $01
    db $02
    db $04
    db $08
    db $10
    db $20
    db $40
    db $80

CoeffecientTable:   ;Writes the value for the coeffecient index
    db DSP_C0
    db DSP_C1
    db DSP_C2
    db DSP_C3
    db DSP_C4
    db DSP_C5
    db DSP_C6
    db DSP_C7

;Sine table, this includes the FIRST QUARTER of the sine wave, from 0 to 127
;It is intended that the sine gets flipped both horizontally and vertically
;The vertical can be done by changing the sign bit within the table
SineTable:
db $00,$03,$06,$09,$0C,$10,$13,$16,$19,$1C
db $1F,$22,$25,$28,$2B,$2E,$31,$33,$36,$39
db $3C,$3F,$41,$44,$47,$49,$4C,$4E,$51,$53
db $55,$58,$5A,$5C,$5E,$60,$62,$64,$66,$68
db $6A,$6B,$6D,$6F,$70,$71,$73,$74,$75,$76
db $78,$79,$7A,$7A,$7B,$7C,$7D,$7D,$7E,$7E
db $7E,$7F,$7F,$7F

fill !CodeBuffer-pc()
assert pc() == !CodeBuffer

DirTable:                          ;These are just for debugging purposes
;Test sine+saw sample + dir page
db $08,$0C,$08,$0C,$23,$0C,$23,$0C

SampleTable:                        ;These are just for debugging purposes
db $84, $17, $45, $35, $22, $22, $31, $21, $10, $68, $01, $21, $0D, $01, $08, $0B, $C3, $3E, $5B, $09, $8B, $D7
db $B1, $E0, $BC, $AF, $78
db $B8, $87, $1F, $00, $F1, $0F, $1F, $00, $00, $8F, $E1, $13, $12, $2D, $52, $14, $10, $F7

PitchTable:
    for t = 0..224
        dw $0000+(!t*$0100)
    endfor

OrderTable:
    .Tune0:
        .Ord0:
            dw PatternMemory_Pat3
            dw PatternMemory_Pat1
            dw PatternMemory_Pat1
            dw PatternMemory_Pat1
            dw PatternMemory_Pat1
            dw PatternMemory_Pat1
            dw PatternMemory_Pat1
            dw PatternMemory_Pat1
        .Ord1:
            dw PatternMemory_Pat2
            dw PatternMemory_Pat1
            dw PatternMemory_Pat1
            dw PatternMemory_Pat1
            dw PatternMemory_Pat1
            dw PatternMemory_Pat1
            dw PatternMemory_Pat1
            dw PatternMemory_Pat1

PatternMemory:
    .Pat0:
    %SetDelayCoefficient(0, $FF/16)
    %SetDelayCoefficient(1, $EE/16)
    %SetDelayCoefficient(2, $DD/16)
    %SetDelayCoefficient(3, $CC/16)
    %SetDelayCoefficient(4, $BB/16)
    %SetDelayCoefficient(5, $AA/16)
    %SetDelayCoefficient(6, $99/16)
    %SetDelayCoefficient(7, $88/16)
    %SetDelayTime(4)
    %SetDelayVolume($60,$60)
    %SetDelayFeedback($40)
    %SetSpeed($04)
    %SetChannelVolume($40, $40)
    %SetInstrument(0)
    %SetVib($00)
    %PlayPitch($1000)
    %Sleep($02)
    %PlayPitch($2000)
    %Sleep($02)
    %PlayPitch($1E00)
    %Sleep($02)
    %PlayPitch($1C00)
    %Sleep($02)
    %PlayPitch($1A00)
    %Sleep($02)
    %PlayPitch($1800)
    %Sleep($02)
    %PlayPitch($1600)
    %Sleep($02)
    %PlayPitch($1400)
    %Sleep($02)
    %PlayPitch($1200)
    %Sleep($02)
    %PlayPitch($1000)
    %Sleep($02)
    %Break()
    .Pat1:
    %Sleep($FF)
    %Break()
    .Pat2:
    %SetVib($F2)
    %Sleep($04)
    %Goto(0)
    .Pat3:
    %SetChannelVolume($60, $60)
    %SetInstrument($00)
    %SetSpeed($04)
    %PlayNote($02)
    %Sleep($06)
    %PlayNote($04)
    %Sleep($06)
    %PlayNote($08)
    %Sleep($06)
    %PlayNote($10)
    %Sleep($06)
    %PlayNote($20)
    %Sleep($06)
    %Goto($01)

SfxTable:
    .SFX_Null:
        dw SfxPat_Null
        dw SfxPat_Null
        dw SfxPat_Null
        dw SfxPat_Null
        dw SfxPat_Null
        dw SfxPat_Null
        dw SfxPat_Null
        dw SfxPat_Null
    .SFX_1:
        dw SfxPat_Null
        dw SfxPat_Null
        dw SfxPat_Null
        dw SfxPat_Null
        dw SfxPat_Null
        dw SfxPat_Null
        dw SfxPat_Sfx1_0
        dw SfxPat_Sfx1_1
    .SFX_2:
        dw SfxPat_Sfx2_0
        dw SfxPat_Null
        dw SfxPat_Null
        dw SfxPat_Null
        dw SfxPat_Null
        dw SfxPat_Null
        dw SfxPat_Null
        dw SfxPat_Null

SfxPat:
    .Null:
    %SetSpeed($FE)
    %Sleep(255)
    %Goto(0)
    .Sfx1_0:
    %SetSpeed($01)
    %SetChannelVolume($7F, $7F)
    %SetInstrument($3)
    %PlayPitch($1800)
    %Sleep($10)
    %PlayPitch($2000)
    %Sleep($10)
    %Break()
    .Sfx1_1:
    %SetSpeed($02)
    %PlayPitch($2200)
    %Sleep($10)
    %PlayPitch($2400)
    %Break()
    .Sfx2_0:
    %SetSpeed($03)
    %PlayPitch($0800)
    %Sleep($0E)
    %SetPort($84)
    %Sleep(12)
    %Goto(0)

    ;List of available music tracks
SubtuneList:
    dw OrderTable_Tune0

    ;List of available sound effects
SFXList:
    dw SfxTable_SFX_1

InstrumentMemory:
%WriteInstrument($00, $FF, $80, $7F, %00000000)
%WriteInstrument($01, $FF, $80, $7F, %00000000)
%WriteInstrument($01, $FF, $80, $7F, %00000000)
%WriteInstrument($00, $FF, $80, $93, %00000000)  ;Test SFX

Engine_End:

arch 65816
base ROM_Engine_Start+(Engine_End-DriverStart)
ROM_Engine_End: