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
	int ClockCycleRound = (BufferSize / 44100.0) * MAX_CLOCK_DSP;
	std::vector<spc_sample_t> InterBuf(ClockCycleRound/16);
	


	if (ClockBase <= 0)
	{
		ClockBase += MAX_CLOCK_DSP;
		spc_dsp_run(Spc, ClockBase);
		spc_end_frame(Spc, ClockBase);
	}
	spc_dsp_set_output(Dsp, Output, BufferSize);
	spc_set_output(Spc, Output, BufferSize);

	spc_filter_run(Filter, Output, BufferSize);
}

/*
void SnesAPUHandler::APU_Run(spc_sample_t* Output, int BufSize)
{
	//spc_play(Spc, BufSize, Output);
	spc_filter_run(Filter, Output, BufSize);
}
*/
void SnesAPUHandler::APU_Kill()
{
	spc_delete(Spc);
}

void SnesAPUHandler::APU_COM()
{
	//Communicate between the CPU and SPC
}
