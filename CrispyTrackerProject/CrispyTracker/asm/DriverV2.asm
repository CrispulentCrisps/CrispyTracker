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
%spc_write(DSP_MVOL_L, $7F)
%spc_write(DSP_MVOL_R, $7F)
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
mov SPC_Timer1, #$FF                ;Divide timer to run at ~31hz
mov ZP.OrderChangeFlag, #$01       ;Set change flag at start to load initial pattern data

DriverLoop:                         ;Main driver loop
    mov X, #0                       ;Reset counter
    mov ZP.CurrentChannel,#$7       ;Increment channel index
    
    .TickIncrement:
    mov Y, SPC_Count1               ;Check counter
    beq .TickIncrement              ;If the timer is set to 0
    
    inc X                           ;Increment our counter
    cmp X, ZP.TickThresh            ;Check if the counter has reached the 
    bmi .TickIncrement              ;Go back to the tick incrementer if the counter is not
                                    ;Assuming we've hit the threshold
    mov1 C, ZP.OrderChangeFlag.0    ;Check the order flag with the 0th bit
    bcc +                           ;Skip if the carry flag isn't set
    dec X
    call ReadPatterns
    +

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

    cmp ZP.OrderChangeFlag, #1
    bne +
    inc ZP.OrderPos
    mov ZP.OrderChangeFlag, #0
    +
    jmp DriverLoop


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
    mov ZP.OrderPos, A          ;Return pos
    mov ZP.OrderChangeFlag, #0  ;Set the order change flag
    ret

Row_Break:
    mov ZP.OrderChangeFlag, #1  ;Set the order change flag
    ret                         ;Break out of read rows

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
    %SetSpeed($04)
    %Sleep($10)
    %Break()
    .Pat1:
    %SetSpeed($10)
    %Sleep($20)

InstrumentMemory:
%WriteInstrument($7F, $7F, $FF, $80, $7F, %00000000, $01, $40)
%WriteInstrument($7F, $80, $FF, $80, $7F, %00000000, $01, $60)
%WriteInstrument($80, $80, $FF, $80, $7F, %00000000, $01, $80)
.EndOfInstrument:

Engine_End:

arch 65816
base ROM_Engine_Start+(Engine_End-DriverStart)
ROM_Engine_End: