#include "SnesAPUHandler.h"

void SnesAPUHandler::APU_Startup()
{
	//Starting up the DSP and SPC that the emu will use
	spc_init_rom(Spc, IPL_ROM);
	spc_reset(Spc);
	spc_mute_voices(Spc, 0b00000000);

	spc_dsp_init(Dsp, DSP_MEMORY);
	spc_dsp_reset(Dsp);
}

void SnesAPUHandler::APU_Update(spc_sample_t* Output)
{
	if (reg == PAL)
	{
		ClockBase -= CLOCK_TICK_PAL;
	}
	else
	{
		ClockBase -= CLOCK_TICK_NTSC;
	}

	if (ClockBase <= 0)
	{
		ClockBase += MAX_CLOCK_DSP;
		spc_end_frame(Spc, ClockBase);
	}

	spc_dsp_run(Spc, ClockBase);

	//spc_dsp_read(Dsp, 0x7F);
	//spc_dsp_write(Dsp, 0x7F, 0xF);

	if (spc_sample_count(Spc) < 32)
	{
		spc_set_output(Spc, Output, sizeof(Output));
	}
	if (spc_dsp_sample_count(Dsp) < 32)
	{
		spc_dsp_set_output(Spc, Output, sizeof(Output));
	}
}

void SnesAPUHandler::APU_Run()
{
}

void SnesAPUHandler::APU_Kill()
{
	spc_delete(Spc);
}

void SnesAPUHandler::APU_COM()
{
	//Communicate between the CPU and SPC
}
