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
mov X, #0
mov ZP.TrackSettings, #$00     ;Set the track settings

%spc_write(DSP_FLG, $00)
%spc_write(DSP_MVOL_L, $11)
%spc_write(DSP_MVOL_R, $11)
%spc_write(DSP_EVOL_L, $40)
%spc_write(DSP_EVOL_R, $40)
%spc_write(DSP_ESA, $BF)
%spc_write(DSP_EDL, $00)
%spc_write(DSP_EFB, $20)
%spc_write(DSP_DIR, $20)

;Audio register reset routine
mov A, #0
-
mov Y, #0                           ;Shove into reset value
mov X, #7
mov SPC_RegADDR, A                  ;Shove address in
Inner:
mov SPC_RegData, Y                  ;Reset volume left
inc SPC_RegADDR                     ;Increment address value
dec X                               ;Decrement the loop counter
bne Inner
clrc                                ;Clear carry
adc A, #$10                         ;Add 16
bvc -

mov SPC_Control, #$01               ;Set control bit to enable Timer 0
mov SPC_Timer1, #$20                ;Divide timer to run at ~250hz
mov ZP.OrderChangeFlag, #$01       ;Set change flag at start to load initial pattern data

DriverLoop:                         ;Main driver loop
    mov X, #0                       ;Reset counter
    
    .TickIncrement:
    mov Y, SPC_Count1               ;Check counter
    beq .TickIncrement              ;If the timer is set to 0
    
    call ProcessEffects
    inc X                           ;Increment our counter
    cmp X, ZP.TickThresh            ;Check if the counter has reached the 
    bmi .TickIncrement              ;Go back to the tick incrementer if the counter is not
                                    ;Assuming we've hit the threshold
    mov1 C, ZP.OrderChangeFlag.0    ;Check the order flag with the 0th bit
    bcc +                           ;Skip if the carry flag isn't set
    mov X, #$07
    mov Y, #$00
    -                                   ;Sleep Clear Loop
    mov ZP.ChannelSleepCounter+X, Y     ;Clear sleep counter
    dec X                               ;Decrement counter
    bpl -
    call ReadPatterns
    mov ZP.OrderChangeFlag, #0
    +

    mov ZP.CurrentChannel,#$7       ;Increment channel index
    .ChannelLoop:                   ;Main channel loop
    mov X, ZP.CurrentChannel
    mov Y, ZP.ChannelSleepCounter+X
    bne .SkipRow                    ;Check if the sleep counter != 0
    call ReadRows
    bra .SkipDec
    .SkipRow:                       ;Sleep counter routine
    dec ZP.ChannelSleepCounter+X
    .SkipDec:
    dec ZP.CurrentChannel
    bpl .ChannelLoop

    cmp ZP.OrderChangeFlag, #1      ;Order change
    bne +
    mov A, ZP.OrderPosGoto
    mov ZP.OrderPos, A
    +
    jmp DriverLoop


ProcessEffects:
    push X
    mov ZP.CurrentChannel, #$07
    .EffectsLoop:
    mov ZP.TempPitchProcess, #0
    mov ZP.TempPitchProcess+1, #0
    mov ZP.TempVolumeProcess, #0
    mov ZP.TempVolumeProcess+1, #0
    mov A, ZP.CurrentChannel        ;Grab channel index
    asl A                           ;Double since we are working with 2 bytes
    mov ZP.TempScratchMemH, A       ;Return

    ;---------------;
    ;   Portamento  ;
    ;---------------;
    mov X, ZP.CurrentChannel
    mov A, ZP.PortValue+X
    beq .SkipPort
    mov ZP.TempMemADDRL, A
    mov ZP.TempMemADDRH, #0
    mov1 C, ZP.TempMemADDRL.7
    bcc +
    mov ZP.TempMemADDRH, #$FF
    +
    mov X, ZP.TempScratchMemH
    mov Y, ZP.ChannelPitches+1+X
    mov A, ZP.ChannelPitches+X
    addw YA, ZP.TempMemADDRL
    mov ZP.ChannelPitches+1+X, Y
    mov ZP.ChannelPitches+X, A  
    .SkipPort:
    
    ;--------------------;
    ;       Vibrato      ;
    ;--------------------;
    mov X, ZP.CurrentChannel
    mov A, ZP.VibratoValue+X
    beq .SkipVib
    and A, #$F0
    xcn A
    asl A
    mov ZP.TempScratchMem, A    ;Hold speed in temporary memory
    mov A, ZP.SineIndex+X       ;Grab sine index
    adc A, ZP.TempScratchMem    ;Increment sine index by speed value
    mov ZP.SineIndex+X, A
    mov Y, A                    ;Grab sine index value
    mov A, ZP.VibratoValue+X
    and A, #$0F                 ;Grab depth
    lsr A
    call GetSineValue           ;Call sine function
    call SignedMul              ;multiply
    mov ZP.R0, A
    mov ZP.R1, Y
    mov A, ZP.TempPitchProcess
    mov Y, ZP.TempPitchProcess+1
    addw YA, ZP.R0
    movw ZP.TempPitchProcess, YA
    .SkipVib:
    
    ;----------------;
    ;   Tremolando   ;
    ;----------------;
    mov X, ZP.CurrentChannel
    mov A, ZP.TremolandoValue+X
    beq .SkipTrem
    and A, #$F0
    xcn A
    mov ZP.TempScratchMem, A    ;Hold speed in temporary memory
    mov A, ZP.SineIndex+X       ;Grab sine index
    adc A, ZP.TempScratchMem    ;Increment sine index by speed value
    mov ZP.SineIndex+X, A
    mov Y, A                    ;Grab sine index value
    mov A, ZP.TremolandoValue+X
    and A, #$0F                 ;Grab depth
    call GetSineValue           ;Call sine function
    call SignedMul              ;multiply
    mov X, #$80
    div YA, X
    mov ZP.R0, A
    mov ZP.R1, A
    mov A, ZP.TempVolumeProcess
    mov Y, ZP.TempVolumeProcess+1
    addw YA, ZP.R0
    mov ZP.TempVolumeProcess, A
    mov ZP.TempVolumeProcess+1, A
    .SkipTrem:
    
    ;---------------------------;
    ;       Mixing & Output     ;
    ;---------------------------;
    ;Grab channel memory
    mov ZP.TempMemADDRL, #(InstrumentMemory)&$FF        ;Create word addr to instrument memory
    mov ZP.TempMemADDRH, #(InstrumentMemory>>8)&$FF     ;
    mov X, ZP.CurrentChannel                            ;Shove channel index into X
    mov A, ZP.ChannelInstrumentIndex+X                  ;Grab instrument index
    mov Y, #$08                                         ;Shove in multiplier
    mul YA                                              ;Multiply
    addw YA, ZP.TempMemADDRL                            ;Add on instrument memory location
    movw ZP.TempMemADDRL, YA                            ;Return

    ;Mix L
    mov X, ZP.TempScratchMemH                           ;Grab premul index
    mov Y, #5                                           ;Reset X
    mov A, (ZP.TempMemADDRL)+Y                          ;Grab instrument L volume
    mov Y, ZP.ChannelVolume+X                           ;Shove L volume into X
    call SignedMul                                      ;Multiply both volumes together
    mov X, #128                                         ;Shove in Divispr
    div YA, X                                           ;Divide
    adc A, ZP.TempVolumeProcess
    mov ZP.ChannelVolumeOutput, A                       ;Shove into volume output
    
    ;Mix R
    mov X, ZP.TempScratchMemH
    mov Y, #6                                           ;Reset X
    mov A, (ZP.TempMemADDRL)+Y                          ;Grab instrument R volume
    mov Y, ZP.ChannelVolume+1+X                         ;Shove R volume into X
    call SignedMul                                      ;Multiply both volumes together
    mov X, #128                                         ;Shove in Divispr
    div YA, X                                           ;Divide
    adc A, ZP.TempVolumeProcess+1
    mov ZP.ChannelVolumeOutput+1, A                     ;Shove into volume output

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
    mov A, ZP.ChannelPitches+X                          ;Grab current channel's lo pitch
    adc A, ZP.TempPitchProcess                          ;Add lo byte to pitch offset
    mov ZP.ChannelPitchesOutput, A                      ;Put into output
    mov A, ZP.ChannelPitches+1+X                        ;Grab current channel's hi pitch
    adc A, ZP.TempPitchProcess+1                        ;Add hi byte to pitch offset
    mov ZP.ChannelPitchesOutput+1, A                    ;Put into output

    mov A, ZP.ChannelPitchesOutput                      ;Grab lo pitch
    mov SPC_RegData, A                                  ;Return
    inc SPC_RegADDR                                     ;Increment address
    mov A, ZP.ChannelPitchesOutput+1                    ;Grab hi pitch
    mov SPC_RegData, A                                  ;Return

    dec ZP.CurrentChannel
    bmi .BreakLoop
    jmp .EffectsLoop
    .BreakLoop:
    pop X
    ret

ReadPatterns:
    mov A, ZP.OrderPos                         ;Grab the current order position
    xcn A                                      ;Mult by 16
    mov ZP.TempMemADDRH, A                     ;Shove into hi zp
    mov ZP.TempMemADDRL, A                     ;Shove into lo zp
    and ZP.TempMemADDRH, #$0F                  ;Get lo nibble
    and ZP.TempMemADDRL, #$F0                  ;Get hi nibble
    mov A, #OrderTable&$FF                     ;Shove lo table addr into A
    mov Y, #(OrderTable>>8)&$FF                ;Shove hi table addr into Y
    addw YA, ZP.TempMemADDRL                   ;Add offset to YA
    movw ZP.TempMemADDRL, YA                   ;Return address to memory
    mov Y, #$F                                 ;Set Y up for loop
    -                                          ;Loop point
    mov A, (ZP.TempMemADDRL)+Y                 ;Indirectly shove addr value into A
    mov ZP.SequenceAddr+Y, A                   ;Copy value to sequence addr
    dec Y                                      ;Decrement loop counter
    bpl -                                      ;Loop

    ret

ReadRows:
    call GrabCommand
    asl A                               ;Mult by 2 to prevent reading in the wrong address byte
    mov X, A                            ;Shove into X for the jump table
    jmp (.RowJumpTable+X)               ;Goto jumptable + command index

.RowJumpTable:
    dw Row_SetSpeed
    dw Row_Sleep
    dw Row_Goto
    dw Row_Break
    dw Row_PlayNote
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
    dw Row_SetMasterVolume
    dw Row_SetChannelVolume
    dw Row_SetArp
    dw Row_SetPort
    dw Row_SetVib
    dw Row_SetTrem
    dw Row_SetVolSlide
    dw Row_SetPanbr
    dw Row_Stop
    
Row_SetSpeed:
    call GrabCommand
    mov ZP.TickThresh, A
    jmp ReadRows

Row_Sleep:
    call GrabCommand
    mov X, ZP.CurrentChannel
    mov ZP.ChannelSleepCounter+X, A
    ret                             ;ret is exiting out of the read rows subroutine

Row_Goto:   ;Broken, causes the command addr to get fucked up and read rubbish data
    call GrabCommand
    mov ZP.OrderPosGoto, A      ;Return pos
    mov ZP.OrderChangeFlag, #1  ;Set the order change flag
    ret

Row_Break:
    mov ZP.OrderPosGoto, ZP.OrderPos
    inc ZP.OrderPosGoto
    mov ZP.OrderChangeFlag, #1  ;Set the order change flag
    ret                         ;Break out of read rows

Row_PlayNote:

    jmp ReadRows

Row_PlayPitch:
    ;Pitch application
    call GrabCommand                ;Grab lo byte of the pitch
    mov ZP.ChannelPitches+X, A      ;Shove lo byte into pitch
    call GrabCommand                ;Grab hi byte of the pitch
    mov ZP.ChannelPitches+1+X, A    ;Shove hi byte into pitch

    ;KON State
    mov X, ZP.CurrentChannel            ;Grab current channel
    mov A, BitmaskTable+X               ;Get bitmask index via X
    mov ZP.TempScratchMem, A            ;Shove into scratch memory for ORing
    mov SPC_RegADDR, #DSP_KON           ;Shove KON addr in
    or SPC_RegData, ZP.TempScratchMem   ;OR into A
    jmp ReadRows

Row_SetInstrument:
    mov ZP.TempMemADDRL, #(InstrumentMemory)&$FF
    mov ZP.TempMemADDRH, #(InstrumentMemory>>8)&$FF
    call GrabCommand
    mov X, ZP.CurrentChannel
    mov ZP.ChannelInstrumentIndex+X, A
    mov Y, #$08
    mul YA
    addw YA, ZP.TempMemADDRL
    movw ZP.TempMemADDRL, YA

    mov A, ZP.CurrentChannel            ;Grab current channel
    xcn A                               ;XCN to get correct channel in memory
    or A, #$04                          ;Add 4 for correct nibble
    mov SPC_RegADDR, A                  ;SCRN
    mov X, #0                           ;Reset X
    mov A, (ZP.TempMemADDRL+X)          ;Shove value into A
    mov SPC_RegData, A                  ;Apply
    
    incw ZP.TempMemADDRL                ;Increment
    mov A, (ZP.TempMemADDRL+X)          ;Grab Value
    inc SPC_RegADDR                     ;Inc address
    mov SPC_RegData, A                  ;Apply
    
    incw ZP.TempMemADDRL                ;Increment
    mov A, (ZP.TempMemADDRL+X)          ;Grab Value
    inc SPC_RegADDR                     ;Inc address
    mov SPC_RegData, A                  ;Apply
    
    incw ZP.TempMemADDRL                ;Increment
    mov A, (ZP.TempMemADDRL+X)          ;Grab Value
    inc SPC_RegADDR                     ;Inc address
    mov SPC_RegData, A                  ;Apply

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
    call GrabCommand            ;Grab value
    mov SPC_RegADDR, #DSP_FLG   ;Get correct addr
    mov SPC_RegData, A          ;Apply
    jmp ReadRows

Row_SetDelay:

    jmp ReadRows

Row_SetDelayVolume:

    jmp ReadRows

Row_SetDelayFeedback:

    jmp ReadRows

Row_SetDelayCoeff:

    jmp ReadRows

Row_SetMasterVolume:
    call GrabCommand            ;Grab L volume
    mov SPC_RegADDR, #DSP_MVOL_L;Shove correct addr to get L master volume
    mov SPC_RegData, A          ;Apply
    call GrabCommand            ;Grab R volume
    mov SPC_RegADDR, #DSP_MVOL_R;Shove correct addr to get R master volume
    mov SPC_RegData, A          ;Apply
    jmp ReadRows

Row_SetChannelVolume:
    call GrabCommand                ;Grab L volume
    mov ZP.ChannelVolume+X, A       ;Apply
    call GrabCommand                ;Grab R volume
    mov ZP.ChannelVolume+1+X, A     ;Apply
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
Row_Stop:
    ret

        ;----------------------------;
        ;       Command Grabber      ;
        ;----------------------------;
        ;
        ;   Input:
        ;       ZP.CurrentChannel
        ;
        ;   Output:
        ;       A: Netx byte from command list
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
    mov A, #64                      ;Shove subtraction into A
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
    db $00
    db $03
    db $06
    db $09
    db $0C
    db $10
    db $13
    db $16
    db $19
    db $1C
    db $1F
    db $22
    db $25
    db $28
    db $2B
    db $2E
    db $31
    db $33
    db $36
    db $39
    db $3C
    db $3F
    db $41
    db $44
    db $47
    db $49
    db $4C
    db $4E
    db $51
    db $53
    db $55
    db $58
    db $5A
    db $5C
    db $5E
    db $60
    db $62
    db $64
    db $66
    db $68
    db $6A
    db $6B
    db $6D
    db $6F
    db $70
    db $71
    db $73
    db $74
    db $75
    db $76
    db $78
    db $79
    db $7A
    db $7A
    db $7B
    db $7C
    db $7D
    db $7D
    db $7E
    db $7E
    db $7E
    db $7F
    db $7F
    db $7F

fill $2000-pc()
assert pc() == $2000

;Test sine+saw sample + dir page
db $08,$20,$08,$20,$22,$20,$22,$20
db $84, $17, $45, $35, $22, $22, $31, $21, $10, $68, $01, $21, $0D, $01, $08, $0B, $C3, $3E, $5B, $09, $8B, $D7, $B1, $E0, $BC, $AF, $78
db $B8, $87, $1F, $00, $F1, $0F, $1F, $00, $00, $8F, $E1, $13, $12, $2D, $52, $14, $10, $F7

OrderTable:
    .Ord0:
    dw PatternMemory_Pat0
    dw PatternMemory_Pat0
    dw PatternMemory_Pat0
    dw PatternMemory_Pat0
    dw PatternMemory_Pat0
    dw PatternMemory_Pat0
    dw PatternMemory_Pat0
    dw PatternMemory_Pat0
    .Ord1:
    dw PatternMemory_Pat1
    dw PatternMemory_Pat1
    dw PatternMemory_Pat1
    dw PatternMemory_Pat1
    dw PatternMemory_Pat1
    dw PatternMemory_Pat1
    dw PatternMemory_Pat1
    dw PatternMemory_Pat1

PatternMemory:
    .Pat0:
    %PlayPitch($1234)
    %SetSpeed($10)
    %SetTremo($1F)
    ;%SetPort($02)
    ;%SetVib($54)
    %SetMasterVolume($7F, $7F)
    %SetChannelVolume($7F, $7F)
    %SetInstrument(0)
    %Sleep($10)
    %Break()
    .Pat1:
    %Sleep($20)
    %Goto($00)
    .Pat2:
    %PlayPitch($1010)
    %Sleep($10)
    %Break()

InstrumentMemory:
%WriteInstrument($01, $FF, $80, $7F, %00000000, $7F, $7F, $40)
%WriteInstrument($01, $FF, $80, $7F, %00000000, $7F, $FF, $60)
%WriteInstrument($01, $FF, $80, $7F, %00000000, $FF, $FF, $80)
.EndOfInstrument:

Engine_End:

arch 65816
base ROM_Engine_Start+(Engine_End-DriverStart)
ROM_Engine_End: