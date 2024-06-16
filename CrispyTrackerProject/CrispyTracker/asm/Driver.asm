;|==============================|
;|                              |
;|  Cobalt Audio Diver Program  |
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

base $0200          ;Set audio driver code to 0x0200 [closest to the start of memory we can get away with]

DriverStart:         ;Start of the driver
%spc_write(DSP_FLG, $20)
%spc_write(DSP_MVOL_L, $7F)
%spc_write(DSP_MVOL_R, $7F)
%spc_write(DSP_ESA, $DD)
%spc_write(DSP_EDL, $00)
%spc_write(DSP_DIR, $03)
%spc_write(DSP_V0_GAIN, $7F)
%spc_write(DSP_V0_ADSR1, $8F)
%spc_write(DSP_V0_ADSR2, $F1)
%spc_write(DSP_V0_VOL_L, $20)
%spc_write(DSP_V0_VOL_R, $20)
%spc_write(DSP_V0_PITCHL, $00)
%spc_write(DSP_V0_PITCHH, $10)
%spc_write(DSP_V0_SCRN, $00)
%spc_write(DSP_NON, $00)
%spc_write(DSP_PMON, $00)
%spc_write(DSP_EON, $00)
%spc_write(DSP_KOF, $00)
%spc_write(DSP_KON, $00)

mov SPC_Control, #$01   ;Set control bit to enable Timer 0
mov SPC_Timer1, #$FF    ;Divide timer to run at ~31hz
mov X, #00
mov COM_SequencePos, #SequenceMemory&$FF
mov COM_SequencePos+1, #(SequenceMemory>>8)&$FF

DriverLoop:         ;Main driver loop
    bra .TickRoutine
.ReadRows:
    mov A, (COM_SequencePos+X)  ;Grab the sequence position and store in A
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

.NoteCommands:
    cmp Y, #$8                  ;Check if we're doing an absolute pitch or a note table
    bpl .PlayPitch              ;If it's 

.PlayNote:


.PlayPitch:
    
    %spc_write(DSP_KON, 00)

.TickRoutine:       ;Routine for incrementing the tick counter and postiion within the track
    
    mov A, SPC_Count1   ;Check counter
    beq .TickRoutine    ;If the timer is set to 0
                        ;Assuming the timer is 1
    inc X               ;Increment our counter
    cmp X, #31          ;Check if the counter has reached the threshold
    bmi .TickRoutine    ;Go back to the tick routine if the counter is not 
                        ;Assuming we've hit the threshold
    mov X, #00          ;Reset counter
    %spc_write(DSP_KOF, $01)
    %spc_write(DSP_KON, $01)
    %spc_write(DSP_KOF, $00)
    bra .ReadRows

bra DriverLoop

fill $0300-pc()
;Test sine sample + dir page
db $04,$03,$04,$03
db $84, $17, $45, $35, $22, $22, $31, $21, $10, $68, $01, $21, $0D, $01, $08, $0B, $C3, $3E, $5B, $09, $8B, $D7, $B1, $E0, $BC, $AF, $78
SequenceMemory:
%PlayPitch($0010, 0);Write note in
%PlayPitch($00A8, 0);Write note in
%PlayPitch($0010, 0);Write note in
%PlayPitch($00A8, 0);Write note in
%PlayPitch($0000, 0);Write note in
Engine_End:

arch 65816
base ROM_Engine_Start+(Engine_End-DriverStart)
ROM_Engine_End: