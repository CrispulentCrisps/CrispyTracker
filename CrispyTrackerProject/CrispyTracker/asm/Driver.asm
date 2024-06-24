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

;
;   General Memory Layout
;   0200    Audio driver code section
;   

incsrc "Macros.asm"
incsrc "spc700.asm"
incsrc "dsp.asm"

arch spc700     ;Set architecture to SPC-700

org $C10000     ;Go to bank C1000

ROM_Engine_Start:

base $0200              ;Set audio driver code to 0x0200 [closest to the start of memory we can get away with]

DriverStart:            ;Start of the driver
%spc_write(DSP_FLG, $20)
%spc_write(DSP_MVOL_L, $7F)
%spc_write(DSP_MVOL_R, $7F)
%spc_write(DSP_EVOL_L, $40)
%spc_write(DSP_EVOL_R, $40)
%spc_write(DSP_ESA, $BF)
%spc_write(DSP_EDL, $00)
%spc_write(DSP_EFB, $60)

%spc_write(DSP_DIR, $05)

%spc_write(DSP_V0_GAIN, $7F)
%spc_write(DSP_V0_ADSR1, $8F)
%spc_write(DSP_V0_ADSR2, $F0)
%spc_write(DSP_V0_VOL_L, $20)
%spc_write(DSP_V0_VOL_R, $20)
%spc_write(DSP_V0_PITCHL, $00)
%spc_write(DSP_V0_PITCHH, $00)
%spc_write(DSP_V0_SCRN, $00)

%spc_write(DSP_V1_GAIN, $7F)
%spc_write(DSP_V1_ADSR1, $8F)
%spc_write(DSP_V1_ADSR2, $F0)
%spc_write(DSP_V1_VOL_L, $20)
%spc_write(DSP_V1_VOL_R, $20)
%spc_write(DSP_V1_PITCHL, $00)
%spc_write(DSP_V1_PITCHH, $00)
%spc_write(DSP_V1_SCRN, $00)

%spc_write(DSP_V2_GAIN, $7F)
%spc_write(DSP_V2_ADSR1, $8F)
%spc_write(DSP_V2_ADSR2, $F0)
%spc_write(DSP_V2_VOL_L, $20)
%spc_write(DSP_V2_VOL_R, $20)
%spc_write(DSP_V2_PITCHL, $00)
%spc_write(DSP_V2_PITCHH, $00)
%spc_write(DSP_V2_SCRN, $00)

%spc_write(DSP_V3_GAIN, $7F)
%spc_write(DSP_V3_ADSR1, $8F)
%spc_write(DSP_V3_ADSR2, $F0)
%spc_write(DSP_V3_VOL_L, $20)
%spc_write(DSP_V3_VOL_R, $20)
%spc_write(DSP_V3_PITCHL, $00)
%spc_write(DSP_V3_PITCHH, $00)
%spc_write(DSP_V3_SCRN, $00)

%spc_write(DSP_NON, $00)
%spc_write(DSP_PMON, $00)
%spc_write(DSP_EON, $FF)
%spc_write(DSP_KOF, $00)
%spc_write(DSP_KON, $00)

%spc_write(DSP_C0, $00)
%spc_write(DSP_C1, $00)
%spc_write(DSP_C2, $00)
%spc_write(DSP_C3, $00)
%spc_write(DSP_C4, $00)
%spc_write(DSP_C5, $00)
%spc_write(DSP_C6, $00)
%spc_write(DSP_C7, $00)

mov SPC_Control, #$01   ;Set control bit to enable Timer 0
mov SPC_Timer1, #$FF    ;Divide timer to run at ~31hz
mov X, #00
mov COM_SequencePos, #SequenceMemory&$FF
mov COM_SequencePos+1, #(SequenceMemory>>8)&$FF
mov COM_FlagVal, #0

DriverLoop:         ;Main driver loop
    jmp .TickRoutine

.PositionReset:
    mov SPC_RegADDR, #DSP_FLG      ;Get to the flag register
    mov SPC_RegData, COM_FlagVal   ;Write value for flag val
    mov COM_SequencePos, #SequenceMemory&$FF
    mov COM_SequencePos+1, #(SequenceMemory>>8)&$FF
    jmp .TickRoutine

.ReadRows:
    mov X, #00
    mov A, (COM_SequencePos+X)  ;Grab the sequence position and store in A
    beq .PositionReset          ;Check if we have hit the End Of Row command
    
    incw COM_SequencePos        ;Increments the sequence pos pointer
    mov X, A                    ;Shove A into X
    and A, #$F                  ;Get lower 4 nibbles to A
    mov Y, A                    ;Shove lower nibbles into Y
    mov A, X                    ;Reset A to its previous state held in X
    and A, #$F0                 ;Get higher 4 nibbles to A
    mov X, A                    ;Shove higher 4 nibbles into X
                                
                                ;Find command type
    cmp X, #$10                 ;Compare the higher nibble to 10
    beq .NoteCommands           ;Compared and found X = 0
    cmp X, #$30                 ;Compare the higher nibble to 30
    beq .SpecialCommands        ;Compared and found X = 0

.NoteCommands:
    cmp Y, #$8                  ;Check if we're doing an absolute pitch or a note table
    bpl .PlayPitch              ;If Y < 8 we know it's an absolute pitch command

.SpecialCommands:
    cmp Y, #$0
    beq .SetNoise
    cmp Y, #$1
    beq .SetDelayTime
    cmp Y, #$2
    beq .SetDelayVol
    cmp Y, #$3
    beq .SetDelayFeedback
    cmp Y, #$4
    bpl .SetDelayCoeff

.PlayNote:

.PlayPitch:
    mov X, #0                       ;Reset X to 0 since we know the command type
    mov A, Y
    and A, #%11110111               ;Sub 8 from A
    mov Y, A
    xcn A
    or A, #DSP_V0_PITCHH
    mov Y, A
    
    mov A, (COM_SequencePos+X)      ;Grab the sequence position and store in A
    mov SPC_RegADDR, Y              ;Get to the Hi pitch
    incw COM_SequencePos            ;Increments the sequence pos pointer
    dec Y
    mov SPC_RegData, A
    mov A, (COM_SequencePos+X)      ;Grab the sequence position and store in A
    mov SPC_RegADDR, Y              ;Get to the Lo pitch
    mov SPC_RegData, A
    incw COM_SequencePos            ;Increments the sequence pos pointer

    mov A, Y                        ;Load Y into A
    xcn A
    mov X, A                        ;Shove channel index into X
    mov A, ChannelTable+X
    or A, COM_KONState
    mov COM_KONState, A
    
    mov SPC_RegADDR, #DSP_KON
    mov SPC_RegData, A

    jmp .ReadRows

.SetNoise:
    mov X, #0                       ;Reset X to 0 since we know the command type
    mov A, (COM_SequencePos+X)      ;Grab position of counter
    incw COM_SequencePos            ;Increments the sequence pos pointer
    and COM_FlagVal, #~$1F         ;Clear bottom 5 bits
    or A, COM_FlagVal              ;Sets the COM_FlagVal to the applied value
    mov COM_FlagVal, A             ;Write into value location in zeropage
    jmp .ReadRows

.SetDelayTime:
    mov X, #0                       ;Reset X to 0 since we know the command type
    mov A, (COM_SequencePos+X)      ;Grab position of counter
    incw COM_SequencePos            ;Increments the sequence pos pointer
    mov SPC_RegADDR, #DSP_EDL
    mov SPC_RegData, A
    jmp .ReadRows
    
.SetDelayVol:
    mov X, #0                       ;Reset X to 0 since we know the command type
    mov A, (COM_SequencePos+X)      ;Grab position of counter
    incw COM_SequencePos            ;Increments the sequence pos pointer
    mov SPC_RegADDR, #DSP_EVOL_L
    mov SPC_RegData, A
    mov A, (COM_SequencePos+X)      ;Grab position of counter
    incw COM_SequencePos            ;Increments the sequence pos pointer
    mov SPC_RegADDR, #DSP_EVOL_R
    mov SPC_RegData, A
    jmp .ReadRows

.SetDelayFeedback:
    mov X, #0                       ;Reset X to 0 since we know the command type
    mov A, (COM_SequencePos+X)      ;Grab position of counter
    incw COM_SequencePos            ;Increments the sequence pos pointer
    mov SPC_RegADDR, #DSP_EFB
    mov SPC_RegData, A
    jmp .ReadRows

.SetDelayCoeff:
    mov X, #0                       ;Reset X to 0 since we know the command type
    mov A, (COM_SequencePos+X)      ;Grab position of counter
    incw COM_SequencePos            ;Increments the sequence pos pointer
    mov X, A                        ;Store the position value into X
    mov A, Y                        ;Shove the coffecient position into A
    dec A                           ;Sub 4 from A, now we have the exact coeffecient index
    dec A
    dec A
    dec A
    mov Y, A                        ;Store back into Y
    mov A, CoeffecientTable+Y       ;Grab the register addr from the addr table offset by Y
    mov SPC_RegADDR, A              ;Shove register addr in from A
    mov SPC_RegData, X              ;Grab value stored in X and shove into regdata
    jmp .ReadRows

.TickRoutine:               ;Routine for incrementing the tick counter and postiion within the track
    mov X, #00              ;Reset counter
    
    .TickIncrement
    mov A, SPC_Count1       ;Check counter
    beq .TickIncrement      ;If the timer is set to 0
                            ;Assuming the timer is 1
    inc X                   ;Increment our counter
    cmp X, #31              ;Check if the counter has reached the threshold
    bmi .TickIncrement      ;Go back to the tick incrementer if the counter is not 
                            ;Assuming we've hit the threshold
    mov X, #00              ;Reset counter
    jmp .ReadRows
jmp DriverLoop

fill $0500-pc()
;Test sine sample + dir page
db $04,$05,$04,$05
db $84, $17, $45, $35, $22, $22, $31, $21, $10, $68, $01, $21, $0D, $01, $08, $0B, $C3, $3E, $5B, $09, $8B, $D7, $B1, $E0, $BC, $AF, $78

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

SequenceMemory:
;%SetNoise($1E)
%SetDelayTime(8)
%SetDelayVolume($40, $20)
%SetDelayFeedback($7F)
%SetDelayCoefficient(0, $40)
%SetDelayCoefficient(1, $20)
%SetDelayCoefficient(2, $10)
%SetDelayCoefficient(3, $08)
%SetDelayCoefficient(4, $04)
%SetDelayCoefficient(5, $02)
%SetDelayCoefficient(6, $01)
%SetDelayCoefficient(7, $00)

%PlayPitch($0800, 0);Write note in
%PlayPitch($0010, 1);Write note in
%PlayPitch($0020, 2);Write note in
%PlayPitch($0030, 3);Write note in
db COM_EndRow
Engine_End:

arch 65816
base ROM_Engine_Start+(Engine_End-DriverStart)
ROM_Engine_End: