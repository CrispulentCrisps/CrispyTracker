#pragma once
#include <iostream>
#include <vector>
#include "Instrument.h"
#include "emu/dsp.h"
#include "emu/spc.h"

#define MAX_CLOCK_DSP       1024000

#define GLOBAL_mvol_l       0x0C
#define GLOBAL_mvol_r       0x1C
#define GLOBAL_evol_l       0x2C
#define GLOBAL_evol_r       0x3C
#define GLOBAL_kon          0x4C
#define GLOBAL_kof          0x5C
#define GLOBAL_flg          0x6C
#define GLOBAL_endx         0x7C
#define GLOBAL_efb          0x0D
#define GLOBAL_pmon         0x2D
#define GLOBAL_non          0x3D
#define GLOBAL_eon          0x4D
#define GLOBAL_dir          0x5D
#define GLOBAL_esa          0x6D
#define GLOBAL_edl          0x7D
#define GLOBAL_c0           0x0F
#define GLOBAL_c1           0x1F
#define GLOBAL_c2           0x2F
#define GLOBAL_c3           0x3F
#define GLOBAL_c4           0x4F
#define GLOBAL_c5           0x5F
#define GLOBAL_c6           0x6F
#define GLOBAL_c7           0x7F

#define SPC_test            0xF0
#define SPC_ctrl            0xF1
#define SPC_regaddr         0xF2
#define SPC_regdat          0xF3
#define SPC_p0              0xF4
#define SPC_p1              0xF5
#define SPC_p2              0xF6
#define SPC_p3              0xF7
#define SPC_t0              0xFA
#define SPC_t1              0xFB
#define SPC_t2              0xFC
#define SPC_c0              0xFD
#define SPC_c1              0xFE
#define SPC_c2              0xFF

//
//  Memory layout
//  0000 - 00EF |   Zeropage
//  00F0 - 00FF |   CPU registers 
//  0100 - 01FF |   Stackpage
//  0200 - FFBF |   RAM
//  FFC0 - FFFF |   IPL ROM
//

enum Region {
    PAL = 0,
    NTSC = 1,
};

class SnesAPUHandler
{
public:
    SNES_SPC* Spc = spc_new();
    spc_dsp_t* Dsp = spc_dsp_new();
    SPC_Filter* Filter = spc_filter_new();

    Instrument* ChannelInst[8];

    struct DSP_Ch_Reg {
        unsigned char vol_l;
        unsigned char vol_r;
        unsigned char pit_l;
        unsigned char pit_r;
        unsigned char scrn;
        unsigned char adsr_1;
        unsigned char adsr_2;
        unsigned char gain;
        unsigned char envx;
        unsigned char outx;
    };

    DSP_Ch_Reg ChannelRegs[8];

    unsigned int SONG_ADDR = 0x10000;
    const unsigned char IPL_ROM[64] = {
    0xcd, 0xef, 0xbd, 0xe8, 0x00, 0xc6, 0x1d, 0xd0, 0xfc, 0x8f, 0xaa, 0xf4,
    0x8f, 0xbb, 0xf5, 0x78, 0xcc, 0xf4, 0xd0, 0xfb, 0x2f, 0x19, 0xeb, 0xf4,
    0xd0, 0xfc, 0x7e, 0xf4, 0xd0, 0x0b, 0xe4, 0xf5, 0xcb, 0xf4, 0xd7, 0x00,
    0xfc, 0xd0, 0xf3, 0xab, 0x01, 0x10, 0xef, 0x7e, 0xf4, 0x10, 0xeb, 0xba,
    0xf6, 0xda, 0x00, 0xba, 0xf4, 0xc4, 0xf4, 0xdd, 0x5d, 0xd0, 0xdb, 0x1f,
    0x00, 0x00, 0xc0, 0xff
    };
    
    const unsigned char* DSP_MEMORY[65536];
    const unsigned char* Sample_Dir_Table[1024];
    spc_time_t ClockBase = MAX_CLOCK_DSP;
    Region reg;
    
    void APU_Startup();
    void APU_Update(spc_sample_t* Output, int BufferSize);
    void APU_Grab_Channel_Status(Instrument* inst, int currrentchannel);
    //void APU_Run(spc_sample_t* Output, int BufSize);
    void APU_Kill();
    void APU_COM();
    void APU_Set_Sample_Directory(std::vector<Sample>& samp);
    void APU_Evaluate_BRR_Loop(Sample* sample, int LoopPoint);
    void APU_Evaluate_BRR_End(Sample* sample, int EndPoint);
    
    bool APU_Set_Master_Vol(signed char vol);
    bool APU_Set_Echo_Vol_l(signed char vol);
};