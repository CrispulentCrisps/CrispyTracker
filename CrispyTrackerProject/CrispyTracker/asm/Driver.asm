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

arch spc700     ;Set architecture to SPC-700

org $C10000     ;Go to bank C1000

ROM_Engine_Start:

base $0200          ;Set audio driver code to 0x0200 [closest to the start of memory we can get away with]

DriverLoop:         ;Main driver loop

.TickRoutine:       ;Routine for incrementing the tick counter and postiion within the track
    

bra DriverLoop

Engine_End:

arch 65816
base ROM_Engine_Start+(Engine_End-DriverLoop)
ROM_Engine_End: