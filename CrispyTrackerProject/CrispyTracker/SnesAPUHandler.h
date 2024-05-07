#pragma once
#include <iostream>
#include "emu/dsp.h"
#include "emu/spc.h"

#define MAX_CLOCK_DSP       1024000
#define CLOCK_TICK_PAL      20480
#define CLOCK_TICK_NTSC     17067

#define DSP_TEST            0xF0
#define DSP_CONTROL         0xF1
#define DSP_ADDR_REG        0xF2
#define DSP_ADDR_DAT        0xF3
#define DSP_PORT_0          0xF4
#define DSP_PORT_1          0xF5
#define DSP_PORT_2          0xF6
#define DSP_PORT_3          0xF7
#define DSP_TIMER_0         0xFA
#define DSP_TIMER_1         0xFB
#define DSP_TIMER_2         0xFC
#define DSP_COUNT_0         0xFD
#define DSP_COUNT_1         0xFE
#define DSP_COUNT_2         0xFF

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
    unsigned int SONG_ADDR = 0x10000;
    
    const unsigned char IPL_ROM[64] = {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    };//How useful
    
    const unsigned char* DSP_MEMORY[65536];
    spc_time_t ClockBase = MAX_CLOCK_DSP;
    Region reg;
    
    void APU_Startup();
    void APU_Update(spc_sample_t* Output, int BufferSize);
    void APU_Run(spc_sample_t* Output, int BufSize);
    void APU_Kill();
    void APU_COM();
};