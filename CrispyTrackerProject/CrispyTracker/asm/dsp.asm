namespace DSP
MVOL_L =    $0C
MVOL_R =    $1C
EVOL_L =    $2C
EVOL_R =    $3C
KON =       $4C
KOF =       $5C
FLG =       $6C
ENDX =	    $7C
EFB	=       $0D
PMON =	    $2D
NON =	    $3D
EON =	    $4D
DIR =	    $5D
ESA =	    $6D
EDL =	    $7D
C0 =	    $0F
C1 =	    $1F
C2 =	    $2F
C3 =	    $3F
C4 =	    $4F
C5 =	    $5F
C6 =	    $6F
C7 =	    $7F

macro write_dsp_voice_registers(n)
V<n>_VOL_L  = $<n>0
V<n>_VOL_R  = $<n>1
V<n>_PITCHL = $<n>2
V<n>_PITCHH = $<n>3
V<n>_SCRN   = $<n>4
V<n>_ADSR1  = $<n>5
V<n>_ADSR2  = $<n>6
V<n>_GAIN   = $<n>7
V<n>_ENVX   = $<n>8
V<n>_OUTX   = $<n>9
endmacro
%write_dsp_voice_registers(0)
%write_dsp_voice_registers(1)
%write_dsp_voice_registers(2)
%write_dsp_voice_registers(3)
%write_dsp_voice_registers(4)
%write_dsp_voice_registers(5)
%write_dsp_voice_registers(6)
%write_dsp_voice_registers(7)

namespace off