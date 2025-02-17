#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <vector>
#include <array>
#include "Channel.h"
#include "FileDef.h"
#include "emu/dsp.h"
#include "emu/spc.h"
#include "emu/SNES_SPC.h"
// References
// 
// https://snes.nesdev.org/wiki/S-SMP#P
// https://problemkaputt.de/fullsnes.htm#snesapumemoryandiomap
//

#define MAX_CLOCK_DSP       1024000

#define Sample_Dir_Page     0x0C00
#define Sample_Mem_Page     0x0D00
#define Echo_Buffer_Addr    0xEE00

//
//  Memory layout
//  0000 - 00EF     |   Zeropage        \
//  00F0 - 00FF     |   CPU registers    |
//  0100 - 01FF     |   Stackpage        |      Reserved/Static
//  0200 - 0C00     |   Driver           |
//  0C00 - 0D00     |   DIR page        /
// 
//  0D00 - XXXX     |   Sample page    \
//  XXXX - YYYY     |   Instrument page |
//  YYYY - ZZZZ     |   Patterns page   |       Dynamic data
//  ZZZZ - WWWW     |   Orders page     |
//  WWWW - SSSS     |   Subtune page    |
//  SSSS - FFFF     |   Echo page      /
//

enum Region {
	PAL = 0,
	NTSC = 1,
};

class SnesAPUHandler
{

public:
	SNES_SPC* Spc = spc_new();
	SPC_Filter* Filter = spc_filter_new();

	struct DSP_Ch_Reg {
		unsigned char vol_l;
		unsigned char vol_r;
		unsigned char pit_l;
		unsigned char pit_h;
		unsigned char scrn;
		unsigned char adsr_1;
		unsigned char adsr_2;
		unsigned char gain;
		unsigned char envx;
		unsigned char outx;
	};

	DSP_Ch_Reg ChannelRegs[8];

	u16 InstAddr       = 0x0200;	//Instrument adress
	u16 SequenceAddr   = 0x0200;	//Sequence entry address
	u16 PatternAddr    = 0x0200;	//Pattern address
	u16 MusicOrderAddr = 0x0200;	//Music pattern addresses
	u16 SfxOrderAddr   = 0x0200;	//Sfx pattern addresses
	u16 MusicSubAddr   = 0x0200;	//Music subtune address
	u16 SFXSubAddr     = 0x0200;	//SFX subtune address

	std::vector<InstEntry>      InstMem;
	std::vector<SequenceEntry>  SeqMem;
	std::vector<PatternEntry>   PatMem;
	std::vector<SubtuneEntry>   SubMem;

	std::vector<Row> uniquerows;
	std::vector<u16> MusicOrders;
	std::vector<u16> SfxOrders;

	const unsigned char IPL_ROM[64] = {
	0xcd, 0xef, 0xbd, 0xe8, 0x00, 0xc6, 0x1d, 0xd0, 0xfc, 0x8f, 0xaa, 0xf4,
	0x8f, 0xbb, 0xf5, 0x78, 0xcc, 0xf4, 0xd0, 0xfb, 0x2f, 0x19, 0xeb, 0xf4,
	0xd0, 0xfc, 0x7e, 0xf4, 0xd0, 0x0b, 0xe4, 0xf5, 0xcb, 0xf4, 0xd7, 0x00,
	0xfc, 0xd0, 0xf3, 0xab, 0x01, 0x10, 0xef, 0x7e, 0xf4, 0x10, 0xeb, 0xba,
	0xf6, 0xda, 0x00, 0xba, 0xf4, 0xc4, 0xf4, 0xdd, 0x5d, 0xd0, 0xdb, 0x1f,
	0x00, 0x00, 0xc0, 0xff
	};
	
	unsigned char DSP_MEMORY[1024*64];

	spc_time_t timer;				//Running timer for SPC-700

	u16 LastSamplePoint;

	std::vector<u16> PitchTable;

	EffectsHandler EffectHandle;

	int8_t ChannelVolume_L[8];
	int8_t ChannelVolume_R[8];

	u8 KONState;
	u8 KOFState;

	u8 Handshake = 1;

	u16 SPCPtr = DATA_START;

	ComType ExCom[EXCOM_SIZE] = {com_Sleep, com_Stop, com_Break, com_Goto};  //Exclusive commands that must be at the end of a given sequence chunk

	void APU_Startup();
	void APU_Update(spc_sample_t* Output, int BufferSize);
	//void APU_Grab_Channel_Status(Channel* ch, Instrument* inst, int ypos);
	void APU_Play_Note_Editor(Channel* ch, Instrument* inst, int note, bool IsOn);

	//void APU_Process_Effects(Channel* ch, Instrument* inst, int ypos, int& speed, int& patindex, int currenttick);

	void APU_Kill();
	void SPCWrite(u8 byte);
	void WriteCommand(Command com);
	void APU_UpdateTuneMemory(vector<Instrument>& inst, vector<Sample>& sample, vector<Subtune>& sub, vector<Patterns>& pat, int subind);
	void APU_EvaluateSequenceData(vector<Patterns>& pat, vector<Instrument>& inst, int rowsize);               //Expects StoragePatterns [aka all unique patterns] as input
	void APU_Set_Sample_Memory(std::vector<Sample>& samp);
	void APU_Set_Sample_Directory(std::vector<Sample>& samp);
	void APU_Evaluate_BRR_Loop(Sample* sample, int LoopPoint);
	void APU_Evaluate_BRR_Loop_Start(Sample* sample);
	void APU_Write_Music_Orders(vector<Patterns>& pat, vector<Subtune>& sub);
	void APU_Write_Subtunes();
	void APU_Update_Instrument_Memory(std::vector<Patterns>& pat, std::vector<Instrument>& inst, int TrackSize);
	void APU_Update_Sequence_Memory(std::vector<Patterns>& pat, std::vector<Instrument>& inst, int TrackSize);
	void APU_Update_Pattern_Memory(std::vector<Patterns>& pat, std::vector<Instrument>& inst, int TrackSize);
	bool APU_Set_Master_Vol(signed char vol);
	void APU_Set_Echo(unsigned int dtime, int* coef, signed int dfb, signed int dvol);
	void APU_Init_Echo();

	void APU_Start_Tune(int subind);

	void APU_Audio_Stop();
	void APU_Audio_Start();
	void APU_SoftReset();

	int APU_Return_Cycle_Since_Last_Frame();

	void APU_Rebuild_Sample_Memory(std::vector<Sample>& samp);

	void APU_Debug_Dump_BRR();
	void APU_Debug_Dump_DIR();
	void APU_Debug_Dump_SPC();
	void APU_Debug_Dump_FLG();
	void APU_Debug_Dump_INST();
	int APU_Debug_KON_State();
	int APU_Debug_KOF_State();
	int APU_Debug_PIT_State(int index, int byte);
	int APU_Debug_VOL_State(int index, int byte);
};