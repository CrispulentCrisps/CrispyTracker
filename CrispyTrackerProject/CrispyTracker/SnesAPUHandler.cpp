#include "SnesAPUHandler.h"

void SnesAPUHandler::APU_Startup()
{
	//Starting up the DSP and SPC that the emu will use
	spc_init_rom(Spc, IPL_ROM);
	spc_dsp_new();
	spc_reset(Spc);
	spc_mute_voices(Spc, 0b00000000);
}

void SnesAPUHandler::APU_Update(spc_sample_t Output)
{
	spc_set_output(Spc, &Output,sizeof(Output));
}

void SnesAPUHandler::APU_Kill()
{
	spc_delete(Spc);
}

void SnesAPUHandler::APU_COM()
{
	//Communicate between the CPU and SPC
}
