org $C08000

arch 65816
reset_handler:
    SEI                                 ; The very, very, very first thing to do is disable all
    STZ.W HW_NMITIMEN                   ; interrupts to make absolutely sure our initialization code
    STZ.W HW_HDMAEN                     ; is not interrupted and the handlers don't work with half-
                                        ; written data.

    LDA.B #$8F                          ; Then turn off the PPU so that we don't push garbage to the
    STA.W HW_INIDISP                    ; screen.

    CLC                                 ; Switch to native mode.
    XCE

    REP #$18                            ; Disable decimal mode and set index registers to 16 bits.
    SEP #$20                            ; Set accumulator and memory operations to 8 bits.

    LDX.W #$01FF                        ; Stack to 01FF
    TXS

    PEA.W $0000                         ; Direct page to 0000
    PLD

if HAS_FAST_ROM
    LDA.B #$01                          ; Enabler fast ROM access at bank 80 and onwards, then jump
    STA.W HW_MEMSEL                     ; there to make use of it.
    JML $800000+(.next&$FFFF)
.next:
    PHK                                 ; We also want fast ROM data reads to relocate the data bank
    PLB                                 ; registers to where we jumped.
endif

    JSR reset_ppu                       ; Reset the PPU to a base state, since we might be in a hot
                                        ; reset.

    STZ.W HW_CGADD                      ; Load a nice blue background into colour 0, the global
    LDA.B #$4582&$FF                    ; background colour.
    STA.W HW_CGDATA
    LDA.B #($4582>>8)&$FF
    STA.W HW_CGDATA

    LDX.W #rom_spc_engine_start&$FFFF   ; Store the three byte address to the SPC engine in D_A0,
    STX.B D_A0                          ; D_A2, and D_A2
    LDA.B #(rom_spc_engine_start>>16)&$FF
    STA.B D_A2
    LDY.W #spc_engine                   ; Place the destination address (0200) into Y and the 
                                        ; length in bytes into X.
    LDX.W #rom_spc_engine_end-rom_spc_engine_start
    REP #$30                            ; And finally the jump target into A after setting up a16 
    LDA.W #spc_engine                   ; mode.
    JSR upload_to_spc
    SEP #$20

    STZ.B D_VSYNC_COUNTER               ; Reset the VSync counter to 0 to let one VBlank happen
    LDA.B #$81                          ; before we start running real code. Then start running the
    STA.W HW_NMITIMEN                   ; NMI handler.

    CLI                                 ; Re-enable IRQ.
                                        ; Then we fall through into the main loop.

main_loop:
    WAI                                 ; Wait for the next interrupt to have happened. On non-lag
                                        ; frames this gives the VBlank routine a couple extra cycles
                                        ; because it doesn't have to wait for the last instruction
                                        ; of the busy loop to finish.
.loop:
    LDA.B D_VSYNC_COUNTER               ; Check whether the VSync counter is still negative, in that
    BMI main_loop                       ; case no VBlank has happened yet and we have to wait more.

    LDA.B #$FF                          ; Reset the VSync counter to -1 which signals the VBlank
    STA.B D_VSYNC_COUNTER               ; routine that it can run now.

    BRA .loop                           ; Jump past the WAI in case we are lagging so we don't
                                        ; potentially waste a frame.

macro interrupt_entry()
    SEI                                 ; Disable IRQs so we don't interrupt the interrupt.
if HAS_FAST_ROM                         ; If we have fast ROM jump to it for more SPEEEEED.
    JML $800000+(.next&$FFFF)
.next:
endif
    REP #$30                            ; Set up 16 bit registers so that we can push everything at
    PHA                                 ; once instead of one 8 bit half at a time.
    PHX
    PHY
    PHB


    PHK                                 ; We don't have to persist the program bank, instead we push
    PLB                                 ; it because it's a compact way of synchronizing the data
                                        ; bank with it.

    LDA.W #$0000                        ; Reset the direct pointer to 0
    TCD                                 
                                        ; Note that we stay in i16,x16 mode. If an interrupt handler
                                        ; wants something different it has to set that up itself.
endmacro

nmi_handler:
    %interrupt_entry()

    SEP #$20

    LDA.B #$80                          ; Blank the screen during the VBlank routine so if we
    STA.W HW_INIDISP                    ; overrun we don't create corrupted graphics, but only a
                                        ; black line at the top of the screen.

    INC.B D_VSYNC_COUNTER               ; The VSync counter is reset to -1 at the end of the main
    BNE .done                           ; loop, so if we increment it and overflow to 0 we know that
                                        ; the loop just finished and is ready for a proper VBank.

.done:
    LDA.B #$0F                          ; Turn the screen back on at the end of VBlank.
    STA.W HW_INIDISP

    BRA interrupt_return

irq_handler:
    %interrupt_entry()

interrupt_return:                       ; All interrupts share the same calling convention, so they
                                        ; can all use this same return procedure.
    REP #$30                            ; The interrupt might have fiddled with the register sizes,
    PLB                                 ; so reset them before pulling all of the register back off
    PLY                                 ; of the stack.
    PLX
    PLA

    RTI                                 ; Finally return from the interrupt.

reset_ppu:
    PHP                                 ; Persist the P register and set register sizes to a8,i16
    REP #$10
    SEP #$20

    LDA.B #$8F                          ; Blank the display so that we can modify PPU registers.
    STA.W HW_INIDISP

    STZ.W HW_OBSEL                      ; OBSEL = 0

    LDX.W #HW_BGMODE                    ; BGMODE-BG34NBA = 0
    LDY.W #8
    JSR .byte_fill_loop

    LDY.W #8                            ; BG1HOFS-BG4VOFS = 0
    JSR .latch_fill_loop

    LDX.W #HW_M7SEL                     ; All of the other mode 7 registers are write twice, so just
    STZ.W $0000, X                      ; handle the first one specially.
    INX
    LDY.W #6                            ; M7A-M7Y = 0
    JSR .latch_fill_loop

    LDX.W #HW_W12SEL                    ; W12SEL-TSW = 0
    LDY.W #13
    JSR .byte_fill_loop

    LDA.B #$30                          ; CGWSEL's default value is 30, not 00.
    STA.W HW_CGWSEL
    STZ.W HW_CGADSUB
    LDA.B #$E0                          ; Enable all colours.
    STA.W HW_COLDATA

    STZ.W HW_SETINI

    LDA.B #$80                          ; RAM->VRAM fill.
    STA.W HW_VMAIN
    STY.W HW_VMADDL
    STY.W HW_DAS0L
    LDX.W #(HW_VMDATAL&$FF)<<8|$09
    STX.W HW_DMAP0
    LDX.W #zero_value
    STX.W HW_A1T0L
    STZ.W HW_A1B0
    LDA.B #$01
    STA.W HW_MDMAEN

    STZ.W HW_CGADD                      ; RAM->CGRAM fill.
    LDX.W #$0200
    STX.W HW_DAS0L
    LDX.W #(HW_CGDATA&$FF)<<8|$0A
    STX.W HW_DMAP0
    STA.W HW_MDMAEN

    STY.W HW_OAMADDL                    ; RAM->OAM fill.
    LDX.W #$0220
    STX.W HW_DAS0L
    LDX.W #(HW_OAMDATA&$FF)<<8|$0A
    STX.W HW_DMAP0
    STA.W HW_MDMAEN

    PLP

    RTS
.byte_fill_loop:                        ; Helper functions to fill byte-by-byte or write zero twice
    STZ.W $0000, X                      ; for write-twice/latch registers. Both write to X and
    INX                                 ; increment it after each write, and decrement Y until it
    DEY                                 ; reaches zero.
    BNE .byte_fill_loop
    RTS
.latch_fill_loop:
    STZ.W $0000, X
    STZ.W $0000, X
    INX
    DEY
    BNE .byte_fill_loop
    RTS

zero_value: db $00, $00, $00, $00       ; Four zeroes for streaming through DMA.

upload_to_spc:                          ; Uploads a block of data to the SPC. The source address
                                        ; must be in D_A0, D_A1, and D_A2, the destination address
                                        ; in Y and the number of bytes to transfer in X, and the
                                        ; jump target at the end of the transfer in A. Clobbers all
                                        ; registers, but doesn't touch P.

    SEI                                 ; This function contains a lot of tight busy loops which can
                                        ; miss data if an interrupt occurs. So turn the IRQ off and
                                        ; assume that no NMI will happen.
    
    PHP                                 ; Preserve the P register because we will be messing with
    REP #$30                            ; register sizes. Then move to a16,i16. We should already be
                                        ; here, but don't technically have to be for very short
                                        ; copies.

    PHA                                 ; Store the jump address for the end of the procedure.

    LDA.W #$BBAA                        ; Wait for the SPC to become ready. This can be done with
-   CMP.W HW_APUI00                     ; 16 bit reads, but 16 bit writes are not permitted.
    BNE -

    TYA                                 ; Write the destination address into APU2 and APU3 by moving
    SEP #$20                            ; it into A, then setting a8 mode and first writing one half
    STA.W HW_APUI02                     ; of a, then switching out for B and doing the second half.
    XBA
    STA.W HW_APUI03

    LDA.B #$CC                          ; Load the kick byte, but also write it as the command. All
    STA.W HW_APUI01                     ; that matters is that the command is not zero, as that will
    STA.W HW_APUI00                     ; start a call on the SPC.

    LDY.W #$0000                        ; Reset Y to zero as the data pointer.

-   CMP.W HW_APUI00                     ; Wait for the SPC to acknowledged our kick.
    BNE -

.transfer_loop:
    LDA.B [D_A0],Y                      ; Read the next byte and pass it to the SPC. This will
    STA.W HW_APUI01                     ; never exceed the capabilities of one index register
                                        ; because the SPC only has 64k of RAM.
                         
    TYA                                 ; Move the lower half of Y to A to pass to the SPC as
    STA.W HW_APUI00                     ; notice that we have written the next byte. Then    
    INY                                 ; increment Y.

-   CMP.W HW_APUI00                     ; Wait for the SPC to acknowledge our byte.
    BNE -

    DEX                                 ; X now holds the remaining bytes, so decrement and check if
    BNE .transfer_loop                  ; we haven't hit zero yet. If that is the case jump back up.

    REP #$30                            ; Reload A with the jump address from the stack. To not lose
    PLA                                 ; the upper half we have to temporarily move to a16 mode.
    SEP #$20
    STA.W HW_APUI02                     ; The same trick as the beginning to store A in two eight
    XBA                                 ; bit stores.
    STA.W HW_APUI03

    STZ.W HW_APUI01                     ; Write zero as the command this time to tell the SPC that
                                        ; it will now jump somewhere.

    TYA                                 ; Get the lower half of Y (the data offset) into A and add
    INC                                 ; one to notify the SPC that we are done writing bytes.
    STA.W HW_APUI00

-   CMP.W HW_APUI00                     ; Wait for the SPC to acknowledge. This one may only be
    BNE -                               ; around for a few cycles, depending on the code that was
                                        ; uploaded so it is the reason why interrupts are disabled.

    PLP                                 ; Restore the pre-call state.
    CLI
    RTS

org $00FFB0

namespace header
db "00"                                 ; Developer ID '00' is common for home brew
db "XXXX"                               ; Game ID, uppercase ASCII
fillbyte $00
fill 6
db $00                                  ; Flash memory size
db $00                                  ; Extension ram size
db $00                                  ; "Special" version, whatever that means
db $00                                  ; Chipset subtype

title:
db "Test"                               ; Title, padded to 20 bytes
fillbyte ' '
fill 20-(pc()-$FFC0)
db $00                                  ; Zero here specifies that we have the extended header
db $21|(HAS_FAST_ROM<<4)                ; Speed and Map mode: 001X = Is fast, 1 = HiROM
db $00                                  ; Chipset 0 = ROM only
db $07                                  ; ROM size 7 = 128kb
db $00                                  ; RAM size
db $02                                  ; Country code 2 = Europe/PAL
db $33                                  ; Original developer id, 33 means reference extended header
db $00                                  ; Version
dw $FFFF                                ; Checksum complement
dw $0000                                ; Checksum

dw $FFFF                                ; WRAM-Boot if "XBOO"
dw $FFFF
dw $FFFF                                ; 65816 COP vector
dw $FFFF                                ; 65816 BRK vector
dw $FFFF                                ; 65816 ABORT vector
dw nmi_handler                          ; 65816 NMI vector
dw $FFFF                                ; -
dw irq_handler                          ; 65816 IRQ vector
dw $FFFF                                ; -
dw $FFFF                                ; -
dw $FFFF                                ; 6502 COP vector
dw $FFFF                                ; -
dw $FFFF                                ; 6502 ABORT vector
dw $FFFF                                ; 6502 NMI vector
dw reset_handler                        ; 6502 RESET vector
dw $FFFF                                ; 6502 IRQ vector
namespace off
