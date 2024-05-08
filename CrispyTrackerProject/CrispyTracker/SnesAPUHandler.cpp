#include "SnesAPUHandler.h"

void SnesAPUHandler::APU_Startup()
{
	//Starting up the DSP and SPC that the emu will use
	spc_init_rom(Spc, IPL_ROM);
	spc_reset(Spc);
	spc_mute_voices(Spc, 0b00000000);
	spc_clear_echo(Spc);
	spc_filter_clear(Filter);

	spc_dsp_init(Dsp, DSP_MEMORY);
	spc_dsp_reset(Dsp);
}

void SnesAPUHandler::APU_Update(spc_sample_t* Output, int BufferSize)
{
	if (reg == PAL)
	{
		ClockBase -= CLOCK_TICK_PAL;

		if (ClockBase < CLOCK_TICK_PAL)
		{
			ClockBase += MAX_CLOCK_DSP;
			spc_dsp_run(Spc, ClockBase);
			spc_end_frame(Spc, ClockBase);
		}
	}
	else
	{
		ClockBase -= CLOCK_TICK_NTSC;

		if (ClockBase < CLOCK_TICK_NTSC)
		{
			ClockBase += MAX_CLOCK_DSP;
			spc_dsp_run(Spc, ClockBase);
			spc_end_frame(Spc, ClockBase);
		}
	}

	//spc_dsp_read(Dsp, 0x7F);
	//spc_dsp_write(Dsp, 0x7F, 0xF);

	spc_dsp_set_output(Spc, Output, BufferSize);
	spc_set_output(Spc, Output, BufferSize);
	
	//spc_dsp_set_output(Spc, Output, BufferSize);
	//spc_set_output(Spc, Output, BufferSize);
}

void SnesAPUHandler::APU_Run(spc_sample_t* Output, int BufSize)
{
	spc_play(Spc, BufSize, Output);
	spc_filter_run(Filter, Output, BufSize);
}

void SnesAPUHandler::APU_Kill()
{
	spc_delete(Spc);
}

void SnesAPUHandler::APU_COM()
{
	//Communicate between the CPU and SPC
}
