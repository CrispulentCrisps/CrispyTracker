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
%spc_write(DSP_FLG, $20)
%spc_write(DSP_MVOL_L, $7F)
%spc_write(DSP_MVOL_R, $7F)
%spc_write(DSP_EVOL_L, $40)
%spc_write(DSP_EVOL_R, $40)
%spc_write(DSP_ESA, $BF)
%spc_write(DSP_EDL, $00)
%spc_write(DSP_EFB, $60)

%spc_write(DSP_DIR, $20)

%spc_write(DSP_V0_GAIN, $7F)
%spc_write(DSP_V0_ADSR1, $8F)
%spc_write(DSP_V0_ADSR2, $F0)
%spc_write(DSP_V0_VOL_L, $00)
%spc_write(DSP_V0_VOL_R, $00)
%spc_write(DSP_V0_PITCHL, $00)
%spc_write(DSP_V0_PITCHH, $00)
%spc_write(DSP_V0_SCRN, $00)

%spc_write(DSP_V1_GAIN, $7F)
%spc_write(DSP_V1_ADSR1, $8F)
%spc_write(DSP_V1_ADSR2, $F0)
%spc_write(DSP_V1_VOL_L, $00)
%spc_write(DSP_V1_VOL_R, $00)
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
%spc_write(DSP_V3_VOL_L, $00)
%spc_write(DSP_V3_VOL_R, $00)
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

DriverLoop:                         ;Main driver loop
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
    cmp X, #$20                 ;Compare the higher nibble to 20
    beq .InstrumentCommands     ;Compared and found X = 0
    cmp X, #$30                 ;Compare the higher nibble to 30
    beq .SpecialCommands        ;Compared and found X = 0
    cmp X, #$40                 ;Compare the higher nibble to 40
    beq .VolumeCommands         ;Compared and found X = 0

.NoteCommands:
    cmp Y, #$8                  ;Check if we're doing an absolute pitch or a note table
    bpl .GotoPlayPitch          ;If Y < 8 we know it's an absolute pitch command

.GotoPlayPitch:
    jmp .PlayPitch

.InstrumentCommands:
    jmp .SetInstrument

.SpecialCommands:
    cmp Y, #$0
    beq .GotoSetNoise
    cmp Y, #$1
    beq .GotoSetDelayTime
    cmp Y, #$2
    beq .GotoSetDelayVol
    cmp Y, #$3
    beq .GotoSetDelayFeedback
    cmp Y, #$4
    bpl .GotoSetDelayCoeff

;GOTO's to avoid beq and blp going out of bounds
.GotoSetDelayCoeff:
    jmp .SetDelayCoeff
.GotoSetDelayFeedback:
    jmp .SetDelayFeedback
.GotoSetDelayVol:
    jmp .SetDelayVol
.GotoSetNoise:
    jmp .SetNoise
.GotoSetDelayTime:
    jmp .SetDelayTime

.VolumeCommands:
    cmp Y, #$0                  ;Check if we're doing an absolute pitch or a note table
    beq .GotoSetMasterVolume    ;If we know the command is for the master volume
    cmp Y, #$1                  ;Check if we're doing an absolute pitch or a note table
    bpl .GotoSetVolume          ;If we know the command is for the master volume

.GotoSetMasterVolume:
    jmp .SetMasterVolume
.GotoSetVolume:
    jmp .SetVolume

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

.SetInstrument:
    mov X, #0                                           ;Reset X to 0 since we know the command type
    mov A, (COM_SequencePos+X)                          ;Grab position of counter
    incw COM_SequencePos                                ;Increments the sequence pos pointer
    push Y                                              ;Store Y in stack
                                                        ;Grab the right position in memory
    mov COM_TempMemADDRL, #InstrumentMemory&$FF
    mov COM_TempMemADDRH, #(InstrumentMemory>>8)&$FF
    mov Y, #8                                           ;Multiply the offset by 8, since instruments are 8 bytes large
    mul YA
    addw YA, COM_TempMemADDRL
    mov COM_TempMemADDRH, Y
    mov COM_TempMemADDRL, A
    mov Y, #0
    mov A, (COM_TempMemADDRL)+Y                         ;Goto point in memory and then store in A

    ;Mix volumes [Breaks when L/R = -128]
    pop X                           ;Take channel index out of the stack from Y and into X
    push X                          ;Store back instrument index for later use
    mov Y,COM_ChannelVol+X          ;Grab the current master volume of the channel
    mov X, #127                     ;Load 127 to use for division
    mul YA                          ;Multiply
    div YA, X                       ;Divide
                                    ;Now shove value in right address
    pop X                           ;Grab the instrument index
    push X                          ;push said instrument index back for R use
    push A                          ;push volume for later
    mov A, X                        ;shove X value into A
    xcn                             ;swap top and bottom nibbles
    mov SPC_RegADDR, A              ;Put in the channel addr
    pop A
    mov SPC_RegData, A              ;Shove in value from stack into A and into this
                                    
                                    ;Find Right volume
    mov Y, #1
    mov A, (COM_TempMemADDRL)+Y     ;Goto point in memory and then store in A
    pop X                           ;Take channel index out of the stack from Y and into X
    push X                          ;Store back instrument index for later use
    mov Y,COM_ChannelVol+X          ;Grab the current master volume of the channel
    mov X, #127                     ;Load 127 to use for division
    mul YA                          ;Multiply
    div YA, X                       ;Divide
                                    ;Now shove value in right address
    pop X                           ;Grab the stack value
    push X                          ;push said stack value back for R use
    push A                          ;push A value for later
    mov A, X                        ;shove X value into A
    xcn                             ;swap top and bottom nibbles
    adc A, #1
    mov SPC_RegADDR, A              ;Put in the channel addr
    pop A
    mov SPC_RegData, A              ;Shove in value from stack into A and into this

    ;Set ADSR1
    mov Y, #2
    mov A, (COM_TempMemADDRL)+Y     ;Goto point in memory and then store in A
    pop X                           ;Grab the stack value
    push X                          ;push said stack value back for later use
    push A                          ;Store ADSR1 value into stack
    mov A, X                        ;shove X value into A
    xcn                             ;swap top and bottom nibbles
    adc A, #5
    mov SPC_RegADDR, A              ;Put in the channel addr
    pop A
    mov SPC_RegData, A              ;Shove in value from stack into A and into this

    ;Set ADSR2
    mov Y, #3
    mov A, (COM_TempMemADDRL)+Y     ;Goto point in memory and then store in A
    pop X                           ;Grab the stack value
    push X                          ;push said stack value back for later use
    push A                          ;Store ADSR1 value into stack
    mov A, X                        ;shove X value into A
    xcn                             ;swap top and bottom nibbles
    adc A, #6
    mov SPC_RegADDR, A              ;Put in the channel addr
    pop A
    mov SPC_RegData, A              ;Shove in value from stack into A and into this

    ;Set Gain
    mov Y, #4
    mov A, (COM_TempMemADDRL)+Y     ;Goto point in memory and then store in A
    pop X                           ;Grab the stack value
    push X                          ;push said stack value back for later use
    push A                          ;Store ADSR1 value into stack
    mov A, X                        ;shove X value into A
    xcn                             ;swap top and bottom nibbles
    adc A, #7
    mov SPC_RegADDR, A              ;Put in the channel addr
    pop A
    mov SPC_RegData, A              ;Shove in value from stack into A and into this

    ;Set Effect state
    mov Y, #5
    mov A, (COM_TempMemADDRL)+Y     ;Goto point in memory and then store in A
    pop X                           ;Grab channel index
    push X                          ;Store instrument index
                                    ;Pitch mod
    mov A, ChannelTable+X           ;Shove in the bitmask index using the channel index
    eor A, #$FF                     ;Grab the inverse of said mask
    and A, COM_PModState            ;Apply mask to PModState
    mov Y, A
    and A, #%00000001               ;Grab the first bit
    beq .SkipPMod                   ;Skip the code if we find the bit is 0
    mov A, ChannelTable+X
    mov SPC_RegADDR, #DSP_PMON      ;Put in the channel addr
    mov SPC_RegData, Y              ;Shove the PMON bitfield in
    .SkipPMod:
                                    ;Noise
    mov A, (COM_TempMemADDRL)+Y     ;Goto point in memory and then store in A
    and A, #%00000010               ;Grab the second bit
    beq .SkipNoise                  ;Skip the code if we find the bit is 0
    mov A, ChannelTable+X
    mov SPC_RegADDR, #DSP_NON       ;Put in the channel addr
    mov SPC_RegData, A              ;Shove the NON bitfield in
    .SkipNoise:
                                    ;Echo on
    mov A, (COM_TempMemADDRL)+Y     ;Goto point in memory and then store in A
    and A, #%00000100               ;Grab the third bit
    beq .SkipEcho                   ;Skip the code if we find the bit is 0
    mov A, ChannelTable+X
    mov SPC_RegADDR, #DSP_EON       ;Put in the channel addr
    mov SPC_RegData, A              ;Shove the EON bitfield in
    .SkipEcho:

    ;Assign sample index to SCRN registers
    mov Y, #6
    mov A, (COM_TempMemADDRL)+Y     ;Goto point in memory and then store in A
    pop X                           ;Grab the stack value
    push X                          ;push said stack value back for later use
    push A                          ;Store ADSR1 value into stack
    mov A, X                        ;shove X value into A
    xcn                             ;swap top and bottom nibbles
    adc A, #4
    mov SPC_RegADDR, A              ;Put in the channel addr
    pop A
    mov SPC_RegData, A              ;Shove in value from stack into A and into this

    pop X
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
    sbc A, #4                       ;Sub 4 from A, now we have the exact coeffecient index
    mov Y, A                        ;Store back into Y
    mov A, CoeffecientTable+Y       ;Grab the register addr from the addr table offset by Y
    mov SPC_RegADDR, A              ;Shove register addr in from A
    mov SPC_RegData, X              ;Grab value stored in X and shove into regdata
    jmp .ReadRows

.SetMasterVolume:
    mov X, #0                       ;Reset X to 0 since we know the command type
    mov A, (COM_SequencePos+X)      ;Grab position of counter
    incw COM_SequencePos            ;Increments the sequence pos pointer
    mov SPC_RegADDR, #DSP_MVOL_L
    mov SPC_RegData, A
    mov A, (COM_SequencePos+X)      ;Grab position of counter
    incw COM_SequencePos            ;Increments the sequence pos pointer
    mov SPC_RegADDR, #DSP_MVOL_R
    mov SPC_RegData, A
    jmp .ReadRows

.SetVolume:
    mov X, #0                       ;Reset X to 0 since we know the command type
    mov A, (COM_SequencePos+X)      ;Grab position of counter
    incw COM_SequencePos            ;Increments the sequence pos pointer
    dec Y                           ;Decrement Y to get to the correct channel
    mov COM_ChannelVol+Y, A       ;Move the data into the right section of memory
    jmp .ReadRows

.TickRoutine:               ;Routine for incrementing the tick counter and postiion within the track
    mov X, #0               ;Reset counter
    
    .TickIncrement
    mov A, SPC_Count1       ;Check counter
    beq .TickIncrement      ;If the timer is set to 0
                            ;Assuming the timer is 1
    jmp .EffectsProcess     ;Process effects
    .ContinueIncrement:
    inc X                   ;Increment our counter
    cmp X, #31              ;Check if the counter has reached the threshold
    bmi .TickIncrement      ;Go back to the tick incrementer if the counter is not
                            ;Assuming we've hit the threshold
    mov X, #00              ;Reset counter
    jmp .ReadRows

.EffectsProcess:
    inc COM_TriangleCounter
    jmp .ContinueIncrement

jmp DriverLoop

fill $2000-pc()
assert pc() == $2000
;Test sine sample + dir page
db $04,$20,$04,$20
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

InstrumentMemory:
%WriteInstrument($7F, $40, $FF, $F0, $7F, $03, $00, $40)
%WriteInstrument($40, $60, $FF, $F0, $7F, $02, $00, $60)
%WriteInstrument($20, $7F, $FF, $F0, $7F, $04, $00, $80)
.EndOfInstrument:

SequenceMemory:
%SetNoise($1E)
%SetDelayTime(8)
%SetDelayVolume($40, $20)
%SetDelayFeedback($7F)
%SetMasterVolume($7F, $CC)
%SetChannelVolume(0, $80)
%SetChannelVolume(1, $40)
%SetChannelVolume(2, $20)
%SetChannelVolume(3, $10)
%SetChannelVolume(4, $08)
%SetChannelVolume(5, $04)
%SetChannelVolume(6, $02)
%SetChannelVolume(7, $01)
%SetInstrument($0, $0)
%SetInstrument($1, $1)
%SetInstrument($2, $2)
;%SetInstrument($3, $8)
;%SetInstrument($4, $10)
;%SetInstrument($5, $20)
;%SetInstrument($6, $40)
;%SetInstrument($7, $80)
%SetDelayCoefficient(0, $40)
%SetDelayCoefficient(1, $20)
%SetDelayCoefficient(2, $10)
%SetDelayCoefficient(3, $08)
%SetDelayCoefficient(4, $04)
%SetDelayCoefficient(5, $02)
%SetDelayCoefficient(6, $01)
%SetDelayCoefficient(7, $00)
%PlayPitch($0004, 0);Write note in
%PlayPitch($0008, 1);Write note in
%PlayPitch($0010, 2);Write note in
%PlayPitch($0020, 3);Write note in
db COM_EndRow
Engine_End:

arch 65816
base ROM_Engine_Start+(Engine_End-DriverStart)
ROM_Engine_End: