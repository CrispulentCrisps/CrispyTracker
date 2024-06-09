namespace HW
INIDISP     = $2100
OBSEL       = $2101
OAMADDL     = $2102
OAMADDH     = $2103
OAMDATA     = $2104
BGMODE      = $2105
MOSAIC      = $2106
BG1SC       = $2107
BG2SC       = $2108
BG3SC       = $2109
BG4SC       = $210A
BG12NBA     = $210B
BG34NBA     = $210C
BG1HOFS     = $210D
BG1VOFS     = $210E
BG2HOFS     = $210F
BG2VOFS     = $2110
BG3HOFS     = $2111
BG3VOFS     = $2112
BG4HOFS     = $2113
BG4VOFS     = $2114
VMAIN       = $2115
VMADDL      = $2116
VMADDH      = $2117
VMDATAL     = $2118
VMDATAH     = $2119
M7SEL       = $211A
M7A         = $211B
M7B         = $211C
M7C         = $211D
M7D         = $211E
M7X         = $211F
M7Y         = $2120
CGADD       = $2121
CGDATA      = $2122
W12SEL      = $2123
W34SEL      = $2124
WOBJSEL     = $2125
WH0         = $2126
WH1         = $2127
WH2         = $2128
WH3         = $2129
WBGLOG      = $212A
WOBJLOG     = $212B
TM          = $212C
TS          = $212D
TMW         = $212E
TSW         = $212F
CGWSEL      = $2130
CGADSUB     = $2131
COLDATA     = $2132
SETINI      = $2133
MPYL        = $2134
MPYM        = $2135
MPYH        = $2136
SLHV        = $2137
RDOAM       = $2138
RDVRAML     = $2139
RDVRAMH     = $213A
RDCGRAM     = $213B
OPHCT       = $213C
OPVCT       = $213D
STAT77      = $213E
STAT78      = $213F

APUI00      = $2140
APUI01      = $2141
APUI02      = $2142
APUI03      = $2143

WMDATA      = $2180
WMADDL      = $2181
WMADDM      = $2182
WMADDH      = $2183

JOYWR       = $4016
JOYA        = $4016
JOYB        = $4017

NMITIMEN    = $4200
WRIO        = $4201
WRMPYA      = $4202
WRMPYB      = $4203
WRDIVL      = $4204
WRDIVH      = $4205
WRDIVB      = $4206
HTIMEL      = $4207
HTIMEH      = $4208
VTIMEL      = $4209
VTIMEH      = $420A
MDMAEN      = $420B
HDMAEN      = $420C
MEMSEL      = $420D

RDNMI       = $4210
TIMEUP      = $4211
HVBJOY      = $4212
RDIO        = $4213
RDDIVL      = $4214
RDDIVH      = $4215
RDMPYL      = $4216
RDMPYH      = $4217
JOY1L       = $4218
JOY1H       = $4219
JOY2L       = $421A
JOY2H       = $421B
JOY3L       = $421C
JOY3H       = $421D
JOY4L       = $421E
JOY4H       = $421F

macro write_dma_registers(n)
DMAP<n> = $43<n>0
BBAD<n> = $43<n>1
A1T<n>L = $43<n>2
A1T<n>H = $43<n>3
A1B<n>  = $43<n>4
DAS<n>L = $43<n>5
DAS<n>H = $43<n>6
DASB<n> = $43<n>7
A2A<n>L = $43<n>8
A2A<n>H = $43<n>9
NTRL<n> = $43<n>A
UNKN<n> = $43<n>B
endmacro
%write_dma_registers(0)
%write_dma_registers(1)
%write_dma_registers(2)
%write_dma_registers(3)
%write_dma_registers(4)
%write_dma_registers(5)
%write_dma_registers(6)
%write_dma_registers(7)
namespace off