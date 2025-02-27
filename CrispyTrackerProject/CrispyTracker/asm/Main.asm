;|==============================|
;|                              |
;|  Cobalt Engine Test Program  |
;|                              |
;|          08/06/2024          |
;|                              |
;|==============================|

hirom                           ;Sets the SNES mapping mode to HIROM

incsrc "hardware.asm"           ;Include namespace HW which contains names for each memory and register addr in the SNES
incsrc "DriverV2.asm"           ;Includes the audio driver Cobalt
incsrc "main-h.asm"             ;Includes Test 

arch 65816                      ;sets the architecture to the 65816 CPU on the SNES

org $8000                       ;Go to 0x8000
Reset:
    ;Now we initialise the SNES itself, since we've written the ROM header
    sei                             ;Disable interrupt registers
    stz.w HW_NMITIMEN               ;Store 0 in HW_NMITIMEN
    stz.w HW_HDMAEN                 ;Store 0 in HW_HDMAEN
    clc                             ;Clears carry flag to 0
    xce                             ;Take the 0 in clc and shove into emulation flag, thus exiting 6502 mode
    rep #%00011000                  ;Turn off decimal mode and we set the X register to 16 bits
    sep #%00100000                  ;Set the M flag to 8 bit, used for accumulator and memory instructions
    ldx.w #$1FFF                    ;Set stack over to 0x1FFF
    txs                             ;Send X register to stack
    pea.w 0                         ;Writes 2 0bytes to the stack [no # needed as every value is a direct load]
    pld                             ;Take value from stack and shove to D
    lda.b #1                        ;Load 1 into A
    sta.w HW_MEMSEL                 ;Write A value to HW_MEMSEL, mainly to speed up the CPU code
    jml .ResetOffset                ;Jump to start of program code

.ResetOffset
    phk                         ;Push to stack
    plb                         ;Pull B

    ;Now we turn the screen black to avoid garbage data being written to the screen
    lda.b #%10000000            ;Load 80 into A
    sta.w HW_INIDISP            ;Sends A value to HW_INIDISP

    jsr LoadDriver              ;Load file into SPC

    sep #$20
    lda.b #$80
    sta.w HW_NMITIMEN
    sep #$20                    ;Set A to 8bit
    stz.w HW_APUI00
    stz.w HW_APUI01
    stz.w HW_APUI02
    stz.w HW_APUI03
    stz.w MZP.MusicPlayed
    stz.w MZP.MusicSetup
    
    ldx.w #MusicComTable
    stx.b AudPtr

    ;Play music
    lda.b #$00
    sta.b MComIndex
    sta.b MComVal
    jsr AddMCom
    ;Play SFX
    lda.b #$01
    sta.b MComIndex
    sta.b MComVal
    jsr AddMCom

    ;jsr PlayMusic

SendTune:
    stz.w MZP.NMIDone
MainLoop:
    lda.b LoadingDriver
    bne +
    jsr ExecuteMCom
    +
    -
    lda.w MZP.NMIDone
    bne SendTune
    bra -

NMIDriverTest:
    sep #$20
    inc.w MZP.NMIDone
    inc.w MZP.SFXTimer
    lda.w MZP.SFXTimer
    cmp.b #$F0
    bne .SkipTimer
    stz.w MZP.SFXTimer
    ;Reset APU
    ;%WriteMCom($0A, $01)
    ;Play SFX
    %WriteMCom($01, $01)
    %WriteMCom($01, $00)
    ;;Set settings byte
    ;%WriteMCom($03, $03)
    ;;Mute channel 0
    ;%WriteMCom($05, $01)
    
    ;;Pause track
    ;%WriteMCom($06, $01)
    ;;Set fade speed
    ;%WriteMCom($09, $FF)
    ;;Set Master Volume
    ;%WriteMCom($02, $00)
    ;;Fade
    ;%WriteMCom($07, $00)
    .SkipTimer:
    rts

    ;Add value to Music Command table
AddMCom:
    pha
    phx
    phy
    php
    sep #$20
    lda.b MComIndex
    sta.b (AudPtr)
    rep #$20
    inc.b AudPtr
    sep #$20
    lda.b MComVal
    sta.b (AudPtr)
    rep #$20
    inc.b AudPtr
    sep #$20
    lda.b #!MComEnd
    sta.b (AudPtr)
    plp
    ply
    plx
    pla
    rts

    ;Loop and execute Music commands until $FF byte is reached
ExecuteMCom:
    pha
    phx
    phy
    php
    sep #$20
    ldx.w #MusicComTable
    stx.b AudPtr
    .ReadCom:
    ;Check previous command was executed
    lda.w HW_APUI01
    cmp.w MZP.SFXRec
    bne .ReadCom
    lda.b (AudPtr)
    cmp #!MComEnd
    beq .Finished       ;Jump out if finish byte detected
    sta.w HW_APUI02     ;Command index
    rep #$20
    inc.b AudPtr
    sep #$20
    ;Send command data to APU
    lda.b (AudPtr)
    sta.w HW_APUI00     ;Command value
    lda.w MZP.SFXRec
    sta.w HW_APUI01
    inc.w MZP.SFXRec
    -    
    lda.w HW_APUI03     ;Check RESET flag
    beq +
    jsr LoadDriver
    bra .Finished
    +
    lda.w HW_APUI01     ;Wait to make sure commands are synced with SPC-700 update
    cmp.w MZP.SFXRec
    bne -
    ;Increment pointer to next command
    rep #$20
    inc.b AudPtr
    sep #$20
    bra .ReadCom        ;Go back to check next command

    .Finished:
    ;Overwrite first command with NULL command to prevent constant retrigger
    lda.b #!MComEnd
    sta.w MusicComTable
    plp
    ply
    plx
    pla
    rts

NMIHandler:
    pha
    phx
    phy
    jsr.w NMIDriverTest
    ply
    plx
    pla
    rti


;This section of code is here to load the IPL rom into the SPC700 to have it start up
LoadDriver:
    pha
    phx
    phy
    php
    sep #$20
    lda.b #$01
    sta.b LoadingDriver
    .CheckPorts:
    lda.w HW_APUI00                     ;Load in the value held in APU-0
    cmp.b #$AA                          ;Compare if the current value in APU-0 is 0xAA
    bne .CheckPorts                     ;Go to .CheckPorts if the compare fails

    lda.w HW_APUI01                     ;Load in the value held in APU-1
    cmp.b #$BB                          ;Compare if the current value in APU-1 is 0xBB
    bne .CheckPorts                     ;Go to .CheckPorts if the compare fails

    .WriteToPorts:
    lda.b #DriverStart&$FF              ;Load last byte to the A register
    sta.w HW_APUI02                     ;Sends A value into HW_APUI02
    lda.b #(DriverStart>>8)&$FF         ;Load first byte to the A register
    sta.w HW_APUI03                     ;Sends A value into HW_APUI03
    lda.b #$01                          ;Load first byte to the A register
    sta.w HW_APUI01                     ;Sends A value into HW_APUI01
    lda.b #$CC                          ;Load first byte to the A register
    sta.w HW_APUI00                     ;Sends A value into HW_APUI00

    .ReadPort0:
    lda.w HW_APUI00                     ;Load in the value held in APU-0
    cmp.b #$CC                          ;Compare if the current value in APU-0 is 0xCC
    bne .ReadPort0                      ;Go to .ReadPort0 if the compare fails

    ;Load driver
    ldx.w #(ROM_Engine_Start)&$FFFF
    stx.b $00
    lda.b #(ROM_Engine_Start>>16)&$FF
    sta.b $02

    ldy.w #0                            ;Reset Y register
    ldx.w #ROM_Engine_End               ;Sets the X register to the last byte

    .TransferLoop:
    lda.b [0],Y                         ;Takes address from zeropage, adds Y and then loads from the generated addr
    sta.w HW_APUI01                     ;Write address to APU-1
    tya                                 ;Transfer Y data to A
    sta.w HW_APUI00                     ;Store value into APU-0
    iny                                 ;Increment Y

    .Port0Wait:
    cmp.w HW_APUI00                     ;Check if we have got the right value into APU-0
    bne .Port0Wait

    cpy.w #ROM_Engine_End               ;Check if the Y register is the same as the ROM end point
    bne .TransferLoop                   ;If it fails, go back to the transfer loop

    lda.b #DriverStart&$FF              ;Load 00 into A
    sta.w HW_APUI02                     ;Write A value into APU-2
    lda.b #(DriverStart>>8)&$FF         ;Load 02 into A
    sta.w HW_APUI03                     ;Write A value into APU-3
    stz.w HW_APUI01                     ;Write zeropage value into APU-1

    .NonZeroCheck:
    iny                                 ;Increment Y
    tya                                 ;Transfer Y to A
    beq .NonZeroCheck
    sta.w HW_APUI00                     ;Write increment to APU-0

    .CheckIfTransferDone
    cmp.w HW_APUI00                     ;Check if we have got the right value into APU-0
    bne .CheckIfTransferDone
    
    lda.b #$01                          ;Reset SFXRec
    sta.w MZP.SFXRec
    stz.b LoadingDriver
    plp
    ply
    plx
    pla
    rts

                                ;First half of the header
org $FFB0                       ;Goto FFB0
db "00"                         ;ROM Region
db "CAUD"                       ;Unique code for identification
fill 6                          ;Fill 6 bytes
db 0,0,0,0
                                ;Second half of header
db "Cobalt Engine Aud Dvr"      ;Program name [21 characters long]
db $31                          ;Set ROM identification and speed
db 0                            ;Set what we have available [in this case ROM]
db 8                            ;Set ROM size [256k]
db 0                            ;Set RAM size [0k]
db 2                            ;Set region [2 for PAL]
db $33                          ;Old ID code [replaced with new value]
db 0                            ;ROM version
dw $FFFF
dw $0000

org $FFE0                       ;Goto 0xFFE0

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