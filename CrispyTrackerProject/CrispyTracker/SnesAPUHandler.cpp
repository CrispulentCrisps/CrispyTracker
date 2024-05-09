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

	for (int i = 0; i < 8; i++)
	{
		ChannelRegs[i].vol_l = (i * 16);
		ChannelRegs[i].vol_r = (i * 16) + 1;
		ChannelRegs[i].pit_l = (i * 16) + 2;
		ChannelRegs[i].pit_r = (i * 16) + 3;
		ChannelRegs[i].scrn = (i * 16) + 4;
		ChannelRegs[i].adsr_1 = (i * 16) + 5;
		ChannelRegs[i].adsr_2 = (i * 16) + 6;
		ChannelRegs[i].gain = (i * 16) + 7;
		ChannelRegs[i].envx = (i * 16) + 8;
		ChannelRegs[i].outx = (i * 16) + 9;
	}

	spc_dsp_write(Dsp, GLOBAL_mvol_l, 127);
	spc_dsp_write(Dsp, GLOBAL_mvol_r, 127);
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

void SnesAPUHandler::APU_Grab_Channel_Status(Instrument* inst, int currrentchannel)
{

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

void SnesAPUHandler::APU_Set_Sample_Directory(std::vector<Sample>& samp)
{
	int CurrentDir = 0;
	for (int i = 0; i < samp.size(); i++)
	{
		int DirSize = samp[i].brr.DBlocks.size() / 8;
		samp[i].brr.SampleDir = CurrentDir + DirSize;
		CurrentDir += DirSize;
	}
}

void SnesAPUHandler::APU_Evaluate_BRR_Loop(Sample* sample, int LoopPoint)
{
	int BlockPos = LoopPoint >> 4;
	for (int x = BlockPos -1; x < BlockPos + 1; x++)
	{
		if (x <= sample->brr.DBlocks.size() && x > 0)
		{
			sample->brr.DBlocks[x].HeaderByte |= sample->LoopFlag;
		}
	}
}

void SnesAPUHandler::APU_Evaluate_BRR_End(Sample* sample, int EndPoint)
{
	int BlockPos = EndPoint >> 4;
	for (int x = BlockPos - 1; x < BlockPos + 1; x++)
	{
		if (x <= sample->brr.DBlocks.size() && x > 0)
		{
			sample->brr.DBlocks[x].HeaderByte |= sample->EndFlag;
		}
	}
}

bool SnesAPUHandler::APU_Set_Master_Vol(signed char vol)
{
	if (vol >= -128 && vol <= 127)
	{
		spc_dsp_write(Dsp, GLOBAL_mvol_l, vol);
		spc_dsp_write(Dsp, GLOBAL_mvol_r, vol);
		return true;
	}
	else
	{
		cout << "\nEMU ERROR: VOL NOT WITHIN -128 & 127: VOL --> " << vol;
		return false;
	}
}
