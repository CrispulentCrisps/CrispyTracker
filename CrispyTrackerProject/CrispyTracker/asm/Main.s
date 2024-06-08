;|==============================|
;|                              |
;|  Cobalt Engine Test Program  |
;|                              |
;|          08/06/2024          |
;|                              |
;|==============================|

hirom                           ;Sets the SNES mapping mode to HIROM
arch 65816                      ;sets the architecture to the 65816 CPU on the SNES

org $8000                       ;Go to 0x8000
Reset:
    bra Reset                   ;Branch to reset

org $FFB0                       ;Goto FFB0
db "00"                         ;Region
db "CTTC"                       ;Unique code for identification
fill 6                          ;Fill 6 bytes
db 0,0,0,0
                                ;Second half of header
db "Cobalt Engine Test Pg"      ;Program name [21 characters long]
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
dw $FFFF
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