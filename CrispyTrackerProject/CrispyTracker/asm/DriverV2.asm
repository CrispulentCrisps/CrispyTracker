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

base $0200              ;Set audio driver code to 0x0200 [closest to the start of memory we can get away with]

DriverStart:            ;Start of the driver
mov COM_TempMemADDRL, #Engine_End&$FF
mov COM_TempMemADDRH, #Engine_End>>8
mov X, #0
mov A, #0
.MemClearLoop:
db $C7, $00                     ;Equivelant to mov (COM_TempMemADDRL+X), A; reason is the ASAR doesn't put this line in code, instead putting in C5 00 00
incw COM_TempMemADDRL
bne .MemClearLoop

mov X, #$FF
mov SP, X
mov X, #0
mov COM_TrackSettings, #$00     ;Set the track settings

%spc_write(DSP_FLG, $00)
%spc_write(DSP_MVOL_L, $7F)
%spc_write(DSP_MVOL_R, $7F)
%spc_write(DSP_EVOL_L, $40)
%spc_write(DSP_EVOL_R, $40)
%spc_write(DSP_ESA, $BF)
%spc_write(DSP_EDL, $00)
%spc_write(DSP_EFB, $20)

%spc_write(DSP_DIR, $20)

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

.TickRoutine:               ;Routine for incrementing the tick counter and postiion within the track
    mov X, #0               ;Reset counter
    
    .TickIncrement:
    mov Y, SPC_Count1       ;Check counter
    beq .TickIncrement      ;If the timer is set to 0
                            ;Assuming the timer is 1
    ;jmp .EffectsProcess     ;Process effects
    .ContinueIncrement:
    inc X                   ;Increment our counter
    cmp X, COM_TickThresh   ;Check if the counter has reached the 
    ;cmp X, #$31             ;Check if the counter has reached the threshold
    bmi .TickIncrement      ;Go back to the tick incrementer if the counter is not
                            ;Assuming we've hit the threshold
    mov X, #00              ;Reset counter
    jmp .ReadRows

fill $2000-pc()
assert pc() == $2000

;Test sine+saw sample + dir page
db $08,$20,$08,$20,$22,$20,$22,$20
db $84, $17, $45, $35, $22, $22, $31, $21, $10, $68, $01, $21, $0D, $01, $08, $0B, $C3, $3E, $5B, $09, $8B, $D7, $B1, $E0, $BC, $AF, $78
db $B8, $87, $1F, $00, $F1, $0F, $1F, $00, $00, $8F, $E1, $13, $12, $2D, $52, $14, $10, $F7
ChannelTable:   ;Writes the value for the channel bitfield
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

InstrumentMemory:
%WriteInstrument($7F, $7F, $FF, $80, $7F, %00000000, $01, $40)
%WriteInstrument($7F, $80, $FF, $80, $7F, %00000000, $01, $60)
%WriteInstrument($80, $80, $FF, $80, $7F, %00000000, $01, $80)
.EndOfInstrument:

Engine_End:

arch 65816
base ROM_Engine_Start+(Engine_End-DriverStart)
ROM_Engine_End: