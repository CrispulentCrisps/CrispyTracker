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
    jml $800000+(.ResetOffset&$FFFF);Jump to $80 in terms of memory X axis

.ResetOffset
    phk                         ;Push to stack
    plb                         ;Pull B

    ;Now we turn the screen black to avoid garbage data being written to the screen
    lda.b #%10000000            ;Load 80 into A
    sta.w HW_INIDISP            ;Sends A value to HW_INIDISP

                                ;This section of code is here to load the IPL rom into the SPC700 to have it start up
LoadDriver:

.CheckPorts:
    lda.w HW_APUI00             ;Load in the value held in APU-0
    cmp.b #$AA                  ;Compare if the current value in APU-0 is 0xAA
    bne .CheckPorts             ;Go to .CheckPorts if the compare fails

    lda.w HW_APUI01             ;Load in the value held in APU-1
    cmp.b #$BB                  ;Compare if the current value in APU-1 is 0xBB
    bne .CheckPorts             ;Go to .CheckPorts if the compare fails

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

    lda.b #ROM_Engine_Start&$FF         ;Loads first byte of the address into A
    sta.b $0                            ;Load 0 into zeropage
    lda.b #(ROM_Engine_Start>>8)&$FF    ;Loads second byte of the address into A
    sta.b $1                            ;Load 1 into zeropage
    lda.b #(ROM_Engine_Start>>16)&$FF   ;Loads third byte of the address into A
    sta.b $2                            ;Load 2 into zeropage

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
    
    sep #$20                    ;Set A to 8bit
    stz.w HW_APUI00
    stz.w HW_APUI01
    stz.w HW_APUI02
    stz.w HW_APUI03
    lda.b #$01
    sta.w MZP.SFXRec
    
MainLoop:
    lda.b #$80
    sta.w HW_NMITIMEN
    -
    bra -

NMIDriverTest:
    inc.w MZP.SFXTimer
    lda.w MZP.SFXTimer
    and #$80
    beq .SkipTimer
    lda.b #$00
    sta.w HW_APUI00     ;Subtune index
    lda.b #$01
    sta.w HW_APUI02     ;Audio type [SFX]
    lda.w MZP.SFXRec
    sta.w HW_APUI01
    inc.w MZP.SFXRec
    stz.w MZP.SFXTimer
    .SkipTimer:
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

                                ;First half of the header
org $FFB0                       ;Goto FFB0
db "00"                         ;ROM Region
db "CTTC"                       ;Unique code for identification
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