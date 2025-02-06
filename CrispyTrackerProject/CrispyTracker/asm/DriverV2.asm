;|==============================|
;|                              |
;|  Cobalt Audio Diver Program  |
;|                              |
;|          Written by          |
;|            Crisps            |
;|                              |
;|           Started            |
;|          08/06/2024          |
;|                              |
;|         Version 1.0b         |
;|          02/01/2025          |
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
mov.b Apu3, #$00
mov.b Apu2, #$00
mov.b Apu1, #$00
mov.b Apu0, #$00

mov ZP.TempMemADDRL, #Engine_End&$FF
mov ZP.TempMemADDRH, #Engine_End>>8
mov.b ZP.R1, #$FF
mov.b ZP.R0, #$C0
mov.b X, #0
mov A, #0
.MemClearLoop:
mov.b A, #$00
db $C7, $00                     ;Equivelant to mov (ZP.TempMemADDRL+X), A; reason is the ASAR doesn't put this line in code, instead putting in C5 00 00
incw.b ZP.TempMemADDRL
movw YA, ZP.TempMemADDRL
cmpw.b YA, ZP.R0
bne .MemClearLoop

mov.b X, #$FF
mov SP, X
setp
mov SP, X
clrp
mov.b X, #$00
mov.b Y, #$00
mov.b A, #$00
-
mov $0100+Y, A
dec.b Y
bne -

mov ZP.TrackSettings, #$00     ;Set the track settings

;Audio register reset routine
mov A, #0
-
mov Y, #$00                         ;Shove into reset value
mov.b X, #$0F
mov SPC_RegADDR, A                  ;Shove address in
Inner:
mov SPC_RegData, Y                  ;Reset volume left
inc.b SPC_RegADDR                     ;Increment address value
dec.b X                               ;Decrement the loop counter
bpl Inner
clrc                                ;Clear carry
adc.b A, #$10                       ;Add 16
bvc -

mov SPC_Control, #$01               ;Set control bit to enable Timer 0 and keep IPL rom inside
mov SPC_Timer1, #$20                ;Divide timer to run at ~250hz
setp
mov A, #$01
mov Y, #$08
-
mov.b OP.OrderChangeFlag+Y, A         ;Set change flag at start to load initial pattern data for both SFX and music
mov.b OP.StopFlag+Y, A                ;Set change flag at start to load initial pattern data for both SFX and music
dec.b Y
bpl -
clrp
mov Y, #$07
mov A, #$00
-
mov ZP.VCOut+Y, A                   ;Reset the VCOut state to F to prevent channel injection
dec.b Y
bpl -
mov.b ZP.SFXRec, #$00

%spc_write(DSP_FLG, $00)
%spc_write(DSP_MVOL_L, $00)
%spc_write(DSP_MVOL_R, $00)
%spc_write(DSP_EVOL_L, $00)
%spc_write(DSP_EVOL_R, $00)
%spc_write(DSP_ESA, $C0)
%spc_write(DSP_EDL, $00)
%spc_write(DSP_EFB, $20)
%spc_write(DSP_DIR, $0C)
%spc_write(DSP_PMON, $00)
%spc_write(DSP_EON, $00)
%spc_write(DSP_NON, $00)

mov.b ZP.OutVol, #$7F               ;Set output volume to max
setp
mov.b OP.MaxVolTarget, #$7F         ;Set output target volume to max
clrp

mov A, LoadFlag
bne .SkipAddr
mov.b ZP.MasterVol, #$00
mov.b ZP.EchoVol, #$00
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
.SkipAddr:
DriverLoop:                             ;Main driver loop
    mov.b X, #0                         ;Reset counter
    
    .TickIncrement:
    call RecieveSub
    call CheckProgrammerControls
    mov.b Y, SPC_Count1                 ;Check counter
    beq .TickIncrement                  ;If the timer is set to 0
    call ProcessEffects                 ;KEEP AROUND, for some reason the effects sound much more correct when this is called an odd number of times [don't ask why]
    call FinaliseOutput
    setp
    mov.b A, OP.StopFlag                ;Stop track from progressing
    clrp
    bne .CheckSFX
    inc.b X                             ;Increment our counter
    cmp.b X, ZP.TickThresh              ;Check if the counter has reached the 
    bmi .TickIncrement                  ;Go back to the tick incrementer if the counter is not
                                        ;Assuming we've hit the threshold
    setp
    mov.b A, OP.OrderChangeFlag         ;Check the order flag with the 0th bit
    beq +                               ;Skip if the carry flag isn't set
    mov.b X, #$07
    mov Y, #$00
    -                                   ;Sleep Clear Loop
    mov.b OP.ChannelSleepCounter+X, Y   ;Clear sleep counter
    dec.b X                             ;Decrement counter
    bpl -
    clrp
    mov.w A, OrderPtr
    mov.b ZP.R0, A
    mov.w A, OrderPtr+1
    mov.b ZP.R1, A
    call ReadPatterns
    setp
    mov.b OP.OrderChangeFlag, #$00
    +
    clrp
    
    mov.b ZP.CurrentChannel,#$07        ;Increment channel index
    .ChannelLoop:                       ;Main channel loop
    mov.b X, ZP.CurrentChannel
    setp
    mov.b Y, OP.ChannelSleepCounter+X
    clrp
    bne .SkipRow                    ;Check if the sleep counter != 0
    call ReadRows
    bra .SkipDec
    .SkipRow:                       ;Sleep counter routine
    setp
    dec.b OP.ChannelSleepCounter+X
    .SkipDec:
    clrp
    call ProcessEffects
    dec.b ZP.CurrentChannel
    bpl .ChannelLoop
    setp
    mov.b A, OP.OrderChangeFlag      ;Order change
    beq +
    mov.b Y, #$0E
    .SetSeq:
    setp
    mov.b A, OP.SequenceTarget+Y
    clrp
    mov.b ZP.SequenceAddr+Y, A
    dec Y
    bpl .SetSeq
    clrp
    +
    clrp
    .CheckSFX:
    call HandleSFX                  ;Handle SFX interpreting
    jmp DriverLoop

ProcessEffects:
    push X
    clrp
    .EffectsLoop:
    mov.b ZP.TempPitchProcess,    #$00
    mov.b ZP.TempPitchProcess+1,  #$00
    mov.b ZP.TempVolumeProcess,   #$00
    mov.b ZP.TempVolumeProcess+1, #$00
    mov.b A, ZP.CurrentChannel        ;Grab channel index
    asl A                           ;Double since we are working with 2 bytes
    mov.b ZP.TempScratchMemH, A       ;Return

    ;-------------------;
    ;   Volume Slide    ;
    ;-------------------;
    mov.b X, ZP.TempScratchMemH
    mov.b A, ZP.VolSlideValue+X
    mov.b ZP.R0, A
    and.b A, #$0F
    mov.b ZP.R1, A
    beq .Up
    ;Dec volume
    setp
    mov.b A, OP.ChannelVolume+X
    clrp
    setc
    sbc.b A, ZP.R1
    setp
    mov.b OP.ChannelVolume+X, A
    clrp
    inc.b X
    setp
    mov.b A, OP.ChannelVolume+X
    clrp
    setc
    sbc.b A, ZP.R1
    setp
    mov.b OP.ChannelVolume+X, A
    clrp
    bra .SkipVSlide
    .Up:
    ;Inc volume
    mov.b A, ZP.R0
    xcn
    mov.b ZP.R1, A
    setp
    mov.b A, OP.ChannelVolume+X
    clrc
    clrp
    adc.b A, ZP.R1
    setp
    mov.b OP.ChannelVolume+X, A
    clrp
    inc.b X
    setp
    mov.b A, OP.ChannelVolume+X
    clrp
    clrc
    adc.b A, ZP.R1
    setp
    mov.b OP.ChannelVolume+X, A
    clrp
    .SkipVSlide:

    ;---------------;
    ;   Portamento  ;
    ;---------------;
    mov.b X, ZP.CurrentChannel
    mov.b A, ZP.PortValue+X
    mov.b ZP.TempMemADDRL, A
    mov.b ZP.TempMemADDRH, #$00
    mov1 C, ZP.TempMemADDRL.7
    bcc +
    mov.b ZP.TempMemADDRH, #$FF
    +
    mov.b X, ZP.TempScratchMemH
    setp
    mov.b Y, OP.ChannelPitches+1+X
    mov A, OP.ChannelPitches+X
    clrp
    addw YA, ZP.TempMemADDRL
    setp
    mov.b OP.ChannelPitches+1+X, Y
    mov.b OP.ChannelPitches+X, A
    clrp
    
    ;--------------------;
    ;       Vibrato      ;
    ;--------------------;
    mov.b X, ZP.CurrentChannel
    mov.b A, ZP.VibratoValue+X
    and.b A, #$F0
    xcn A
    asl A
    mov.b ZP.TempScratchMem, A      ;Hold speed in temporary memory
    mov.b A, ZP.SineIndexVib+X      ;Grab sine index
    adc.b A, ZP.TempScratchMem      ;Increment sine index by speed value
    mov.b ZP.SineIndexVib+X, A
    mov Y, A                        ;Grab sine index value
    mov.b A, ZP.VibratoValue+X
    and.b A, #$0F                   ;Grab depth
    call GetSineValue               ;Call sine function
    call SignedMul                  ;Multiply
    mov.b ZP.R0, A
    mov.b ZP.R1, Y
    mov.b A, ZP.TempPitchProcess
    mov.b Y, ZP.TempPitchProcess+1
    addw YA, ZP.R0
    movw ZP.TempPitchProcess, YA
    
    ;----------------;
    ;   Tremolando   ;
    ;----------------;
    mov.b X, ZP.CurrentChannel
    mov.b A, ZP.TremolandoValue+X
    and.b A, #$F0
    xcn A
    mov.b ZP.TempScratchMem, A          ;Hold speed in temporary memory
    mov.b A, ZP.SineIndexTrem+X         ;Grab sine index
    adc.b A, ZP.TempScratchMem          ;Increment sine index by speed value
    mov.b ZP.SineIndexTrem+X, A
    mov Y, A                            ;Grab sine index value
    mov.b A, ZP.TremolandoValue+X
    and.b A, #$0F                       ;Grab depth
    call GetSineValue                   ;Call sine function
    call SignedMul                      ;Multiply sine influence
    mov.b ZP.R0, A                      ;Shove into temp memory
    mov.b ZP.R1, Y
    lsr.b ZP.R1                         ;Div by 2
    ror.b ZP.R0                         ;Rotate by 2
    lsr.b ZP.R1                         ;Div by 2
    ror.b ZP.R0                         ;Rotate by 2
    lsr.b ZP.R1                         ;Div by 2
    ror.b ZP.R0                         ;Rotate by 2
    lsr.b ZP.R1                         ;Div by 2
    ror.b ZP.R0                         ;Rotate by 2
    mov.b A, ZP.TempVolumeProcess       ;Grab current volume process
    clrc
    adc.b A, ZP.R0
    mov.b ZP.TempVolumeProcess, A       ;Apply
    mov.b A, ZP.TempVolumeProcess+1     ;Grab current volume process
    clrc
    adc.b A, ZP.R0
    mov.b ZP.TempVolumeProcess+1, A     ;Apply

    ;----------------;
    ;    Panbrello   ;
    ;----------------;
    mov.b X, ZP.CurrentChannel
    mov.b A, ZP.PanbrelloValue+X
    and.b A, #$F0
    xcn A
    mov.b ZP.TempScratchMem, A          ;Hold speed in temporary memory
    mov.b A, ZP.SineIndexPanbr+X        ;Grab sine index
    adc.b A, ZP.TempScratchMem          ;Increment sine index by speed value
    mov.b ZP.SineIndexPanbr+X, A
    mov Y, A                            ;Grab sine index value
    mov.b A, ZP.PanbrelloValue+X
    and.b A, #$0F                       ;Grab depth
    call GetSineValue                   ;Call sine function
    call SignedMul                      ;Multiply sine influence
    mov.b ZP.R0, A                      ;Shove into temp memory
    mov.b ZP.R1, Y
    lsr.b ZP.R1                         ;Div by 2
    ror.b ZP.R0                         ;Rotate by 2
    lsr.b ZP.R1                         ;Div by 2
    ror.b ZP.R0                         ;Rotate by 2
    lsr.b ZP.R1                         ;Div by 2
    ror.b ZP.R0                         ;Rotate by 2
    lsr.b ZP.R1                         ;Div by 2
    ror.b ZP.R0                         ;Rotate by 2
    mov.b A, ZP.TempVolumeProcess       ;Grab current volume process
    clrc
    adc.b A, ZP.R0
    mov.b ZP.TempVolumeProcess, A     ;Apply
    mov.b A, ZP.TempVolumeProcess+1   ;Grab current volume process
    setc
    sbc.b A, ZP.R0
    mov.b ZP.TempVolumeProcess+1, A   ;Apply

    mov.b A, ZP.CurrentChannel
    and.b A, #$07
    mov.b ZP.InjectionChannel, A
    ;Prevent doing channel mixing if working Virtual channels
    mov.b A, ZP.CurrentChannel
    and.b A, #$08
    bne .CheckSFXInjection

    ;Music
    ;Check if the current VC-Out is ON
    mov.b Y, ZP.InjectionChannel
    mov.b A, ZP.VCOut+Y
    beq .OutputAudio
    jmp .SkipInst

    .CheckSFXInjection
    ;SFX
    ;Check if the current VC-Out is OFF
    mov.b Y, ZP.InjectionChannel
    mov.b A, ZP.VCOut+Y
    bne .OutputAudio
    jmp .SkipInst

    ;---------------------------;
    ;       Mixing & Output     ;
    ;---------------------------;
    .OutputAudio:
    ;Grab current instrument
    mov.w A, InstPtr
    mov.w Y, InstPtr+1
    clrp
    mov.b ZP.TempMemADDRL, A
    mov.b ZP.TempMemADDRH, Y
    mov.b X, ZP.CurrentChannel
    setp
    mov.b A, OP.ChannelInstrumentIndex+X
    clrp
    mov.b Y, #!InstWidth
    mul YA
    addw.b YA, ZP.TempMemADDRL
    movw.b ZP.TempMemADDRL, YA
    mov.b A, ZP.InjectionChannel
    xcn
    or.b A, #$04
    mov.b SPC_RegADDR, A
    mov.b X, #$00
    mov Y, #$03

    .InstWrite:
    mov.b A, (ZP.TempMemADDRL+X)
    mov.b SPC_RegData, A
    inc.b SPC_RegADDR
    incw.b ZP.TempMemADDRL
    dec Y
    bpl .InstWrite

    ;PMON, EON, NON
    mov.b X, ZP.InjectionChannel            ;Grab current channel
    mov.w A, BitmaskTable+X                 ;Grab bitfield
    eor.b A, #$FF                           ;Invert current bit
    mov.b ZP.TempScratchMem, A              ;Shove into temp memory
    mov.b SPC_RegADDR, #DSP_PMON            ;Shove PMON into regaddr
    mov.b X, #$00                           ;Reset X
    mov.b ZP.TempScratchMemH, #$01          ;Shove 1 into temp memory
    mov.b Y, #$03                           ;Initialise loop counter

    .RestartLoop:
    and.b SPC_RegData, ZP.TempScratchMem    ;AND the current bitfield with inverted bitmask table
    mov.b A, (ZP.TempMemADDRL+X)            ;Grab effects state
    and.b A, ZP.TempScratchMemH             ;AND effects state and current comparison bit
    beq .SkipAppl                           
    eor.b ZP.TempScratchMem, #$FF           ;Undo inversion in scratch memory
    or.b SPC_RegData, ZP.TempScratchMem     ;Combine the cleared bit with the comparison bit
    eor.b ZP.TempScratchMem, #$FF           ;Reinvert the channel bit
    .SkipAppl:  
    asl.b ZP.TempScratchMemH                ;LShift comparison bit
    adc.b SPC_RegADDR, #$10                 ;Add on $10 to the address to get to other bitfields
    dec.b Y                                 ;Dec loop counter
    bne .RestartLoop

    mov.b A, ZP.CurrentChannel
    asl A
    mov.b ZP.TempScratchMemH, A
    mov.b X, A
    setp
    mov.b A, OP.ChannelVolume+X
    clrp
    mov.b ZP.ChannelVolumeOutput, A                     ;Shove into volume output
    
    mov.b X, ZP.TempScratchMemH
    setp
    mov.b A, OP.ChannelVolume+1+X
    clrp
    mov.b ZP.ChannelVolumeOutput+1, A                     ;Shove into volume output

    ;Add Volume offset from effects
    ;L
    mov.b A, ZP.ChannelVolumeOutput
    adc.b A, ZP.TempVolumeProcess
    bvc .SkipLCorr                                      ;Overflow detection
    bmi .NegativeL                                      ;Check negative
    mov.b A, #$80                                       ;Clamp output
    bra .SkipLCorr
    .NegativeL:
    mov.b A, #$7F                                       ;Clamp output
    .SkipLCorr:
    mov ZP.ChannelVolumeOutput, A
    ;R
    mov.b A, ZP.ChannelVolumeOutput+1
    adc.b A, ZP.TempVolumeProcess+1
    bvc .SkipRCorr                                      ;Overflow detection
    bmi .NegativeR                                      ;Check negative
    mov.b A, #$80                                       ;Clamp output
    bra .SkipRCorr
    .NegativeR:
    mov.b A, #$7F                                       ;Clamp output
    .SkipRCorr:
    mov.b ZP.ChannelVolumeOutput+1, A

    ;Force output to mono
    mov.b C, ZP.TrackSettings.0
    bcc .SkipMono
    clr1.b ZP.ChannelVolumeOutput.7
    clr1.b ZP.ChannelVolumeOutput+1.7
    .SkipMono:

    ;Disable echo flag
    mov.b C, ZP.TrackSettings.1
    bcc .SkipEchoDisable
    mov.b SPC_RegADDR, #DSP_FLG
    mov.b A, SPC_RegData
    or.b A, #$20
    mov.b SPC_RegData, A
    .SkipEchoDisable:

    ;Mask out channel volume if mask is on
    mov.b X, ZP.InjectionChannel
    mov.b A, ZP.ChannelMask
    and.w A, BitmaskTable+X
    beq +
    mov.b ZP.ChannelVolumeOutput, #$00
    mov.b ZP.ChannelVolumeOutput+1, #$00
    +

    ;Apply Volume
    mov.b X, ZP.TempScratchMemH                           ;Grab Premult channel index
    mov.b Y, ZP.ChannelVolumeOutput                       ;Grab L output volume
    mov.b A, ZP.InjectionChannel                          ;Grab current channel
    xcn A                                                 ;Swap nibbles to get correct channel addr
    mov.b SPC_RegADDR, A                                  ;Shove channel addr
    mov.b SPC_RegData, Y                                  ;Shove data in
    inc.b SPC_RegADDR                                     ;Inc address to get R volume
    mov.b Y, ZP.ChannelVolumeOutput+1                     ;Grab R output volume
    mov.b SPC_RegData, Y                                  ;Shove data in
    
    ;Channel pitch application
    mov.b A, ZP.InjectionChannel                          ;Grab current channel
    xcn A                                                 ;XCN for correct register addr
    mov.b ZP.TempMemADDRL, A                              ;Store multiplied index
    or A, #$02                                            ; + 2
    mov.b SPC_RegADDR, A                                  ;Shove into addr
    
    mov.b X, ZP.TempScratchMemH                           ;Grab Premult channel index
    setp
    mov.b A, OP.ChannelPitches+X                          ;Grab current channel's lo pitch
    clrp
    adc.b A, ZP.TempPitchProcess                          ;Add lo byte to pitch offset
    mov.b ZP.ChannelPitchesOutput, A                      ;Put into output
    setp
    mov.b A, OP.ChannelPitches+1+X                        ;Grab current channel's hi pitch
    clrp
    adc.b A, ZP.TempPitchProcess+1                        ;Add hi byte to pitch offset
    mov.b ZP.ChannelPitchesOutput+1, A                    ;Put into output

    mov A, ZP.ChannelPitchesOutput
    setp
    mov.b OP.RegPitchWrite+X, A
    clrp
    mov A, ZP.ChannelPitchesOutput+1
    setp
    mov.b OP.RegPitchWrite+1+X, A
    clrp
    ;Jump over if working Virtual channels
    .SkipInst:
    pop X
    ret

FinaliseOutput:
    push X
    mov Y, #$0F
    mov X, #$07
    -
    mov A, X
    xcn
    or.b A, #$03
    mov.b SPC_RegADDR, A

    mov A, ZP.VCOut+X
    beq +
    setp
    mov.b A, OP.RegPitchWrite+$10+Y
    clrp
    mov.b SPC_RegData, A
    dec Y
    dec.b SPC_RegADDR
    setp
    mov.b A, OP.RegPitchWrite+$10+Y
    clrp
    mov.b SPC_RegData, A
    bra .DecLoop
    +
    setp
    mov.b A, OP.RegPitchWrite+Y
    clrp
    mov.b SPC_RegData, A
    dec Y
    dec.b SPC_RegADDR
    setp
    mov.b A, OP.RegPitchWrite+Y
    clrp
    mov.b SPC_RegData, A
    .DecLoop:
    dec Y
    dec X
    bpl -
    clrp
    mov.b A, ZP.KONState
    beq +
    mov.b SPC_RegADDR, #DSP_KON
    mov.b SPC_RegData, ZP.KONState
    mov.b ZP.KONState, #$00
    +

    ;Handle fade code
    mov.b A, ZP.FadeFlag
    beq .SkipFade
    mov.b A, ZP.OutVol
    mov1 C, ZP.FadeSpeed.7
    bcc +
    sbc.b A, ZP.FadeSpeed
    bcs .ZVal
    bra .CompareMax
    +
    setc
    sbc.b A, ZP.FadeSpeed
    bcc .ZVal
    .CompareMax:
    setp
    cmp A, OP.MaxVolTarget
    bmi .ApplyFade
    ;Assume we've hit the max volume
    mov A, OP.MaxVolTarget
    bra .ApplyFade
    .ZVal:
    mov.b A, #$00
    .ApplyFade:
    clrp
    mov.b ZP.OutVol, A
    .SkipFade:
    
    ;Mix L/R audio output with new master volume
    mov.b SPC_RegADDR, #DSP_MVOL_L
    mov.b A, ZP.MasterVol
    mov.b Y, ZP.OutVol
    mov.b X, #$7F
    mul YA
    div YA, X
    mov.b SPC_RegData, A
    mov.b SPC_RegADDR, #DSP_MVOL_R
    mov.b A, ZP.MasterVol
    mov.b Y, ZP.OutVol
    mov.b X, #$7F
    mul YA
    div YA, X
    mov.b SPC_RegData, A

    ;Mix L/R echo output with new master volume
    mov.b SPC_RegADDR, #DSP_EVOL_L
    mov.b A, ZP.EchoVol
    mov.b Y, ZP.OutVol
    mov.b X, #$7F
    mul YA
    div YA, X
    mov.b SPC_RegData, A
    mov.b SPC_RegADDR, #DSP_EVOL_R
    mov.b A, ZP.EchoVol
    mov.b Y, ZP.OutVol
    mov.b X, #$7F
    mul YA
    div YA, X
    mov.b SPC_RegData, A
    .SkipOutput:
    pop X
    ret

    ;
    ;   Clobberlist:
    ;       ZP.R0       Aim table lo
    ;       ZP.R1       Aim table hi
    ;       ZP.R3       Hardware/Virtual channel flag
    ;
ReadPatterns:
    mov.b ZP.R3, #$00
    mov.b X, ZP.CurrentChannel
    mov A, X
    and.b A, #$08
    beq +
    mov.b ZP.R3, #$01
    mov.b A, ZP.CurrentChannel
    and.b A, #$07
    asl A
    mov X, A
    setp
    mov.b A, OP.OrderPos+2+X                   ;Grab the current order ptr [sfx]
    mov.b Y, OP.OrderPos+3+X
    bra .OrderRead
    +
    setp
    mov.b A, OP.OrderPos                       ;Grab the current order ptr [music]
    mov.b Y, OP.OrderPos+1
    .OrderRead:
    clrp
    movw ZP.TempMemADDRL, YA                   ;Return address to memory
    ;Fill address pointers for music and SFX
    mov.b A, ZP.R3
    bne .SFXAddr
    mov.b Y, #$0F                               ;Set Y up for loop
    -                                           ;Loop point
    mov A, (ZP.TempMemADDRL)+Y                  ;Indirectly shove addr value into A
    mov ZP.SequenceAddr+Y, A                    ;Copy value to sequence addr
    .SfxAddrLoop:
    dec Y                                       ;Decrement loop counter
    bpl -                                       ;Loop
    ret
    .SFXAddr:
    mov.b A, ZP.CurrentChannel
    and.b A, #$07
    asl.b A
    mov Y, A
    mov.b A, (ZP.TempMemADDRL)+Y               ;Indirectly shove addr value into A
    mov.b ZP.SequenceAddr+$10+Y, A             ;Copy value to sequence addr
    inc.b Y                                    ;Decrement loop counter
    mov.b A, (ZP.TempMemADDRL)+Y               ;Indirectly shove addr value into A
    mov.b ZP.SequenceAddr+$10+Y, A             ;Copy value to sequence addr
    ret

HandleSFX:
    ;Handle timers
    mov.b ZP.CurrentChannel, #$0F
    .SfxLoop:
    mov.b X, ZP.CurrentChannel
    ;Check if our tick counter has reached 0
    setp
    mov.b A, OP.StopFlag-7+X
    bne .DecTick
    mov.b A, OP.ChannelSleepCounter+X
    bne .DecTick
    ;Read pattern if the counter is currently 0
    clrp
    mov.b A, ZP.CurrentChannel
    and.b A, #$07
    mov.b X, A
    setp
    mov.b A, OP.OrderChangeFlag+1+X
    beq .SkipPatternRead
    ;Erase flag
    mov.b A, #$00
    mov.b OP.OrderChangeFlag+1+X, A
    ;Setup pattern read
    clrp
    mov.b A, ZP.CurrentChannel
    and A, #$07
    mov.b X, A
    mov.w A, SfxPatPtr
    mov.w Y, SfxPatPtr+1
    mov.b ZP.R0, A
    mov.b ZP.R1, Y
    ;Note, don't do -7 on the OrderPos, you'll end up reading at $01F9
    call ReadPatterns
    clrp
    mov.B X, ZP.CurrentChannel
    mov.b A, ZP.VCTickThresh+X
    setp
    mov.b OP.ChannelSleepCounter+X, A
    clrp
    .SkipPatternRead:
    ;If tick timer == 0 then we decrement our sleep counter
    mov.b X, ZP.CurrentChannel
    setp
    mov.b A, OP.ChannelSleepCounter+X
    clrp
    bne .ApplySleep
    call ReadRows
    .ApplySleep:
    mov.b X, ZP.CurrentChannel
    setp
    mov.b OP.ChannelSleepCounter+X, A
    .SkipSleep:
    .DecTick:
    dec.b OP.ChannelSleepCounter+X
    clrp
    call ProcessEffects
    dec.b ZP.CurrentChannel
    mov.b X, ZP.CurrentChannel
    cmp.b X, #$07
    bne .SfxLoop
    mov.b ZP.CurrentChannel, #$00
    ret

ReadRows:
    call GrabCommand
    mov.b ZP.R0, A
    setc
    sbc.b A, #RC.PlayNote&$FF
    bcc +
    jmp (Row_PlayNote)
    +
    mov.b A, ZP.R0
    asl A                               ;Mult by 2 to prevent reading in the wrong address byte
    mov.b X, A                          ;Shove into X for the jump table
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
    dw Row_MasterVol
    
Row_SetSpeed:
    clrp
    mov.b A, ZP.CurrentChannel
    and.b A, #$08
    bne +
    ;Music
    call GrabCommand
    mov.b ZP.TickThresh, A
    bra ReadRows
    +
    mov.b A, ZP.CurrentChannel
    and A, #$07
    mov.b Y, A
    call GrabCommand
    mov.b ZP.VCTickThresh+Y, A
    bra ReadRows

Row_Sleep:
    call GrabCommand
    mov.b X, ZP.CurrentChannel
    setp
    mov.b OP.ChannelSleepCounter+X, A
    clrp
    ret                             ;ret is exiting out of the read rows subroutine

Row_Goto:
    call GetGotoInd
    mov A, X
    asl A
    mov X, A
    call GrabCommand
    setp
    mov.b OP.OrderPos+X, A            ;Return pos
    clrp
    call GrabCommand
    setp
    mov.b OP.OrderPos+1+X, A          ;Return pos
    mov A, #$01
    mov.b OP.OrderChangeFlag+X, A           ;Set the order change flag
    clrp
    ret

Row_Break:
    call GetGotoInd
    mov A, X
    asl A
    mov X, A
    setp
    mov.b A, OP.OrderPos+X
    clrc
    adc.b A, #$10
    mov.b OP.OrderPos+X, A
    bcc +
    mov.b A, OP.OrderPos+1+X
    inc A
    mov.b OP.OrderPos+1+X, A   
    +
    mov A, X
    bne .DoSfx
    mov.b X, #$00
    mov.b Y, #$0F
    -
    mov.b A, (OP.OrderPos)+Y
    mov.b OP.SequenceTarget+Y, A
    dec Y
    mov.b A, (OP.OrderPos)+Y
    mov.b OP.SequenceTarget+Y, A
    dec Y
    bpl -
    mov.b OP.OrderChangeFlag, #$01         ;Set the order change flag
    clrp
    ret
    .DoSfx:
    mov.b A, (OP.OrderPos+X)
    mov.b OP.SequenceTarget+X, A

    inc.b OP.OrderPos+X
    bcc +
    inc.b OP.OrderPos+1+X
    +

    mov.b A, (OP.OrderPos+X)
    mov.b OP.SequenceTarget+1+X, A
    mov A, #$01
    mov.b OP.OrderChangeFlag+X, A         ;Set the order change flag
    clrp
    ret                                 ;Break out of ReadRows

Row_PlayNote:
    mov.b A, ZP.R0                    ;Grab pitch index
    setc
    sbc A, #(RC.PlayNote)&$FF       ;Offset index by PlayNote
    asl A
    mov Y, A
    setp
    mov.b A, (PitchPtr)+Y             ;Grab lo pitch in table
    mov.b OP.ChannelPitches+X, A
    inc.b Y
    mov.b A, (PitchPtr)+Y             ;Grab hi pitch in table
    mov.b OP.ChannelPitches+1+X, A
    clrp
    bra Row_PlayPitch_SetKOn

Row_PlayPitch:
    ;Pitch application
    call GrabCommand                ;Grab lo byte of the pitch
    setp
    mov.b OP.ChannelPitches+X, A      ;Shove lo byte into pitch
    clrp
    call GrabCommand                ;Grab hi byte of the pitch
    setp
    mov.b OP.ChannelPitches+1+X, A    ;Shove hi byte into pitch
    clrp
    .SetKOn:
    ;KON State
    mov.b A, ZP.CurrentChannel            ;Grab current channel
    and.b A, #$07
    mov X, A
    mov.b A, ZP.KONState
    or.w A, BitmaskTable+X              ;Get bitmask index via X
    mov.b ZP.KONState, A

    mov.b A, ZP.CurrentChannel        ;Check if handling SFX or Music
    and.b A, #$08
    bne .GotoSFX
    ;Music
    ;Set SFX flag off
    mov.b X, ZP.CurrentChannel            ;Grab index
    mov.b A, #$00
    mov.b ZP.VCOut+X, A
    jmp ReadRows
    .GotoSFX:
    ;SFX flag on
    mov.b X, ZP.CurrentChannel              ;Grab index
    mov ZP.VCOut-8+X, A                     ;Offset VCOUT by 8 beacause working with virtual channels
    jmp ReadRows

Row_SetInstrument:
    mov.w A, InstPtr
    mov.w Y, InstPtr+1
    clrp
    mov.b ZP.TempMemADDRL, A
    mov.b ZP.TempMemADDRH, Y
    call GrabCommand
    mov.b X, ZP.CurrentChannel
    setp
    mov.b OP.ChannelInstrumentIndex+X, A
    clrp
    mov Y, #!InstWidth
    mul YA
    addw YA, ZP.TempMemADDRL
    movw ZP.TempMemADDRL, YA

    mov.b A, ZP.CurrentChannel
    and.b A, #$08
    beq +
    jmp ReadRows
    +
    ;SCRN
    mov.b A, ZP.CurrentChannel            ;Grab current channel
    xcn A                                 ;XCN to get correct channel in memory
    or.b A, #$04                          ;Add 4 for correct nibble
    mov.b SPC_RegADDR, A                  ;SCRN
    mov.b Y, #$03
    .ApplyInst:
    mov.b X, #$00                         ;Reset X
    mov.b A, (ZP.TempMemADDRL+X)          ;Shove value into A
    mov.b SPC_RegData, A                  ;Apply
    incw ZP.TempMemADDRL
    inc.b SPC_RegADDR
    dec Y
    bpl .ApplyInst

    ;PMON, EON, NON, Channel ON
    mov.b X, ZP.CurrentChannel              ;Grab current channel
    mov.w A, BitmaskTable+X                 ;Grab bitfield
    eor.b A, #$FF                           ;Invert current bit
    mov.b ZP.TempScratchMem, A              ;Shove into temp memory
    mov.b SPC_RegADDR, #DSP_PMON            ;Shove PMON into regaddr
    mov.b X, #$00                           ;Reset X
    mov.b ZP.TempScratchMemH, #$01          ;Shove 1 into temp memory
    mov.b Y, #$03                           ;Initialise loop counter

    .RestartLoop:
    and.b SPC_RegData, ZP.TempScratchMem    ;AND the current bitfield with inverted bitmask table
    mov.b A, (ZP.TempMemADDRL+X)            ;Grab effects state
    and.b A, ZP.TempScratchMemH             ;AND effects state and current comparison bit
    beq .SkipAppl                           
    eor.b ZP.TempScratchMem, #$FF           ;Undo inversion in scratch memory
    or.b SPC_RegData, ZP.TempScratchMem     ;Combine the cleared bit with the comparison bit
    eor.b ZP.TempScratchMem, #$FF           ;Reinvert the channel bit
    .SkipAppl:  
    asl.b ZP.TempScratchMemH                ;LShift comparison bit
    adc.b SPC_RegADDR, #$10                 ;Add on $10 to the address to get to other bitfields
    dec.b Y                                 ;Dec loop counter
    bne .RestartLoop
    jmp ReadRows

Row_SetFlag:
    call GrabCommand                        ;Grab value
    mov.b SPC_RegADDR, #DSP_FLG             ;Get correct addr
    mov.b SPC_RegData, A                    ;Apply
    jmp ReadRows

Row_SetDelay:
    call GrabCommand                ;Grab delay value
    mov.b SPC_RegADDR, #DSP_EDL
    mov.b SPC_RegData, A
    jmp ReadRows

Row_SetDelayVolume:
    call GrabCommand                ;Grab delay value
    mov.b ZP.EchoVol, A
    jmp ReadRows

Row_SetDelayFeedback:
    call GrabCommand                ;Grab delay value
    mov.b SPC_RegADDR, #DSP_EFB
    mov.b SPC_RegData, A
    jmp ReadRows

Row_SetDelayCoeff:
    mov A, X
    lsr A                       ;Divide A by 2 since it's premultiplied for the jump table
    setc
    sbc.w A, #(RC.EchoCoeff)&$FF  ;Grab current coeffecient
    xcn
    clrc
    adc.b A, #$0F
    mov.b SPC_RegADDR, A
    call GrabCommand                ;Grab coeff value
    mov.b SPC_RegData, A              ;Apply
    jmp ReadRows

Row_SetChannelVolume:
    call GrabCommand                ;Grab L volume
    setp
    mov.b OP.ChannelVolume+X, A       ;Apply
    clrp
    call GrabCommand                ;Grab R volume
    setp
    mov.b OP.ChannelVolume+1+X, A     ;Apply
    clrp
    jmp ReadRows

Row_SetArp:
    call GrabCommand
    mov.b X, ZP.CurrentChannel
    mov.b ZP.ArpValue+X, A
    jmp ReadRows

Row_SetPort:
    call GrabCommand
    mov.b X, ZP.CurrentChannel
    mov.b ZP.PortValue+X, A
    jmp ReadRows

Row_SetVib:
    call GrabCommand
    mov.b X, ZP.CurrentChannel
    mov.b ZP.VibratoValue+X, A
    jmp ReadRows

Row_SetTrem:
    call GrabCommand
    mov.b X, ZP.CurrentChannel
    mov.b ZP.TremolandoValue+X, A
    jmp ReadRows

Row_SetVolSlide:
    call GrabCommand
    mov.b X, ZP.CurrentChannel
    mov.b ZP.VolSlideValue+X, A
    jmp ReadRows

Row_SetPanbr:
    call GrabCommand
    mov.b X, ZP.CurrentChannel
    mov.b ZP.PanbrelloValue+X, A
    jmp ReadRows

Row_NoteRelease:
    ;KOF State
    mov.b X, ZP.CurrentChannel            ;Grab current channel
    mov A, BitmaskTable+X                 ;Get bitmask index via X
    mov.b ZP.TempScratchMem, A            ;Shove into scratch memory for ORing
    mov.b SPC_RegADDR, #DSP_KOF           ;Shove KOF addr in
    or.b SPC_RegData, ZP.TempScratchMem   ;OR into A
    ret

Row_Stop:
    setp
    call GetGotoInd
    mov.b A, #$01
    setp
    mov.b OP.StopFlag+X, A
    clrp
    mov A, X
    beq +
    mov A, #$00
    mov.b ZP.VCOut+X, A
    +
    ret

Row_MasterVol:
    call GrabCommand
    mov.b ZP.MasterVol, A
    jmp ReadRows

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
    mov.b A, ZP.CurrentChannel              ;Grab current channel
    asl A                                   ;Mult by 2 to get the correct offset into the sequence pointers
    mov.b X, A
    mov.b A, (ZP.SequenceAddr+X)            ;Grab command from the current channel's command stream
    inc.b ZP.SequenceAddr+X                 ;Increment the channel's lo sequence pointer
    bne +                                   ;Check for 0 if we've overflown
    inc.b ZP.SequenceAddr+1+X               ;Increment hi byte
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
        ;           else return (ZP.CurrentChannel&$08)+1
        ;       
        ;   Clobber list:
        ;       X: command indexing
        ;
        ;
GetGotoInd:
    push P
    clrp
    push A                          ;Store away A value if need be
    mov.b X, #$00                     ;Assume music track flag index
    mov.b A, ZP.CurrentChannel
    and.b A, #$08
    beq +
    mov.b A, ZP.CurrentChannel        ;Grab current channel
    and.b A, #$07                     ;Get channel equivelant
    inc.b A                           ;Increment since first flag is for the music track
    mov.b X, A                        ;Move into X
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
    mov.b ZP.TempScratchMem, Y          ;Shove the index into scratch mem
    mov A, Y                            ;Shove index into A for modulo
    and.b A, #$3F                       ;Remove last 2 bits to clamp to 0-63
    mov.b ZP.TempMemADDRL, A            ;Shove value into temporary memory
    mov1.b C, ZP.TempScratchMem.6       ;Shove 6th bit into carry to determine the X flip
    bcc +                               ;Jump over if (> 64 & < 128) | (> 192 & < 256)
    mov.b A, #$3F                       ;Shove subtraction into A
    setc
    sbc.b A, ZP.TempMemADDRL            ;Subtract
    +
    mov Y, A
    mov.w A, SineTable+Y                ;Add on the sine table value
    mov1 C, ZP.TempScratchMem.7         ;Shove 7th bit into carry to determine the Y flip
    bcc +                               ;Jump over if < 128
    eor.b A, #$FF                       ;Invert values
    inc.b A
    +
    mov Y, A                            ;Return value to Y
    pop A                               ;Grab original A value
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
    push X
    ;Save L and H temp memory as we use it outside of this function
    mov.b X, ZP.TempMemADDRL
    push X
    mov.b X, ZP.TempMemADDRH
    push X
    
    mov.b ZP.TempMemADDRL, A
    mov.b ZP.TempMemADDRH, Y
    mul YA

    ; save product into temporary memory
    mov.b ZP.MulProductTemp, A
    mov.b ZP.MulProductTemp+1, Y

    ; check for MSB
    mov.b A, ZP.TempMemADDRL
    bpl .skip_mul1
    setc
    mov.b A, ZP.MulProductTemp+1
    sbc.b A, ZP.TempMemADDRH
    mov.b ZP.MulProductTemp+1, A
    .skip_mul1:

    ; check for MSB
    mov.b A, ZP.TempMemADDRH
    bpl .skip_mul2
    setc
    mov.b A, ZP.MulProductTemp+1
    sbc.b A, ZP.TempMemADDRL
    mov.b ZP.MulProductTemp+1, A
    .skip_mul2:

    mov.b A, ZP.MulProductTemp
    mov.b Y, ZP.MulProductTemp+1

    ;Restore L and H temp memory
    pop X
    mov.b ZP.TempMemADDRH, X
    pop X
    mov.b ZP.TempMemADDRL, X
    pop X
    ret

    ;------------------------------;
    ;   Recieve Subtunes from CPU  ;
    ;------------------------------;
    ;
    ;   Input:
    ;       Apu0    Subtune index to play
    ;       Apu2    What type of tune/sfx to play
    ;
    ;   Output:
    ;       Parse into Subtune table if Apu0 != 0
    ;
    ;   Clobberlist
    ;       APU01
    ;       ZP.R0            \
    ;       ZP.R1             |
    ;       ZP.TempMemADDRL   |
    ;       ZP.TempMemADDRH  /  Addr pointers
    ;    
    ;       ZP.R2           \   General memory storage
    ;       ZP.R3           /
    ;
    ;   Notes
    ;       ZP.R4 \ Address to subtune
    ;       ZP.R5 /
    ;
RecieveSub:
    push A
    push X
    push Y
    mov.b A, Apu1                         ;Check SEND byte
    cmp.b A, ZP.SFXRec
    beq +                                 ;if SFXRec != SFXRec before then new a new subtune will be selected
    jmp .SkipSFXCheck
    +
    inc.b ZP.FlagVal
    mov.b ZP.TempScratchMem, Apu2
    mov.b ZP.R4+1, #$00
    mov.b A, Apu0
    asl A
    bcc +
    mov.b ZP.R4+1, #$01
    +
    mov.b ZP.R4, A
    mov.b X, #$00                         ;Setup music ptr
    mov.w A, SubPtr
    mov.w Y, SubPtr+1
    clrc
    addw YA, ZP.R4
    mov.b ZP.R0, A
    mov.b ZP.R1, Y                        ;Pointer constructed
    mov.b A, ZP.TempScratchMem            ;Check if APU2 is playing music, if not then we assume it's SFX
    cmp.b A, #ProCom.PlayMusic
    beq .DoMusic
    cmp.b A, #ProCom.PlaySfx               ;Check if APU2 is playing sfx, if not then we assume it's SFX
    beq .DoSFX
    jmp .SkipSFXCheck
    .DoSFX:
    ;SFX
    mov A, SfxListPtr
    mov Y, SfxListPtr+1
    clrc
    addw YA, ZP.R4
    mov.b ZP.R0, A
    mov.b ZP.R1, Y
    mov.b X, #$00                               ;Set X to 0 for some pointer shenanigans
    mov.b A, (ZP.R0+X)                          ;Grab order index
    mov.b ZP.R3, A
                                                ;Grab current Address table for SFX
    xcn A                                       ;Mult by 16
    mov.b ZP.TempMemADDRH, A                    ;Shove into hi zp
    mov.b ZP.TempMemADDRL, A                    ;Shove into lo zp
    and.b ZP.TempMemADDRH, #$0F                 ;Get lo nibble
    and.b ZP.TempMemADDRL, #$F0                 ;Get hi nibble  
    mov A, SfxPatPtr                            ;Shove lo table addr into A
    mov Y, SfxPatPtr+1                          ;Shove hi table addr into Y
    addw YA, ZP.TempMemADDRL                    ;Add offset to YA
    movw ZP.TempMemADDRL, YA                    ;Return address to memory

    mov.b X, #$0E
    mov.b Y, #$07                               ;Set loop index
    mov.b A, ZP.R3                              ;Grab saved value
    .OrderSetLoopSFX:
    ;Check to make sure the current address pattern is not NULL
    clrp
    mov.b ZP.R2, Y
    mov.b A, ZP.R2
    asl A
    inc A
    mov.b Y, A
    mov.b A, (ZP.TempMemADDRL)+Y                ;Grab hi byte of the address, if 0 then it must be blank
    bne .SetOrder
    mov.b Y, ZP.R2
    bra .DecLoop
    .SetOrder:
    mov.b A, (ZP.TempMemADDRL)+Y                ;Write address in
    mov.b ZP.SequenceAddr+$11+X, A
    dec Y
    mov.b A, (ZP.TempMemADDRL)+Y                ;Write address in
    mov.b ZP.SequenceAddr+$10+X, A
    mov.b A, ZP.R3
    mov.b Y, ZP.R2
    setp
    mov.b OP.SequenceTarget+2+X, A
    mov.b OP.OrderPos+2+X, A
    push A
    mov A, #$01
    mov.b OP.OrderChangeFlag+1+Y, A
    mov A, #$00
    mov.b OP.StopFlag+1+Y, A
    pop A
    .DecLoop:
    dec.b X
    dec.b X
    dec.b Y
    bpl .OrderSetLoopSFX
    clrp
    bra .ClearSleep
    .DoMusic:
    ;Music
    clrp
    mov A, (ZP.R0+X)                            ;Grab order lo byte
    setp
    mov.b OP.SequenceTarget, A                  ;Store to GOTO lo byte
    mov.b OP.OrderPos, A
    clrp
    incw.b ZP.R0
    mov A, (ZP.R0+X)                            ;Grab order hi byte
    setp
    mov.b OP.SequenceTarget+1, A                ;Store to GOTO hi byte
    mov.b OP.OrderPos+1, A
    mov.b OP.OrderChangeFlag, #$01
    mov.b OP.StopFlag, #$00
    clrp
    
    ;Clear sleep timers
    .ClearSleep:
    mov.b Y, #$07
    mov.b X, ZP.TempScratchMem
    beq +
    ;SFX
    mov.b X, #$0E
    setp
    -
    push Y
    push X
    setp
    mov.b A, OP.SequenceTarget+2+X
    mov.b Y, OP.SequenceTarget+3+X
    clrp
    movw.b ZP.TempMemADDRL, YA                  ;Return address to memory
    pop Y                                       ;Grab X from stack early for original index into Y
    inc Y
    mov.b A, (ZP.TempMemADDRL)+Y                ;Grab hi byte of the address, if 0 then it must be blank
    bne .SkipResetFlag
    mov.b ZP.R0, #$01                           ;if upper byte == 0 then it MUST be an empty index
    .SkipResetFlag:
    pop Y
    mov.b A, ZP.R0
    bne .SkipReset
    setp
    mov.b OP.ChannelSleepCounter+8+Y, A
    mov.b OP.ChannelVolume+$10+X, A
    mov.b OP.ChannelVolume+$11+X, A
    clrp
    mov.b ZP.SineIndexVib+8+Y, A
    mov.b ZP.SineIndexPanbr+8+Y, A
    mov.b ZP.SineIndexTrem+8+Y, A
    mov.b ZP.ArpValue+8+Y, A
    mov.b ZP.ArpTimer+8+Y, A
    mov.b ZP.VolSlideValue+8+Y, A
    mov.b ZP.PortValue+8+Y, A
    mov.b ZP.VibratoValue+8+Y, A
    mov.b ZP.TremolandoValue+8+Y, A
    mov.b ZP.PanbrelloValue+8+Y, A
    mov.b ZP.VCTickThresh+Y, A
    .SkipReset:
    dec.b X
    dec.b X
    dec.b Y
    bpl -
    bra .ExitSub
    +
    ;Music
    setp
    mov A, #$00
    mov.b OP.OrderChangeFlag, #$01
    mov.b OP.StopFlag, #$00
    mov.b X, #$0E
    -
    mov.b OP.ChannelVolume+X, A
    mov.b OP.ChannelVolume+1+X, A
    mov.b OP.ChannelSleepCounter+Y, A
    clrp
    mov.b ZP.SineIndexVib+Y, A
    mov.b ZP.SineIndexPanbr+Y, A
    mov.b ZP.SineIndexTrem+Y, A
    mov.b ZP.ArpValue+Y, A
    mov.b ZP.ArpTimer+Y, A
    mov.b ZP.VolSlideValue+Y, A
    mov.b ZP.PortValue+Y, A
    mov.b ZP.VibratoValue+Y, A
    mov.b ZP.TremolandoValue+Y, A
    mov.b ZP.PanbrelloValue+Y, A
    mov.b ZP.VCTickThresh+Y, A
    setp
    dec.b X
    dec.b X
    dec.b Y
    bpl -
    .ExitSub:
    clrp
    ;Increment recieve flag
    inc.b ZP.SFXRec
    mov.b Apu1, ZP.SFXRec
    ;Skip SFX if Apu0 == 0
    .SkipSFXCheck:
    pop Y
    pop X
    pop A
    ret

CheckProgrammerControls:
    push X
    mov.b A, Apu1                         ;Check SEND byte
    cmp.b A, ZP.SFXRec
    bne ReturnProCom_SkipPC               ;if SFXRec != SFXRec before then new a new subtune will be selected
    inc.b ZP.FlagVal
    cmp.b Apu2, #ProCom.PlayMusic
    beq ReturnProCom_SkipPC
    cmp.b Apu2, #ProCom.PlaySfx
    beq ReturnProCom_SkipPC
    mov.b A, Apu2
    setc
    sbc.b A, #ProCom.SetMasterVol
    asl A
    mov X, A
    jmp (ComTable+X)
ReturnProCom:
    ;Increment recieve flag
    inc.b ZP.SFXRec
    mov.b Apu1, ZP.SFXRec
    .SkipPC:
    pop X
    ret

ComTable:
    dw Com_MasterVol
    dw Com_Settings
    dw Com_Div
    dw Com_Mute
    dw Com_Pause
    dw Com_FadeAud
    dw Com_FadeTarget
    dw Com_FadeSpeed
    dw Com_Reset

Com_MasterVol:
    mov.b ZP.OutVol, Apu0
    bra ReturnProCom
Com_Settings:
    mov.b ZP.TrackSettings, Apu0
    bra ReturnProCom
Com_Div:
    mov.b SPC_Timer1, Apu0
    bra ReturnProCom
Com_Mute:
    mov.b A, Apu0
    mov.b ZP.ChannelMask, A
    bra ReturnProCom
Com_Pause:
    setp
    eor.b OP.StopFlag, #$01
    clrp
    bra ReturnProCom
Com_FadeAud:
    mov.b ZP.FadeFlag, #$01
    bra ReturnProCom
Com_FadeTarget:
    mov.b A, Apu0
    setp
    mov.b OP.MaxVolTarget, A
    clrp
    bra ReturnProCom
Com_FadeSpeed:
    mov.b ZP.FadeSpeed, Apu0
    bra ReturnProCom
Com_Reset:
    mov.b SPC_Control, #$81     ;Get IPL rom back
    mov.b Apu3, #$01            ;Send reset flag to 65C816
    jmp $FFC0                   ;Go to IPL ROM

BitmaskTable:   ;General bitmask table
    db $01
    db $02
    db $04
    db $08
    db $10
    db $20
    db $40
    db $80

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

;Start of dynamic data

DirTable:                          ;These are just for debugging purposes
;Test sine+saw sample + dir page
db $08,$0C,$08,$0C,$23,$0C,$23,$0C

SampleTable:                        ;These are just for debugging purposes
db $84, $17, $45, $35, $22, $22, $31, $21, $10, $68, $01, $21, $0D, $01, $08, $0B, $C3, $3E, $5B, $09, $8B, $D7
db $B1, $E0, $BC, $AF, $78
db $B8, $87, $1F, $00, $F1, $0F, $1F, $00, $00, $8F, $E1, $13, $12, $2D, $52, $14, $10, $F7

PitchTable:
    for t = 0..224
        dw $0000+(!t*$0101)
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
    %SetSpeed($06)
    %SetChannelVolume($40, $40)
    %SetInstrument($01)
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
    %SetDelayCoefficient(0, $FF/16)
    %SetDelayCoefficient(1, $EE/16)
    %SetDelayCoefficient(2, $DD/16)
    %SetDelayCoefficient(3, $CC/16)
    %SetDelayCoefficient(4, $BB/16)
    %SetDelayCoefficient(5, $AA/16)
    %SetDelayCoefficient(6, $99/16)
    %SetDelayCoefficient(7, $88/16)
    %SetDelayTime($04)
    %SetDelayVolume($60)
    %SetDelayFeedback($40)
    %Sleep($FF)
    .Pat2:
    %SetVib($62)
    %Sleep($04)
    %Goto(OrderTable_Tune0)
    .Pat3:
    %SetVib($00)
    %SetMasterVolume($7F)
    %SetChannelVolume($60, $60)
    %SetInstrument($01)
    %SetSpeed($08)
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
    %Break()
    ;%Goto(OrderTable_Tune0)

SfxTable:
    .SFX_Null:
        dw $0000
        dw $0000
        dw $0000
        dw $0000
        dw $0000
        dw $0000
        dw $0000
        dw $0000
    .SFX_1:
        dw $0000
        dw $0000
        dw $0000
        dw $0000
        dw $0000
        dw SfxPat_Sfx1_0
        dw SfxPat_Sfx1_1
        dw $0000
    .SFX_2:
        dw $0000
        dw $0000
        dw $0000
        dw $0000
        dw $0000
        dw $0000
        dw $0000
        dw SfxPat_Sfx2_0

SfxPat:
    .Null:
    %SetChannelVolume($00, $00)
    %SetSpeed($01)
    %Sleep($01)
    %Stop()
    .Sfx1_0:
    %SetSpeed($04)
    %SetChannelVolume($7F, $7F)
    %SetInstrument($01)
    %PlayPitch($1800)
    %Sleep($0A)
    %SetChannelVolume($66, $66)
    %PlayPitch($2000)
    %Sleep($0A)
    %SetChannelVolume($44, $44)
    %Sleep($0A)
    %Stop()
    .Sfx1_1:
    %SetSpeed($02)
    %SetChannelVolume($99, $99)
    %SetInstrument($02)
    %PlayNote($05)
    %Sleep($10)
    %PlayNote($05)
    %Sleep($10)
    %Stop()
    .Sfx2_0:
    %SetChannelVolume($5F, $5F)
    %SetInstrument($04)
    %SetSpeed($08)
    %PlayNote($20)
    %Sleep($0E)
    %PlayNote($21)
    %SetPort($80)
    %Sleep($12)
    %Stop()
    .Sfx2_1:
    %SetChannelVolume($7F, $7F)
    %SetInstrument($04)
    %SetPort($7F)
    %SetSpeed($08)
    %PlayNote($1F)
    %Sleep($0E)
    %PlayNote($21)
    %Sleep($12)

    ;List of available music tracks, holds the tune's starting order address
SubtuneList:
    dw OrderTable_Tune0

    ;List of available sound effects, holds the tune's starting order address
SFXList:
    dw SfxTable_SFX_1
    dw SfxTable_SFX_2

InstrumentMemory:
    %WriteInstrument($00, $00, $00, $00, $00)
    %WriteInstrument($01, $FF, $70, $7F, $00)
    %WriteInstrument($01, $FF, $80, $7F, $00)
    %WriteInstrument($01, $FF, $80, $7F, $00)
    %WriteInstrument($00, $FF, $70, $7F, $00)  ;Test SFX

Engine_End:

arch 65816
base ROM_Engine_Start+(Engine_End-DriverStart)
ROM_Engine_End: