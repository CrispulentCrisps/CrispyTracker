incsrc "Cobalt-Vars.asm"

arch SPC700

base $0200
DriverStart:
    ;Setup CPU
    mov.b X, #$FF
    mov SP, X

    mov.b SPC_Test, #$0A
    mov.b SPC_Timer1, #$85  ;Set music timer to 60hz
    mov.b SPC_Timer2, #$2C  ;Set effects timer to 180hz
    mov.b SPC_Control, #$7F ;Set timers on
    mov.b SPC_APU0, #$00
    mov.b SPC_APU1, #$00
    mov.b SPC_APU2, #$00
    mov.b SPC_APU3, #$00

    ;Clear ZP and stack
    mov.b ZP.Ptr0, #$EF
    mov.b ZP.Ptr0+1, #$00
    mov X, #$00
    mov A, #$00
    -
    mov.b (ZP.Ptr0)+Y, A
    decw.b ZP.Ptr0
    bpl -

    ;Reset DSP values
    %spc_write(DSP_MVOL_L, $7F)
    %spc_write(DSP_MVOL_R, $7F)
    %spc_write(DSP_EVOL_L, $00)
    %spc_write(DSP_EVOL_R, $00)
    %spc_write(DSP_EFB, $00)
    %spc_write(DSP_EDL, $00)
    %spc_write(DSP_ESA, $00)
    %spc_write(DSP_DIR, !CodeBuffer>>8)
    %spc_write(DSP_PMON, $00)
    %spc_write(DSP_EON, $00)
    %spc_write(DSP_NON, $00)
    %spc_write(DSP_FLG, $20)

    
    ;TEST instrument
    ;%spc_write($00, $7F)
    ;%spc_write($01, $7F)
    ;%spc_write($02, $00)
    ;%spc_write($03, $10)
    ;%spc_write($04, $00)
    ;%spc_write($05, $00)
    ;%spc_write($06, $00)
    ;%spc_write($07, $7F)
    ;%spc_write(DSP_KON, $01)
    
    if !TRACKER == 0
        ;Set sequence pointers to first order in the table
        nop
        nop
        nop
        nop
        mov.b ZP.Ptr0, #(OrderTable)&$FF
        mov.b ZP.Ptr0+1, #(OrderTable>>8)&$FF
        mov.b ZP.PauseFlag, #$00
    else
        mov.b TuneStart, #(OrderTable)&$FF
        mov.b TuneStart+1, #(OrderTable>>8)&$FF
        mov.b ZP.Ptr0, TuneStart
        mov.b ZP.Ptr0+1, TuneStart+1
        mov.b ZP.PauseFlag, #$01
    endif

    mov.b Y, #(!ChannelCount*2)-1
    -
    mov.b A, (ZP.Ptr0)+Y
    mov.b ZP.ChSeq+Y, A
    dec Y
    bpl -
    
    mov.b ZP.R0, #$00
    mov.b ZP.R1, #$00
    mov.b X, #(!ChannelCount-1)*2
    -
    mov.b Y, #$00
    mov.b ZP.R0, X
    movw.b YA, ZP.R0
    addw.b YA, ZP.Ptr0
    mov.b ZP.ChOrder+1+X, Y
    mov.b ZP.ChOrder+X, A
    dec X
    dec X
    bpl -

    ;Main Timer loops
MainTimerLoop:
    mov.b A, SPC_Count1                 ;Music timer, set to update once a frame
    beq +
    mov.b A, ZP.PauseFlag
    bne +
    mov.b ZP.ChIndex, #!ChannelCount-1
    call MusicRoutine
    +
    mov.b A, SPC_Count2                 ;Effects timer, set to update 4 times a frame
    beq +
    mov.b ZP.ChIndex, #!ChannelCount-1
    call EffectsRoutine
    +
    mov.b A, SPC_APU0
    beq +
    call CommunicationRoutine
    +
    bra MainTimerLoop


MusicRoutine:
    ;Decrement Speed timer
    mov.b A, ZP.MusTimer
    beq +
    dec.b ZP.MusTimer
    jmp .SkipChannelUpdates
    +

    ;Reset mirrors
    mov.b ZP.KON, #$00
    mov.b ZP.KOFF, #$00
    mov.b ZP.EON, #$00
    mov.b ZP.NON, #$00
    mov.b ZP.PMON, #$00
    
    ;Do per-channel operations
    .ChLoop:

    mov.b X, ZP.MusSpeedSel
    mov.b A, ZP.MusSpeed1+X
    mov.b ZP.MusTimer, A
    eor.b ZP.MusSpeedSel, #$01

    mov.b X, ZP.ChIndex


    ;Decrement current timer
    mov A, ZP.ChTimer+X
    beq +
    dec.b ZP.ChTimer+X
    +
    ;Check if timer == 0
    bne .SkipSeqRead
    ;Read new sequence for given track
    call ReadSeqData   
    .SkipSeqRead:
    dec.b ZP.ChIndex
    bpl .ChLoop

    ;Transfer changes to correct registers
        ;Apply changes for given channel
    mov.b X, #$07

    .InstWrite:
    ;Apply instrument changes
    mov.b ZP.Ptr0, #(InstrumentTable)&$FF
    mov.b ZP.Ptr0+1, #(InstrumentTable>>8)&$FF
    mov.b A, ZP.ChInst+X
    mov.b Y, #!InstSize           ;Size of instrument structure
    mul YA
    addw YA, ZP.Ptr0        ;Instrument pointer
    movw.b ZP.Ptr0, YA

    mov.b Y, #$03
    mov.b A, X
    xcn
    or.b A, #$07            ;Reverse write loop, start at GAIN
    mov.b SPC_RegADDR, A

    .InstWriteLoop:
    mov.b A, (ZP.Ptr0)+Y
    mov.b SPC_RegData, A
    dec.b SPC_RegADDR
    dec Y
    bpl .InstWriteLoop

    ;Grab flags from current instrument
    mov.b Y, #$04
    mov.b A, (ZP.Ptr0)+Y
    mov.b ZP.R0, A
    ;Bit test the current flags

    ;Echo
    bbc ZP.R0.0, +
    mov.b A, ZP.EON
    or.w A, BitfieldTable+X
    mov.b ZP.EON, A
    +
    
    ;Noise
    bbc ZP.R0.1, +
    mov.b A, ZP.NON
    or.w A, BitfieldTable+X
    mov.b ZP.NON, A
    +
    
    ;Pitch modulation
    bbc ZP.R0.2, +
    mov.b A, ZP.PMON
    or.w A, BitfieldTable+X
    mov.b ZP.PMON, A
    +

    dec.b X
    bpl .InstWrite

    ;Apply changes to output
    mov.b SPC_RegADDR, #DSP_EON
    mov.b SPC_RegData, ZP.EON
    mov.b SPC_RegADDR, #DSP_NON
    mov.b SPC_RegData, ZP.NON
    mov.b SPC_RegADDR, #DSP_PMON
    mov.b SPC_RegData, ZP.PMON
    
    mov.b SPC_RegADDR, #DSP_KON
    mov.b SPC_RegData, ZP.KON
    mov.b SPC_RegADDR, #DSP_KOF
    mov.b SPC_RegData, ZP.KOFF

    .SkipChannelUpdates:
    ret

ReadSeqData:
    .Readloop:
    %ReadSeqVal()
    asl A
    mov.b X, A
    jmp (SequenceCommands+X)
    bra .Readloop

SequenceCommands:
    dw Com_Sleep
    dw Com_Jump
    dw Com_Break
    dw Com_Inst
    dw Com_Pitch
    dw Com_Volume
    dw Com_Speed1
    dw Com_Speed2
    dw Com_Vibrato
    dw Com_Portamento
    dw Com_VolumeSlide

    ;
    ;   Description:
    ;       Sets a sleep timer, this forces the given channel sequence stream to wait N frames before beginning the next sequence read
    ;
    ;   Assumptions:
    ;       This command is a Termination command, meaning the sequence will stop reading once this command is executed
    ;       Due to this, it is best to have this be at the end of a given sequence stream
    ;
Com_Sleep:
    %ReadSeqVal()
    mov.b X, ZP.ChIndex
    mov.b ZP.ChTimer+X, A
    ret
    
    ;
    ;   Description:
    ;       Sets the order read for the current channel sequence stream
    ;
    ;   Assumptions:
    ;       This command is a Termination command, meaning the sequence will stop reading once this command is executed
    ;       Due to this, it is best to have this be at the end of a given sequence stream
    ;
Com_Jump:
    %ReadSeqVal()
    mov.b ZP.R0, A
    %ReadSeqVal()
    mov.b ZP.R1, A

    mov.b A, ZP.R0
    mov.b ZP.ChOrder+X, A
    mov.b A, ZP.R1
    mov.b ZP.ChOrder+1+X, A
    
    mov.b A, X
    mov.b Y, A
    mov.b A, (ZP.R0)+Y
    mov.b ZP.ChSeq+X, A
    inc Y
    mov.b A, (ZP.R0)+Y
    mov.b ZP.ChSeq+1+X, A
    ret

    ;
    ;   Description:
    ;       Sets
    ;
    ;   Assumptions:
    ;       This command is a Termination command, meaning the sequence will stop reading once this command is executed
    ;       Due to this, it is best to have this be at the end of a given sequence stream
    ;
Com_Break:
    mov.b A, ZP.ChIndex
    asl A
    mov.b X, A

    mov.b A, ZP.ChOrder+X
    adc.b A, #$10
    bcc +
    inc.b ZP.ChOrder+1+X
    +
    mov.b ZP.ChOrder+X, A
    
    mov.b A, ZP.ChOrder+X
    mov.b ZP.R0, A
    mov.b A, ZP.ChOrder+1+X
    mov.b ZP.R1, A

    mov.b Y, #$00
    mov.b A, (ZP.R0)+Y
    mov.b ZP.ChSeq+X, A
    inc Y
    mov.b A, (ZP.R0)+Y
    mov.b ZP.ChSeq+1+X, A
    ret

Com_Inst:
    %ReadSeqVal()
    mov.b X, ZP.ChIndex
    mov.b ZP.ChInst+X, A
    jmp ReadSeqData

Com_Pitch:
    %ReadSeqVal()
    mov.b ZP.ChPitch+X, A
    %ReadSeqVal()
    mov.b ZP.ChPitch+1+X, A
    
    mov.b X, ZP.ChIndex
    mov.b A, ZP.KON
    or.w A, BitfieldTable+X
    mov.b ZP.KON, A
    jmp ReadSeqData

Com_Volume:
    %ReadSeqVal()
    mov.b ZP.ChVol+X, A
    %ReadSeqVal()
    mov.b ZP.ChVol+1+X, A
    jmp ReadSeqData

Com_Speed1:
    %ReadSeqVal()
    mov.b ZP.MusSpeed1, A
    jmp ReadSeqData

Com_Speed2:
    %ReadSeqVal()
    mov.b ZP.MusSpeed2, A
    jmp ReadSeqData

Com_Vibrato:
    %ReadSeqVal()
    mov.b X, ZP.ChIndex
    mov.b ZP.ChVibVal+X, A
    mov.b A, #$00
    mov.b ZP.ChVibPhase+X, A
    jmp ReadSeqData
    
Com_Portamento:
    %ReadSeqVal()
    mov.b X, ZP.ChIndex
    mov.b ZP.ChPortVal+X, A
    jmp ReadSeqData

Com_VolumeSlide:
    %ReadSeqVal()
    mov.b X, ZP.ChIndex
    mov.b ZP.ChVolSlideVal+X, A
    jmp ReadSeqData

    ;
    ;   Communication routine
    ;       Non-zero command is sent via SPC_APU0
    ;       Zero command send back to CPU
    ;       Once command has been read in it will send back the same command value
    ;       that was put into SPC_APU0
    ;
CommunicationRoutine:
    mov.b SPC_APU0, #!PRoCom_NULL
    mov.b ZP.R0, SPC_APU0
    mov.b A, ZP.R0
    dec.b A
    asl A
    mov.b X, A
    jmp (ProComActions+X)
ComReturn:
    mov.b SPC_APU0, ZP.R0   ;Return non-zero value back to CPU
    ret

ProComActions:
    dw ProCom_Load
    dw ProCom_PlaySFX
    dw ProCom_FadeTune
    dw ProCom_SetMaster

    ;
    ;   Tune loader routine:
    ;
    ;       Setup:
    ;           The driver will send $FF to APU03 to request a word of data from the
    ;           main CPU, it will then send $00 back to wait while it transfer the word
    ;
ProCom_Load:
    %spc_write(DSP_FLG, $E0)
    mov.b SPC_Control, #$00
    ;Initialise absolute moves with tune starting address
    mov.b A, TuneStart
    mov.b Y, TuneStart+1
    movw.b ZP.R5, YA
    mov.w .Mov0+1, A
    mov.w .Mov0+2, Y
    mov.w .Mov1+1, A
    mov.w .Mov1+2, Y
    mov.w .Mov2+1, A
    mov.w .Mov2+2, Y
    mov.w .Mov3+1, A
    mov.w .Mov3+2, Y
    
    mov.b ZP.R2, #$00
    mov.b ZP.R3, #$04
    mov.b X, #$00

    ;Make sure both sides are ready for transfer
    mov.b ZP.R0, SPC_APU1
    bne +
    inc.b ZP.R0
    +
    mov.b ZP.R1, SPC_APU2
    mov.b SPC_APU3, #$FF    ;Start transfer

    -
    mov.b A, SPC_APU0       ;3 cycles
    .Mov0:
    mov.w .Mov0+X, A          ;5 cycles
    inc X

    mov.b A, SPC_APU1       ;3 cycles
    .Mov1:
    mov.w .Mov1+X, A        ;5 cycles

    mov.b A, SPC_APU2       ;3 cycles
    .Mov2:
    mov.w .Mov2+X, A          ;5 cycles
    inc X

    mov.b A, SPC_APU3       ;3 cycles
    .Mov3:
    mov.w .Mov3+X, A        ;5 cycles
    inc X
    mov.b SPC_APU3, X       ;4 cycles
    
    bne +
    inc.w .Mov0+2           ;3 cycles
    inc.w .Mov1+2           ;3 cycles
    inc.w .Mov2+2           ;3 cycles
    inc.w .Mov3+2           ;3 cycles
    +
        

    dec.b ZP.R0             ;3 cycles
    bne +                   ;2/4 cycl
    setc                    ;1 cycle
    sbc.b ZP.R1, #$01       ;6 cycles
    bcs +                   ;2/4 cycles
    bra .TransferFinished   ;4 cycles, Escape when top and byte is negative
    +
    bra -                   ;4 cyles
    .TransferFinished:
    ;Tune now loaded, read header information to setup tune

    jmp ComReturn

ProCom_PlaySFX:

    jmp ComReturn

ProCom_FadeTune:

    jmp ComReturn

ProCom_SetMaster:

    jmp ComReturn

EffectsRoutine:
    nop
    .FxLoop:
    mov.b A, ZP.ChIndex
    asl A
    mov.b X, A
    
    ;Setup temporary values to process
    mov.b A, ZP.ChPitch+X
    mov.b ZP.OutPitch, A
    mov.b A, ZP.ChPitch+1+X
    mov.b ZP.OutPitch+1, A

    mov.b A, ZP.ChVol+X
    mov.b ZP.OutVol, A
    mov.b A, ZP.ChVol+1+X
    mov.b ZP.OutVol+1, A

    ;--------------------;
    ;     Portamento     ;
    ;--------------------;
    clrc
    mov.b ZP.R1, #$00
    mov.b X, ZP.ChIndex
    mov.b A, ZP.ChPortVal+X
    asl A
    bcc +
    eor.b ZP.R1, #$FF
    +
    mov.b ZP.R0, A

    mov.b A, X
    asl A
    mov.b Y, A
    mov.b A, ZP.ChPitch+Y
    mov.b ZP.R2, A
    mov.b A, ZP.ChPitch+1+Y
    mov.b ZP.R3, A

    movw YA, ZP.R2
    addw YA, ZP.R0
    movw ZP.R2, YA
    
    mov.b A, X
    asl A
    mov.b Y, A
    mov.b A, ZP.R2
    mov.b ZP.ChPitch+Y, A
    mov.b A, ZP.R3
    mov.b ZP.ChPitch+1+Y, A

    ;-----------------;
    ;     Vibrato     ;
    ;-----------------;
    mov.b X, ZP.ChIndex
    mov.b A, ZP.ChVibVal+X
    and.b A, #$0F
    adc.b A, ZP.ChVibPhase+X
    mov.b ZP.ChVibPhase+X, A
    mov.b Y, A
    mov.w A, SineTable+Y
    mov.b Y, A
    mov.b A, ZP.ChVibVal+X
    and.b A, #$F0
    xcn
    mul YA
    addw YA, ZP.OutPitch
    movw ZP.OutPitch, YA    

    ;-----------------------;
    ;     Volume slides     ;
    ;-----------------------;
    mov.b A, ZP.ChVolSlideCnt+X
    beq +
    dec A
    +
    mov.b ZP.ChVolSlideCnt+X, A
    beq +
    jmp .SkipVolSlide
    +
    mov.b A, ZP.ChVolSlideVal+X
    and.b A, #$0F
    mov.b ZP.ChVolSlideCnt+X, A

    mov.b ZP.R4, #$00           ;Channel signs
    mov.b A, X
    asl A
    mov.b Y, A
    mov.b A, ZP.ChVol+Y
    bpl +
    or.b ZP.R4, #$01
    eor.b A, #$FF
    inc A
    +
    mov.b ZP.R0, A
    mov.b A, ZP.ChVol+1+Y
    bpl +
    or.b ZP.R4, #$02
    eor.b A, #$FF
    inc A
    +
    mov.b ZP.R1, A

    mov.b ZP.R2, #$00
    mov.b A, ZP.ChVolSlideVal+X
    beq .SkipVolSlideDelta
    mov.b ZP.R2, #$01
    bpl .SkipVolSlideDelta
    mov.b ZP.R2, #$FF
    .SkipVolSlideDelta:

    mov.b A, ZP.R0
    clrc
    adc.b A, ZP.R2
    bpl .SkipClampL
    bbs ZP.R2.7, ++
    mov.b A, #$7F
    bra .SkipClampL
    ++
    mov.b A, #$00
    .SkipClampL:
    mov.b ZP.R0, A
    
    mov.b A, ZP.R1
    clrc
    adc.b A, ZP.R2
    bpl .SkipClampR
    bbs ZP.R2.7, ++
    mov.b A, #$7F
    bra .SkipClampR
    ++
    mov.b A, #$00
    .SkipClampR:
    mov.b ZP.R1, A
    
    ;Set channel volume
    bbc ZP.R4.0, +
    eor.b ZP.R0, #$FF
    inc.b ZP.R0
    +
    
    bbc ZP.R4.1, +
    eor.b ZP.R1, #$FF
    inc.b ZP.R1
    +
    
    mov.b A, ZP.R0
    mov.b ZP.ChVol+Y, A
    mov.b A, ZP.R1
    mov.b ZP.ChVol+1+Y, A
    .SkipVolSlide:


    ;Output processed pitch and volume into the corresponding channel
    mov.b A, ZP.ChIndex
    xcn
    mov.b SPC_RegADDR, A
    mov.b SPC_RegData, ZP.OutVol
    inc.b SPC_RegADDR
    mov.b SPC_RegData, ZP.OutVol+1
    
    inc.b SPC_RegADDR
    mov.b SPC_RegData, ZP.OutPitch
    inc.b SPC_RegADDR
    mov.b SPC_RegData, ZP.OutPitch+1
    dec.b ZP.ChIndex
    bmi +
    jmp .FxLoop
    +
    ret

    ;u8 32 * sin(t * 2PI);
SineTable:
    db $20,$21,$22,$22,$23,$24,$25,$25,$26,$27
    db $28,$29,$29,$2A,$2B,$2C,$2C,$2D,$2E,$2E
    db $2F,$30,$30,$31,$32,$32,$33,$34,$34,$35
    db $35,$36,$37,$37,$38,$38,$39,$39,$3A,$3A
    db $3B,$3B,$3B,$3C,$3C,$3D,$3D,$3D,$3E,$3E
    db $3E,$3E,$3F,$3F,$3F,$3F,$3F,$40,$40,$40
    db $40,$40,$40,$40,$40,$40,$40,$40,$40,$40
    db $40,$40,$3F,$3F,$3F,$3F,$3F,$3E,$3E,$3E
    db $3E,$3D,$3D,$3D,$3C,$3C,$3B,$3B,$3B,$3A
    db $3A,$39,$39,$38,$38,$37,$37,$36,$35,$35
    db $34,$34,$33,$32,$32,$31,$30,$30,$2F,$2E
    db $2E,$2D,$2C,$2C,$2B,$2A,$29,$29,$28,$27
    db $26,$25,$25,$24,$23,$22,$22,$21,$20,$1F
    db $1E,$1E,$1D,$1C,$1B,$1B,$1A,$19,$18,$17
    db $17,$16,$15,$14,$14,$13,$12,$12,$11,$10
    db $10,$0F,$0E,$0E,$0D,$0C,$0C,$0B,$0B,$0A
    db $09,$09,$08,$08,$07,$07,$06,$06,$05,$05
    db $05,$04,$04,$03,$03,$03,$02,$02,$02,$02
    db $01,$01,$01,$01,$01,$00,$00,$00,$00,$00
    db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00
    db $01,$01,$01,$01,$01,$02,$02,$02,$02,$03
    db $03,$03,$04,$04,$05,$05,$05,$06,$06,$07
    db $07,$08,$08,$09,$09,$0A,$0B,$0B,$0C,$0C
    db $0D,$0E,$0E,$0F,$10,$10,$11,$12,$12,$13
    db $14,$14,$15,$16,$17,$17,$18,$19,$1A,$1B
    db $1B,$1C,$1D,$1E,$1E,$1F

BitfieldTable:
    db $01
    db $02
    db $04
    db $08
    db $10
    db $20
    db $40
    db $80

fill !CodeBuffer-pc()
assert pc() == !CodeBuffer

DirTable:                          ;These are just for debugging purposes
;Test sine+saw sample + dir page
db $08,(!CodeBuffer>>8),$08,(!CodeBuffer>>8),$23,(!CodeBuffer>>8),$23,(!CodeBuffer>>8)

SampleTable:                        ;These are just for debugging purposes
db $84, $17, $45, $35, $22, $22, $31, $21, $10, $68, $01, $21, $0D, $01, $08, $0B, $C3, $3E, $5B, $09, $8B, $D7
db $B1, $E0, $BC, $AF, $78
db $B8, $87, $1F, $00, $F1, $0F, $1F, $00, $00, $8F, $E1, $13, $12, $2D, $52, $14, $10, $F7

InstrumentTable:
    %WriteInstrument($00, $9F, $F2, $007F, $00)
    %WriteInstrument($01, $FF, $EA, $0060, $00)

OrderTable:
    ;Order 0
    .Order0:
        dw PatternMem_Pattern_0
        dw PatternMem_Pattern_2
        dw PatternMem_Pattern_2
        dw PatternMem_Pattern_2
        dw PatternMem_Pattern_2
        dw PatternMem_Pattern_2
        dw PatternMem_Pattern_2
        dw PatternMem_Pattern_2

    .Order1:
        dw PatternMem_Pattern_1
        dw PatternMem_Pattern_2
        dw PatternMem_Pattern_2
        dw PatternMem_Pattern_2
        dw PatternMem_Pattern_2
        dw PatternMem_Pattern_2
        dw PatternMem_Pattern_2
        dw PatternMem_Pattern_2

PatternMem:
    .Pattern_0:
        %WriteComByte(!COM_VIBRATO, $00)
        %WriteComByte(!COM_SPEED1, $06)
        %WriteComByte(!COM_SPEED2, $06)
        %WriteComWord(!COM_VOLUME, $4040)
        %WriteComByte(!COM_INST, $00)
        %WriteComWord(!COM_PITCH, $0200)
        %WriteComByte(!COM_SLEEP, $08)
        %WriteComWord(!COM_PITCH, $0400)
        %WriteComByte(!COM_SLEEP, $08)
        %WriteComWord(!COM_PITCH, $0C00)
        %WriteComByte(!COM_SLEEP, $08)
        %WriteComWord(!COM_PITCH, $1000)
        %WriteComByte(!COM_SLEEP, $08)
        %WriteCom(!COM_BREAK)
    .Pattern_1:
        %WriteComByte(!COM_INST, $01)
        %WriteComWord(!COM_PITCH, $1600)
        %WriteComByte(!COM_SLEEP, $04)
        %WriteComByte(!COM_VIBRATO, $48)
        %WriteComByte(!COM_SLEEP, $10)
        %WriteComByte(!COM_VIBRATO, $FF)
        %WriteComByte(!COM_SLEEP, $08)
        %WriteComWord(!COM_JUMP, OrderTable_Order0)
    .Pattern_2:
        %WriteComByte(!COM_INST, $00)
        %WriteComWord(!COM_VOLUME, $0000)
        %WriteComByte(!COM_SLEEP, $01)
        %WriteComWord(!COM_JUMP, OrderTable_Order0)
    .Pattern_3:
        %WriteComByte(!COM_VOLSLIDE, $82)
        %WriteComByte(!COM_SPEED1, $06)
        %WriteComByte(!COM_SPEED2, $06)
        %WriteComByte(!COM_INST, $01)
        %WriteComWord(!COM_VOLUME, $817F)
        %WriteComWord(!COM_PITCH, $0C00)
        %WriteComByte(!COM_SLEEP, $04)
        %WriteComWord(!COM_JUMP, OrderTable_Order0)

DriverEnd:
