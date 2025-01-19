;-----------------------;
;   65C816 variables    ;
;-----------------------;
AudPtr =        $0000
MComIndex =     $0002       ;What command to execute
MComVal =       $0003       ;Value assosiated with command

!MComEnd =      $FF

struct MZP      $0200
.SFXTimer       skip 1
.SFXRec         skip 1
.NMIDone        skip 1
.MusicPlayed    skip 1
.MusicSetup     skip 1
endstruct

MusicComTable =     $0400

macro WriteMCom(T, V)
lda.b #<T>
sta.b MComIndex
lda.b #<V>
sta.b MComVal
jsr AddMCom
endmacro