hirom

incsrc "../hardware.asm"           ;Include namespace HW which contains names for each memory and register addr in the SNES
incsrc "65C816-Vars.asm"

arch 65816

org !EntryBank

Reset:
    sei
    stz.w HW_NMITIMEN               ;Store 0 in HW_NMITIMEN
    stz.w HW_HDMAEN                 ;Store 0 in HW_HDMAEN
    clc
    xce
    rep #$FF
    ldx.w #$01FF
    txs

    jml MainCode

org !CodeBank

MainCode:

    jsr LoadMusic

    ldx.w #$001F
    -
    jml +
    +
    dex
    bpl -

    sep #$20
    lda.b #$01      ;Load tune
    sta.w HW_APUI00

    jsr LoadTune

    -
    jmp -

NMIHandler:
    rti

LoadTune:
    php
    phb
    sep #$20
    lda.b #bank(TuneData)
    pha
    plb
    rep #$10
    rep #$20
    lda.w #TuneDataEnd-TuneData
    sta.l HW_APUI01
    tay
    ldx.w #$0000
    sep #$20
    lda.b #$FF
    sta.l HW_APUI03
    lda.b #$00
    sta.b MZP.R0
    -
    lda.l HW_APUI03
    inc
    bne -

    -
    rep #$20
    lda.w TuneData, X   ;6 cycles
    sta.l HW_APUI00     ;4 cycles
    inx
    inx
    lda.w TuneData, X   ;6 cycles
    sta.l HW_APUI02     ;4 cycles
    inx
    inx
    sep #$20
    --
    lda.l HW_APUI03
    cmp.b MZP.R0
    beq --
    sta.b MZP.R0
    rep #$20
    tya
    sec
    sbc.w #$0004
    tay
    bcs -

    plb
    plp
    rts

TuneData:
for i = 0..$1000
    dw !i
endfor
TuneDataEnd:

LoadMusic:
    php
    rep #$10
    sep #$20
    lda.b #$AA
    sta.w HW_APUI00
    lda.b #$BB
    sta.w HW_APUI01

    ldx.w #DriverCode
    stx.b MZP.Ptr0
    lda.b #bank(DriverCode)
    sta.b MZP.Ptr0+2

    .CheckPorts:
    lda.w HW_APUI00
    cmp.b #$AA
    bne .CheckPorts

    lda.w HW_APUI01
    cmp.b #$BB
    bne .CheckPorts

    rep #$21
    lda.w #DriverStart
    sta.w HW_APUI02
    sep #$20

    lda.b #$CC
    sta.w HW_APUI01
    sta.w HW_APUI00
    
    ;Check APU0 is $CC
    -
    cmp.w HW_APUI00
    bne -

    ldx.w #DriverEnd-DriverStart

    stz.b MZP.R0
    sep #$20
    .TransferLoop:
    lda.b [MZP.Ptr0]
    sta.w HW_APUI01
    lda.b MZP.R0
    sta.w HW_APUI00
    
    ;Check if counter has changed
    -
    lda.w HW_APUI00
    cmp.b MZP.R0
    bne -
    inc.b MZP.R0
    rep #$20
    inc.b MZP.Ptr0
    sep #$20
    dex
    bne .TransferLoop
    
    inc.b MZP.R0
    inc.b MZP.R0
    lda.b MZP.R0
    sta.w HW_APUI00
    
    ldx.w #$0200                ;Start of driver memory
    stx.w HW_APUI02

    lda.b #$00
    sta.w HW_APUI01

    .WaitCheck:
    lda.w HW_APUI00             ;Make sure SPC has aknowledged finished transfer
    bne .WaitCheck
    plp
    rts

org $FFC0
db "Cobalt driver program"
db $11  ;ROM type, FAST-Rom
db $00  ;
db $00, $00, $00, $00, $00, $00, $00, $00, $00

dw $FFFF                        ;Vector table
dw $FFFF
dw $FFFF
dw $FFFF
dw $FFFF
dw NMIHandler
dw $FFFF
dw $FFFF
dw $FFFF
dw $FFFF
dw $FFFF
dw $FFFF
dw $FFFF
dw $FFFF
dw Reset
dw $FFFF

org !MusicBank
DriverCode:
incsrc "Cobalt.asm"