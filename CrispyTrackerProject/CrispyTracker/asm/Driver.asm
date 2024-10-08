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
%spc_write(DSP_EON, $00)
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

mov SPC_Control, #$01                           ;Set control bit to enable Timer 0
mov SPC_Timer1, #$FF                            ;Divide timer to run at ~31hz
mov X, #00                                      ;Reset X
mov COM_SequencePos, #SequenceMemory&$FF        ;Grab the lower byte of the sequnce position in memory
mov COM_SequencePos+1, #(SequenceMemory>>8)&$FF ;Grab the higher byte of the sequnce position in memory
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
    and A, #$0F                 ;Get lower 4 nibbles to A
    mov Y, A                    ;Shove lower nibbles into Y
    mov A, X                    ;Reset A to its previous state held in X
    and A, #$F0                 ;Get higher 4 nibbles to A
    mov X, A                    ;Shove higher 4 nibbles into X
                                
                                ;Find command type
    cmp X, #$00                 ;Compare the higher nibble to 00
    beq .StateCommands          ;Compared and found X = 0
    cmp X, #$10                 ;Compare the higher nibble to 10
    beq .NoteCommands           ;Compared and found X = 0
    cmp X, #$20                 ;Compare the higher nibble to 20
    beq .InstrumentCommands     ;Compared and found X = 0
    cmp X, #$30                 ;Compare the higher nibble to 30
    beq .SpecialCommands        ;Compared and found X = 0
    cmp X, #$40                 ;Compare the higher nibble to 40
    beq .VolumeCommands         ;Compared and found X = 0
    cmp X, #$50                 ;Compare the higher nibble to 50
    beq .EffectCommands         ;Compared and found X = 0
    cmp X, #$60                 ;Compare the higher nibble to 60
    beq .PortVibSet             ;Compared and found X = 0
    cmp X, #$70                 ;Compare the higher nibble to 70
    beq .TremVolSet             ;Compared and found X = 0
    cmp X, #$80                 ;Compare the higher nibble to 80
    beq .RetPbrSet              ;Compared and found X = 0

.StateCommands:
    cmp Y, #$1
    beq .GotoSetSpeed

.GotoSetSpeed:
    jmp .SetSpeed

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

.EffectCommands:
    cmp Y, #8               ;Compare Y to see if we're setting the effects bitfield
    beq .GotoSetArpValue    ;otherwise if it's = 0 then we go to arp value

.PortVibSet:
    cmp Y, #8               ;Compare Y to see if we're setting the effects bitfield
    bmi .GotoSetPortValue   ;if it's < 0 then we go to the set the port value
    beq .GotoSetVibValue    ;otherwise if it's = 0 then we go to set the vibrato value

.TremVolSet:
    cmp Y, #8                   ;Compare Y to see if we're setting the effects bitfield
    bmi .GotoSetTremValue       ;if it's < 0 then we go to the set the port value
    beq .GotoSetVolSlideValue   ;otherwise if it's = 0 then we go to set the vibrato value

.RetPbrSet:
    cmp Y, #8               ;Compare Y to see if we're setting the effects bitfield
    bmi .GotoSetPanbrValue  ;otherwise if it's = 0 then we go to set the vibrato value

.GotoSetArpValue:
    jmp .SetArpValue
.GotoSetPortValue:
    jmp .SetPortValue
.GotoSetVibValue:
    jmp .SetVibValue
.GotoSetTremValue:
    jmp .SetTremValue
.GotoSetVolSlideValue:
    jmp .SetVolSlideValue
.GotoSetPanbrValue:
    jmp .SetPanbrValue

.SetSpeed:
    mov X, #0                   ;Reset X to 0 since we know the command type
    mov A, (COM_SequencePos+X)  ;Grab position of counter
    incw COM_SequencePos        ;Increments the sequence pos pointer
    mov COM_TickThresh, A       ;Shove Track speed into zp
    jmp .ReadRows

.PlayNote:
    jmp .ReadRows

.PlayPitch:
    mov X, #0                               ;Reset X to 0 since we know the command type
    mov A, Y                                ;Shove Y into A
    and A, #%11110111                       ;Sub 8 from A
    push A                                  ;Save A value
    mov X, A
    push A
    mov COM_TempScratchMem, A               ;Store index for KON
    mov Y, #0
    
    mov A, COM_ChannelVibratoValue          ;Grab vib value
    and A, #$F0                             ;Grab speed
    xcn A                                   ;Swap nibbles
    asl A                                   ;Multiply the speed !
    asl A                                   ;Multiply the speed !!
    asl A                                   ;Multiply the speed !!!
    mov COM_TriangleCounterVibrato+X, Y     ;Reset Triangle counter
    mov COM_TriangleStateVibrato+X, A       ;Reset Triangle state
    
    mov A, COM_ChannelTremolandoValue
    and A, #$0F
    asl A
    mov COM_TriangleCounterTremo+X, Y       ;Reset Triangle counter
    mov COM_TriangleStateTremo+X, A         ;Reset Triangle state

    mov COM_TriangleSignHolder, Y           ;Reset Triangle state
    pop A
    xcn A                                   ;Swap hi and lo nibbles
    or A, #DSP_V0_PITCHH                    ;Or A with DSP_V0_PITCHH
    mov Y, A                                ;Shove A into Y
    mov X, #0                               ;Reset X
    mov A, (COM_SequencePos+X)              ;Grab the sequence position and store in A
    mov X, A                                ;Store value into stack
    pop A                                   ;Grab channel index
    asl A                                   ;Multiply by 2
    inc A                                   ;Go to the high byte
    push X                                  ;Shove value into stack
    mov X, A                                ;Move index into X
    pop A                                   ;Grab exact value from stack
    mov COM_ChannelPitches+X, A             ;Shove value into Channel Pitches
    mov COM_ChannelPitchesOutput+X, A       ;Shove value into Channel Pitches Copy
    push X                                  ;Store index to stack
    mov X, #0                               ;Reset X
    mov SPC_RegADDR, Y                      ;Get to the Hi pitch
    incw COM_SequencePos                    ;Increments the sequence pos pointer
    mov SPC_RegData, A                      ;Shove pitch value into the regdata
    dec Y                                   ;Decrement Y to get to the lo pitch value
    mov A, (COM_SequencePos+X)              ;Grab the sequence position and store in A
    mov X, A                                ;Store value into stack
    pop A                                   ;Grab channel index
    dec A                                   ;Go to the low byte
    push X                                  ;Shove value into stack
    mov X, A                                ;Move index into X
    pop A                                   ;Grab exact value from stack
    mov COM_ChannelPitches+X, A             ;Shove value into Channel Pitches
    mov COM_ChannelPitchesOutput+X, A       ;Shove value into Channel Pitches Copy
    mov X, #0                               ;Reset X
    mov SPC_RegADDR, Y                      ;Get to the Lo pitch
    mov SPC_RegData, A
    incw COM_SequencePos                    ;Increments the sequence pos pointer
    mov X, COM_TempScratchMem               ;Grab index from memory
    mov A, ChannelTable+X
    or A, COM_KONState
    mov COM_KONState, A
    mov SPC_RegADDR, #DSP_KON
    mov SPC_RegData, A
    jmp .ReadRows

.SetInstrument:
    mov X, #0                                           ;Reset X to 0 since we know the command type
    mov A, (COM_SequencePos+X)                          ;Grab instrument index
    incw COM_SequencePos                                ;Increments the sequence pos pointer
    push Y                                              ;Store Y in stack
                                                        ;Grab the right position in memory
    mov COM_TempMemADDRL, #InstrumentMemory&$FF
    mov COM_TempMemADDRH, #(InstrumentMemory>>8)&$FF
    mov X, A                                            ;Shove index into X to shut the assembler up
    mov COM_ChannelInstrumentIndex+Y, X                 ;Shove instrument index into the index array in zp
    mov Y, #8                                           ;Multiply the offset by 8, since instruments are 8 bytes large
    mul YA                                              ;Multiply
    addw YA, COM_TempMemADDRL                           ;Add on the offset to the inst pointer
    mov COM_TempMemADDRH, Y                             ;Apply
    mov COM_TempMemADDRL, A                             ;Apply

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
    mov A, (COM_TempMemADDRL)+Y         ;Goto point in memory and then store in A
    pop X                               ;Grab channel index
    push X                              ;Store instrument index
    push A                              ;Shove effect state into the stack
                                        ;Pitch mod
    mov A, ChannelTable+X               ;Shove in the bitmask index using the channel index
    eor A, #$FF                         ;Grab the inverse of said mask
    and A, COM_PModState                ;Apply mask to PModState
    mov COM_PModState, A                ;Apply
    pop A                               ;Grab effect state and put into A
    push A                              ;Store for later use
    and A, #%00000001                   ;Grab the first bit
    beq .SkipPMod                       ;Skip the code if we find the bit is 0
    mov A, ChannelTable+X               ;Grab bitfield state
    mov COM_TempORStore, A              ;Store A into COM_TempORStore
    or COM_PModState, COM_TempORStore   ;OR in the Pmod state with the value in A, stored in COM_TempORStore
    mov SPC_RegADDR, #DSP_PMON          ;Put in the channel addr
    mov SPC_RegData, COM_PModState      ;Shove the PMON bitfield in
    .SkipPMod:
                                        ;Noise
    mov A, (COM_TempMemADDRL)+Y         ;Goto point in memory and then store in A
    mov A, ChannelTable+X               ;Shove in the bitmask index using the channel index
    eor A, #$FF                         ;Grab the inverse of said mask
    and A, COM_NoiseState               ;Apply mask to NoiseState
    mov COM_NoiseState, A               ;Apply
    pop A                               ;Grab effect state and put into A
    push A                              ;Store for later use
    and A, #%00000010                   ;Grab the first bit
    beq .SkipNoise                      ;Skip the code if we find the bit is 0
    mov A, ChannelTable+X               ;Grab bitfield state
    mov COM_TempORStore, A              ;Store A into COM_TempORStore
    or COM_NoiseState, COM_TempORStore  ;OR in the Pmod state with the value in A, stored in COM_TempORStore
    mov SPC_RegADDR, #DSP_NON           ;Put in the channel addr
    mov SPC_RegData, COM_NoiseState     ;Shove the NON bitfield in
    .SkipNoise:
                                        ;Echo on
    mov A, (COM_TempMemADDRL)+Y         ;Goto point in memory and then store in A
    mov A, ChannelTable+X               ;Shove in the bitmask index using the channel index
    eor A, #$FF                         ;Grab the inverse of said mask
    and A, COM_EchoState                ;Apply mask to EchoState
    mov COM_EchoState, A                ;Apply
    pop A                               ;Grab effect state and put into A
    push A                              ;Store for later use
    and A, #%00000100                   ;Grab the first bit
    beq .SkipEcho                       ;Skip the code if we find the bit is 0
    mov A, ChannelTable+X               ;Grab bitfield state
    mov COM_TempORStore, A              ;Store A into COM_TempORStore
    or COM_EchoState, COM_TempORStore   ;OR in the Pmod state with the value in A, stored in COM_TempORStore
    mov SPC_RegADDR, #DSP_EON           ;Put in the channel addr
    mov SPC_RegData, COM_EchoState      ;Shove the EON bitfield in
    .SkipEcho:
    pop Y                               ;Here to prevent stack leaks
                                    
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
    dec Y                           ;Decrement Y to get to the correct channel
    mov A, Y
    asl A
    mov Y, A
    mov A, (COM_SequencePos+X)      ;Grab position of counter
    incw COM_SequencePos            ;Increments the sequence pos pointer
    mov COM_ChannelVol+Y, A         ;Move the data into the right section of memory
    inc Y
    mov A, (COM_SequencePos+X)      ;Grab position of counter
    incw COM_SequencePos            ;Increments the sequence pos pointer
    mov COM_ChannelVol+Y, A         ;Move the data into the right section of memory
    jmp .ReadRows

.SetArpValue:
    mov X, #0                       ;Reset X to 0 since we know the command type
    mov A, (COM_SequencePos+X)      ;Grab position of counter
    incw COM_SequencePos            ;Increments the sequence pos pointer
    push A                          ;Store A value
    mov A, Y                        ;Move lower nibble into A to decrement
    sbc A, #8                       ;Decrement by 8 to find the channel index
    mov X, A                        ;Put channel index into X
    pop A                           ;Grab value we stored to put into the arp value array
    mov COM_ChannelArpValue+X, A
    jmp .ReadRows
    
.SetPortValue:
    mov X, #0                           ;Reset X to 0 since we know the command type
    mov A, (COM_SequencePos+X)          ;Grab position of counter
    incw COM_SequencePos                ;Increments the sequence pos pointer
    push A                              ;Store A value
    mov A, Y                            ;Move channel index to A
    mov X, A                            ;Put channel index into X
    pop A                               ;Grab value we stored to put into the port value array
    mov COM_ChannelPortValue+X, A       ;Write value to table
    mov A, (COM_SequencePos+X)          ;Grab value from instruction set
    incw COM_SequencePos                ;Increments the sequence pos pointer
    push A                              ;Shove value to stack as we need to increment X using A
    mov A, X                            ;Shove X into A
    inc A                               ;Increment
    mov X, A                            ;Return value
    pop A                               ;Grab value from stack
    mov COM_ChannelPortValue+X, A       ;Write value to table
    jmp .ReadRows

.SetVibValue:
    mov X, #0                           ;Reset X to 0 since we know the command type
    mov A, Y                            ;Shove Y into A for maths
    sbc A, #8                           ;Decrement 8
    mov Y, A                            ;Return
    mov A, (COM_SequencePos+X)          ;Grab position of counter
    incw COM_SequencePos                ;Increments the sequence pos pointer
    mov COM_ChannelVibratoValue+Y, A    ;Apply value to memory
    and A, #$F0                         ;Grab speed
    xcn A
    mov COM_TriangleStateVibrato+Y, A   ;Set triangle speed
    jmp .ReadRows

.SetTremValue:
    mov X, #0                           ;Reset X to 0 since we know the command type
    mov A, (COM_SequencePos+X)          ;Grab position of counter
    incw COM_SequencePos                ;Increments the sequence pos pointer
    mov COM_ChannelTremolandoValue+Y, A ;Apply value to memory
    and A, #$F0                         ;Grab speed
    xcn A                               ;Swap Nibble
    asl A
    mov COM_TriangleStateTremo+Y, A     ;Set triangle speed
    jmp .ReadRows

.SetVolSlideValue:
    mov X, #0                           ;Reset X to 0 since we know the command type
    mov A, (COM_SequencePos+X)          ;Grab position of counter
    incw COM_SequencePos                ;Increments the sequence pos pointer
    push A                              ;Store value in stack
    mov A, Y                            ;Shove index to take away 8
    sbc A, #8                           ;Subtract 8
    mov Y, A
    pop A                               ;Grab value from stack
    mov COM_ChannelVolSlideValue+Y, A   ;Apply value to memory
    jmp .ReadRows

.SetPanbrValue:
    mov X, #0                           ;Reset X to 0 since we know the command type
    mov A, (COM_SequencePos+X)          ;Grab position of counter
    incw COM_SequencePos                ;Increments the sequence pos pointer
    mov COM_ChannelPanbrelloValue+Y, A  ;Apply value to memory
    and A, #$F0                         ;Grab speed
    xcn A                               ;Swap Nibble
    asl A                               ;Multiply by 2
    mov COM_TriangleStatePanbr+Y, A     ;Set triangle 
    jmp .ReadRows

.TickRoutine:               ;Routine for incrementing the tick counter and postiion within the track
    mov X, #0               ;Reset counter
    
    .TickIncrement:
    mov Y, SPC_Count1       ;Check counter
    beq .TickIncrement      ;If the timer is set to 0
                            ;Assuming the timer is 1
    jmp .EffectsProcess     ;Process effects
    .ContinueIncrement:
    inc X                   ;Increment our counter
    cmp X, COM_TickThresh   ;Check if the counter has reached the 
    ;cmp X, #$31             ;Check if the counter has reached the threshold
    bmi .TickIncrement      ;Go back to the tick incrementer if the counter is not
                            ;Assuming we've hit the threshold
    mov X, #00              ;Reset counter
    jmp .ReadRows

.EffectsProcess:

    .ForEffects:
    push X                          ;Store away the current tick for after the effects processing
    call CountTriangle                  ;Call triangle subroutine

    ;------------------;
    ;    Volume Mix    ;
    ;------------------;
    mov COM_TempMemADDRL, #InstrumentMemory&$FF         ;Grab lower byte of instrument address
    mov COM_TempMemADDRH, #(InstrumentMemory>>8)&$FF    ;Grab higher byte of instrument address
    mov X, COM_EffectChannel                            ;Grab channel index
    mov A, COM_ChannelInstrumentIndex+X                 ;Grab instrument index
    mov Y, #8                                           ;Multiply instrument index by 8 to get the correct position within the instrument table
    mul YA                                              ;16 bit multiplication my beloved <3
    addw YA, COM_TempMemADDRL                           ;16 bit add to get to the correct position in the table
    mov COM_TempMemADDRH, Y                             ;Seperate the 2 bytes into H and L positions
    mov COM_TempMemADDRL, A                             
    
    mov A, COM_EffectChannel
    asl A
    mov X, A
    mov Y, #0                                           ;Reset Y
    mov A, (COM_TempMemADDRL)+Y                         ;Grab instrument left volume
    mov Y,COM_ChannelVol+X                              ;Grab the current master volume of the channel
    mov X, #127                                         ;Divisor
    mul YA                                              ;Multiply the instrument volume by the channel volume
    div YA, X                                           ;Divide by X
    push A                                              ;Store mixed volume to stack
    mov A, COM_EffectChannel                            ;Move index into A
    asl A                                               ;Mult by 2
    mov X, A                                            ;Move index to X
    pop A                                               ;Grab stack value
    mov COM_ChannelVolumeOutput+X, A                    ;Apply

    mov A, COM_EffectChannel                            ;Grab index and shove into A for maths 
    asl A
    mov X, A
    inc X
    mov Y, #1                                           ;Reset Y
    mov A, (COM_TempMemADDRL)+Y                         ;Grab instrument right volume
    mov Y,COM_ChannelVol+X                              ;Grab the current master volume of the channel
    mov X, #127                                         ;Divisor
    mul YA                                              ;Multiply the instrument volume by the channel volume
    div YA, X                                           ;Divide by X
    push A                                              ;Store mixed volume to stack
    mov A, COM_EffectChannel                            ;Move index into A
    asl A                                               ;Mult by 2
    inc A                                               ;Add for R volume
    mov X, A                                            ;Move index to X
    pop A                                               ;Grab stack value
    mov COM_ChannelVolumeOutput+X, A                    ;Apply

    ;------------------;
    ;    Portamento    ;
    ;------------------;
    mov X, COM_EffectChannel                ;Grab channel index
    mov A, X                                ;Shove index into A for mathematical operations
    asl A                                   ;Multiply A by 2
    inc A                                   ;Increment to get the portamento direction
    mov X, A                                ;Shove index into X
    mov Y, COM_ChannelPortValue+X           ;Shove the port direction into Y
    mov COM_PortDir, Y                      ;Shove dir in memory
    dec A                                   ;Decrement to get correct index
    mov X, A                                ;Shove A into X
    mov Y, COM_ChannelPitches+X             ;Grab the channel lo pitch and store in Y
    push X                                  ;Store premultiplied index to stack
    mov A, Y                                ;Put lo pitch into A
    cmp COM_PortDir, #0                     ;Check if dir is set to 0
    clrc
    bne .NegativePort                       ;Go down if the value is != 0
    adc A, COM_ChannelPortValue+X           ;Increment A by the Portamneto value
    jmp .FinishDirCheck                     ;Jump over
    .NegativePort:
    sbc A, COM_ChannelPortValue+X           ;Decrement A by the Portamneto value
    .FinishDirCheck
    mov Y, A                                ;Move portamento value into Y
    pop X                                   ;Grab premultiplied index from stack
    push X                                  ;Store premult for later use
    mov COM_ChannelPitches+X, Y             ;Put value back into channel pitches
    bcc .SkipCarry                          ;Check if the carry flag is off
                                            ;Otherwise increment the hi pitch
    mov A, X                                ;Shove X into A for maths
    inc A                                   ;Increment A
    mov X, A                                ;Put index back into X
    mov A, COM_ChannelPitches+X             ;Move hi pitch value in
    
    cmp COM_PortDir, #0                     ;Check if dir is set to 0
    bne .DecrementHi                        ;Go down if the value is != 0
    inc A                                   ;Increment A
    jmp .ApplyChange                        ;Jump over
    .DecrementHi:                           
    dec A                                   ;Decrement A
    .ApplyChange:
    mov COM_ChannelPitches+X, A             ;Put value back into channel pitches
    .SkipCarry:

    ;-------------------;
    ;      Vibrato      ;
    ;-------------------;
    ;Speed
    mov X, COM_EffectChannel            ;Grab channel index
    mov COM_TempPitchProcess, #0        ;Reset temp pitch process so as to not fuck up other parts
    mov COM_TempPitchProcess+1, #0      ;Reset temp pitch process so as to not fuck up other parts
    mov COM_TempMemADDRL, #0
    ;Depth
    mov X, COM_EffectChannel            ;Grab channel index
    mov A, COM_ChannelVibratoValue+X    ;Grab vibrato value
    and A, #$0F                         ;Grab lower nibble for depth
    mov Y, A                            ;Shove depth into Y
    mov A, COM_TriangleCounterVibrato+X ;Grab triangle counter
    mov COM_TempMemADDRL, A             ;Store A into temporary memory
    call SignedMul                      ;Call SignedMul subroutine
    mov COM_TempPitchProcess, A         ;Shove lo byte into the temp pitch process
    mov COM_TempPitchProcess+1, Y       ;Shove hi byte into the temp pitch process
    ;Apply
    mov A, COM_EffectChannel            ;Grab channel index
    asl A                               ;Multiply by 2
    mov X, A                            ;Shove into X for indexing
    mov A, COM_ChannelPitches+X         ;Shove lo byte into A
    inc X                               ;Increment to get the lo byte
    mov Y, COM_ChannelPitches+X         ;Shove hi byte into Y
    addw YA, COM_TempPitchProcess       ;Add offset into the pitch value
    dec X
    mov COM_ChannelPitchesOutput+X, A   ;Shove in lo byte into pitch lo byte
    inc X                               ;Increment
    mov COM_ChannelPitchesOutput+X, Y   ;Shove Y into hi byte
    
    ;-------------------;
    ;     Tremolando    ;
    ;-------------------;
    mov A, COM_EffectChannel            ;Grab channel index
    asl A                               ;Mult by 2
    mov X, A                            ;Return
    mov Y, COM_ChannelVol+X             ;Grab channel volume
    lsr A                               ;Div by 2
    mov X, A                            ;Return
    mov A, COM_ChannelTremolandoValue+X ;Grab the channel tremo value
    cmp A, #0                           ;Check if the tremovalue is 0 to avoid *0 or /0
    beq .SkipTremo                      ;Skip if equal to 0
    mov A, COM_ChannelTremolandoValue+X ;Grab the channel tremo value
    and A, #$0F                         ;Grab depth
    mov Y, A                            ;Shove depth into Y
    mov A, COM_TriangleCounterTremo+X   ;Grab triangle value
    mov COM_TempMemADDRL, A
    lsr A                               ;Divide by 2
    lsr A                               ;Divide by 2
    lsr A                               ;Divide by 2
    lsr A                               ;Divide by 2
    lsr A                               ;Divide by 2
    call SignedMul
    mov COM_TempVolumeProcess, Y        ;Apply
    mov COM_TempVolumeProcess+1, A      ;Apply
    jmp .AvoidWrongMov                  ;Jump to avoid a wrongful overwrite
    .SkipTremo:
    mov COM_TempVolumeProcess, #0       ;Apply
    mov COM_TempVolumeProcess+1, #0     ;Apply
    .AvoidWrongMov:

    ;-------------------;
    ;    Volume Slide   ;
    ;-------------------;
    mov X, COM_EffectChannel            ;Shove channel index into X
    mov A, COM_ChannelVolSlideValue+X   ;Grab channel volume 
    cmp A, #0                           ;Check if it's 0
    beq .SkipVolumeSlide                ;if so then skip the volume code
    and A, #$F0                         ;Grab upper nibble
    beq .DownSlide                      ;If the lower nibble isn't 0 then we assume it's a downward volume slide
    .UpSlide:
    mov A, COM_ChannelVolSlideValue+X   ;Shove in volume slide value
    xcn                                 ;swap nibbles as otherwise it'll decay the volume by like F0 at most in a single call
    mov COM_TempMemADDRL, A             ;Shove XCN'd value into a temporary addr
    mov A, COM_ChannelVol+X             ;Grab channel volume
    adc A, COM_TempMemADDRL             ;Decrement the volume by the slide value
    mov COM_ChannelVol+X, A             ;Apply value change
    jmp .SkipVolumeSlide
    .DownSlide:
    mov A, COM_ChannelVol+X             ;Grab channel volume
    sbc A, COM_ChannelVolSlideValue+X   ;Decrement the volume by the slide value
    mov COM_ChannelVol+X, A             ;Apply value change
    .SkipVolumeSlide:
    
    ;-------------------;
    ;     Panbrello     ; [Known inversion issues with the volume]
    ;-------------------;
    mov X, COM_EffectChannel            ;Shove channel index into X
    mov A, COM_ChannelPanbrelloValue+X  ;Grab panbr value
    cmp A, #0                           ;Check if it's 0
    beq .SkipPanbr                      ;if so then skip the panbr code
    and A, #$0F                         ;Grab depth nibble
    mov Y, A                            ;Store into Y for multiplication
    mov A, COM_TriangleCounterPanbr+X   ;Shove in triangle counter value
    cmp A, #$80                         ;Store sign into carry
    ror A                               ;Div 2
    cmp A, #$80                         ;Store sign into carry
    ror A                               ;Div 2
    cmp A, #$80                         ;Store sign into carry
    ror A                               ;Div 2
    cmp A, #$80                         ;Store sign into carry
    ror A                               ;Div 2
    mul YA                              ;Mult to get total volume influence
    mov COM_TempScratchMem, A           ;Store panbrello influence to temp memory
    ;Left
    mov COM_TempMemADDRL, A             ;Return value
    ;Right
    not1 COM_TempScratchMem.7           ;Invert Sign bit
    mov A, COM_TempScratchMem           ;Shove into A
    mov COM_TempMemADDRH, A             ;Return value
    ;Apply
    mov A, COM_TempVolumeProcess        ;Grab current volume change
    adc A, COM_TempMemADDRL             ;Add together
    bvc .SkipOverflowL
    eor A, $FF
    .SkipOverflowL:
    mov COM_TempVolumeProcess, A        ;Return
    mov A, COM_TempVolumeProcess+1      ;Grab current volume change
    setc
    sbc A, COM_TempMemADDRH             ;Sub together
    bvc .SkipOverflowR
    eor A, $FF
    .SkipOverflowR:
    mov COM_TempVolumeProcess+1, A      ;Return
    .SkipPanbr:

    ;-------------------;
    ;    Apply Effect   ;
    ;-------------------;
    ;Volume/Panning
    mov A, COM_EffectChannel                            ;Grab channel index
    asl A                                               ;Mult by 2
    mov X, A
    mov A, COM_ChannelVolumeOutput+X                    ;Shove the output volume into A
    adc A, COM_TempVolumeProcess                        ;Add volume offset
    bvc .SkipLCorr                                      ;Skip if the overflow isn't set
    eor A, $FF
    .SkipLCorr:
    mov COM_ChannelVolumeOutput+X, A                    ;Apply offset
    inc X
    mov A, COM_ChannelVolumeOutput+X                    ;Shove the output volume into A
    adc A, COM_TempVolumeProcess+1                      ;Add volume offset
    bvc .SkipRCorr                                      ;Skip if the overflow isn't set
    eor A, $FF
    .SkipRCorr:
    mov COM_ChannelVolumeOutput+X, A                    ;Apply offset

    ;Stereo switch
    mov1 C, COM_TrackSettings.0                         ;Move the mono flag into the carry bit
    bcc .SkipMono                                       ;Check if the carry is 0
    dec X                                               ;We already got the right index, just decrement to get the L volume
    mov A, COM_ChannelVolumeOutput+X                    ;Shove the output volume into A
    mov COM_TempMemADDRH, A                             ;Shove into scratch memory
    mov1 C, COM_TempMemADDRH.7                          ;Move the 7th bit into the carry bit
    bcc .SkipL
    eor A, #$FF                                         ;Invert to remove sign
    mov COM_ChannelVolumeOutput+X, A                    ;Return
    inc X
    .SkipL:                                             ;Grab R index
    mov A, COM_ChannelVolumeOutput+X                    ;Shove the output volume into A
    mov COM_TempMemADDRH, A                             ;Shove into scratch memory
    mov1 C, COM_TempMemADDRH.7                          ;Move the 7th bit into the carry bit
    bcc .SkipMono
    eor A, #$FF                                         ;Invert to remove sign
    mov COM_ChannelVolumeOutput+X, A                    ;Return
    .SkipMono:
    
    ;Apply Volume
    mov A, COM_EffectChannel                            ;Grab channel index
    asl A                                               ;Mult by 2
    mov X, A                                            ;Shove multiplied index into X
    mov A, COM_EffectChannel                            ;Grab channel index
    xcn A                                               ;Swap nibbles
    mov Y, COM_ChannelVolumeOutput+X                    ;Shove the output volume into Y
    mov SPC_RegADDR, A                                  ;Shove A to addr
    mov SPC_RegData, Y                                  ;Shove Master volume into L
    inc A                                               ;Inc addr
    inc X                                               ;Inc index
    mov Y, COM_ChannelVolumeOutput+X                    ;Shove the output volume into Y
    mov SPC_RegADDR, A                                  ;Shove A to addr
    mov SPC_RegData, Y                                  ;Shove Master volume into R

    ;Apply Pitch
    mov X, COM_EffectChannel                            ;Grab channel index
    mov A, X                                            ;Shove index into A
    asl A                                               ;Multiply by 2
    mov X, A                                            ;Move mult value into X
    lsr A                                               ;Divide by 2
    xcn A                                               ;Swap nibbles
    adc A, #2                                           ;Inc 2 to grab low pitch
    mov Y, A                                            ;Shove addr value into Y
    mov A, COM_ChannelPitchesOutput+X                   ;Shove lo pitch into A
    mov SPC_RegADDR, Y                                  ;Shove Y to addr
    mov SPC_RegData, A                                  ;Shove lo pitch value into data
    pop A                                               ;Grab premult index value
    inc A                                               ;Increment
    mov X, A                                            ;Return value
    mov A, Y                                            ;Shove Y into A
    inc A                                               ;Increment
    mov Y, A                                            ;Return value
    mov A, COM_ChannelPitchesOutput+X                   ;Shove hi pitch into A
    mov SPC_RegADDR, Y                                  ;Shove Y to addr
    mov SPC_RegData, A                                  ;Shove hi pitch value into data

    ;-------------------;
    ;   FOR loop check  ;
    ;-------------------;
    pop X                           ;Grab the former tick integer and shove back into X
    mov A, COM_EffectChannel        ;Grab current channel index
    cmp A, #7                       ;Check if we've reached the 7th element
    bne .GotoForEffects             ;If we have then redo the loop

    mov COM_EffectChannel, #0       ;Otherwise we reset the index
    jmp .ContinueIncrement          ;Go back to the increment routine

.GotoForEffects:
    inc A                       ;Increment the channel index
    mov COM_EffectChannel, A    ;Shove A into the channel index
    jmp .ForEffects             ;Jump back and start the loop all over again

jmp DriverLoop

    ;------------------;
    ; Triangle counter ;
    ;------------------;
    ;   Note of usage:
    ;       To use the triangle counter, use CALL to call the subroutine 
    ;       and then reference the triangle counter at the precise memory location
    ;
    ;       To change the speed, multiply the triangle state by the desired number
    ;
CountTriangle:
    ;Vibrato
    mov X, COM_EffectChannel                    ;Grab channel index
    mov A, (COM_TriangleCounterVibrato)+X       ;Grab the triangle counter
    clrc                                        ;Clear the carry flag
    mov COM_TriangleSignHolder, A               ;Store A into TriangleSignHolder
    adc A, COM_TriangleStateVibrato+X           ;Add to TriangleCounter the triangle state
    mov (COM_TriangleCounterVibrato)+X, A       ;Return back to the triangle counter
    bvc .ContinueEffectsVib                     ;Check if overflown
    rol A                                       ;Put sign bit in carry
    eor1 C, COM_TriangleSignHolder.7            ;Invert carry based off of the sign bit in the sign holder
    bcc .ContinueEffectsVib                     ;If the overflow flag is NOT set
    mov A, COM_TriangleStateVibrato+X           ;Shove triangle state into A
    eor A, #$FF                                 ;Invert to get negative
    inc A                                       ;Add one to value
    mov COM_TriangleStateVibrato+X, A           ;Return back to triangle counter
    mov A, COM_TriangleSignHolder               ;Shove sign holder into A
    mov (COM_TriangleCounterVibrato)+X, A       ;Shove back into triangle counter array
    .ContinueEffectsVib:
    ;Tremolando
    mov X, COM_EffectChannel                    ;Grab channel index
    mov A, (COM_TriangleCounterTremo)+X         ;Grab the triangle counter
    clrc                                        ;Clear the carry flag
    mov COM_TriangleSignHolder, A               ;Store A into TriangleSignHolder
    adc A, COM_TriangleStateTremo+X             ;Add to TriangleCounter the triangle state
    mov (COM_TriangleCounterTremo)+X, A         ;Return back to the triangle counter
    bvc .ContinueEffectsTrem                    ;Check if overflown
    rol A                                       ;Put sign bit in carry
    eor1 C, COM_TriangleSignHolder.7            ;Invert carry based off of the sign bit in the sign holder
    bcc .ContinueEffectsTrem                    ;If the overflow flag is NOT set
    mov A, COM_TriangleStateTremo+X             ;Shove triangle state into A
    eor A, #$FF                                 ;Invert to get negative
    inc A                                       ;Add one to value
    mov COM_TriangleStateTremo+X, A             ;Return back to triangle counter
    mov A, COM_TriangleSignHolder               ;Shove sign holder into A
    mov (COM_TriangleCounterTremo)+X, A         ;Shove back into triangle counter array
    .ContinueEffectsTrem:
    ;Panbrello
    mov X, COM_EffectChannel                    ;Grab channel index
    mov A, (COM_TriangleCounterPanbr)+X         ;Grab the triangle counter
    clrc                                        ;Clear the carry flag
    mov COM_TriangleSignHolder, A               ;Store A into TriangleSignHolder
    adc A, COM_TriangleStatePanbr+X             ;Add to TriangleCounter the triangle state
    mov (COM_TriangleCounterPanbr)+X, A         ;Return back to the triangle counter
    bvc .ContinueEffectsPanbr                   ;Check if overflown
    rol A                                       ;Put sign bit in carry
    eor1 C, COM_TriangleSignHolder.7            ;Invert carry based off of the sign bit in the sign holder
    bcc .ContinueEffectsPanbr                   ;If the overflow flag is NOT set
    mov A, COM_TriangleStatePanbr+X             ;Shove triangle state into A
    eor A, #$FF                                 ;Invert to get negative
    inc A                                       ;Add one to value
    mov COM_TriangleStatePanbr+X, A             ;Return back to triangle counter
    mov A, COM_TriangleSignHolder               ;Shove sign holder into A
    mov (COM_TriangleCounterPanbr)+X, A       ;Shove back into triangle counter array
    .ContinueEffectsPanbr:
    clrv                            ;Clear half carry to avoid further flag misery
    ret

    ;-----------------------;
    ; Signed Multiplication ;
    ;-----------------------;
    ;   Note of usage:
    ;       To use the Signed mult, use CALL to call the subroutine 
    ;       where YA is to be used for said multiplication
    ;
    ;       [but not using MUL YA, since it doesn't take into account the sign
    ;        Provided by AArt1256]
    ;
SignedMul:
    ; save multipliers into temporary memory
    mov COM_TempMemADDRL, A
    mov COM_TempMemADDRH, Y
    mul YA

    ; save product into temporary memory
    mov COM_MulProductTemp, A
    mov COM_MulProductTemp+1, Y

    ; check for MSB
    mov A, COM_TempMemADDRL
    bpl skip_mul1
    setc
    mov A, COM_MulProductTemp+1
    sbc A, COM_TempMemADDRH
    mov COM_MulProductTemp+1, A
    skip_mul1:

    ; check for MSB
    mov A, COM_TempMemADDRH
    bpl skip_mul2
    setc
    mov A, COM_MulProductTemp+1
    sbc A, COM_TempMemADDRL
    mov COM_MulProductTemp+1, A
    skip_mul2:

    mov A, COM_MulProductTemp
    mov Y, COM_MulProductTemp+1
    ret

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

SequenceMemory:
    .Seq1:
    %SetSpeed($7F)
    %SetNoise($1F)
    %SetDelayTime(4)
    %SetDelayVolume($20, $20)
    %SetDelayFeedback($50)
    %SetMasterVolume($7F, $7F)
    %SetChannelVolume(0, $7F, $7F)
    %SetChannelVolume(1, $6F, $6F)
    %SetChannelVolume(2, $5F, $5F)
    %SetChannelVolume(3, $4F, $4F)
    %SetChannelVolume(4, $3F, $3F)
    %SetChannelVolume(5, $2F, $2F)
    %SetChannelVolume(6, $1F, $1F)
    %SetChannelVolume(7, $0F, $0F)
    %SetInstrument(0, $0)
    %SetInstrument(1, $1)
    ;%SetPabr(0, $4F)
    ;%SetTremo(0, $FF)
    ;%SetVib(0, $44)
    %SetDelayCoefficient(0, $40)
    %SetDelayCoefficient(1, $20)
    %SetDelayCoefficient(2, $10)
    %SetDelayCoefficient(3, $08)
    %SetDelayCoefficient(4, $04)
    %SetDelayCoefficient(5, $02)
    %SetDelayCoefficient(6, $01)
    %SetDelayCoefficient(7, $00)
    %PlayPitch($8008, $0)
    ;%PlayPitch($8004, $1)
    db COM_EndRow

PatternMemory:
    

Engine_End:

arch 65816
base ROM_Engine_Start+(Engine_End-DriverStart)
ROM_Engine_End: