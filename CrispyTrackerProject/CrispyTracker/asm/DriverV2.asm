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
    mov ZP.CurrentChannel,#$0      ;Increment channel index
    
    .TickIncrement:
    mov Y, SPC_Count1               ;Check counter
    beq .TickIncrement              ;If the timer is set to 0
    
    inc X                           ;Increment our counter
    cmp X, ZP.TickThresh           ;Check if the counter has reached the 
    bmi .TickIncrement              ;Go back to the tick incrementer if the counter is not
                                    ;Assuming we've hit the threshold
    mov1 C, ZP.OrderChangeFlag.0   ;Check the order flag with the 0th bit
    bcc +                           ;Skip if the carry flag isn't set
    dec X
    call ReadPatterns
    +
    call ReadRows
    jmp DriverLoop


ReadPatterns:
    mov A, ZP.OrderPos                         ;Grab the current order position
    xcn A                                       ;Mult by 16
    mov ZP.TempMemADDRH, A                     ;Shove into hi zp
    mov ZP.TempMemADDRL, A                     ;Shove into lo zp
    and ZP.TempMemADDRH, #$0F                  ;Get lo nibble
    and ZP.TempMemADDRL, #$F0                  ;Get hi nibble
    mov A, #OrderTable&$FF                      ;Shove lo table addr into A
    mov Y, #(OrderTable>>8)&$FF                 ;Shove hi table addr into Y
    addw YA, ZP.TempMemADDRL                   ;Add offset to YA
    movw ZP.TempMemADDRL, YA                   ;Return address to memory
    mov Y, #$F                                  ;Set Y up for loop
    -                                           ;Loop point
    mov A, (ZP.TempMemADDRL)+Y                 ;Indirectly shove addr value into A
    mov ZP.SequenceAddr+Y, A                   ;Copy value to sequence addr
    dec Y                                       ;Decrement loop counter
    bpl -                                       ;Loop

    ret

ReadRows:

    ret
.RowJumpTable:

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
    .Ord1:
    dw PatternMemory_Pat0
    dw PatternMemory_Pat1
    dw PatternMemory_Pat2
    dw PatternMemory_Pat3
    dw PatternMemory_Pat4
    dw PatternMemory_Pat5
    dw PatternMemory_Pat6
    dw PatternMemory_Pat7

PatternMemory:
    .Pat0:
    db $FF
    db $00
    .Pat1:
    db $FF
    db $00
    .Pat2:
    db $FF
    db $00
    .Pat3:
    db $FF
    db $00
    .Pat4:
    db $FF
    db $00
    .Pat5:
    db $FF
    db $00
    .Pat6:
    db $FF
    db $00
    .Pat7:
    db $FF
    db $00

InstrumentMemory:
%WriteInstrument($7F, $7F, $FF, $80, $7F, %00000000, $01, $40)
%WriteInstrument($7F, $80, $FF, $80, $7F, %00000000, $01, $60)
%WriteInstrument($80, $80, $FF, $80, $7F, %00000000, $01, $80)
.EndOfInstrument:

Engine_End:

arch 65816
base ROM_Engine_Start+(Engine_End-DriverStart)
ROM_Engine_End: