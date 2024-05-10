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
		ChannelRegs[i].pit_h = (i * 16) + 3;
		ChannelRegs[i].scrn = (i * 16) + 4;
		ChannelRegs[i].adsr_1 = (i * 16) + 5;
		ChannelRegs[i].adsr_2 = (i * 16) + 6;
		ChannelRegs[i].gain = (i * 16) + 7;
		ChannelRegs[i].envx = (i * 16) + 8;
		ChannelRegs[i].outx = (i * 16) + 9;
	}

	for (int i = 0; i < 8; i++)
	{
		KON_arr[i] = false;
	}

	spc_dsp_write(Dsp, GLOBAL_dir, 0x0);

	spc_dsp_write(Dsp, GLOBAL_kon, 0x00);
	spc_dsp_write(Dsp, GLOBAL_kof, 0xFF);

	spc_dsp_write(Dsp, GLOBAL_mvol_l, 127);
	spc_dsp_write(Dsp, GLOBAL_mvol_r, 127);

	spc_dsp_write(Dsp, GLOBAL_flg, 0b10000000);
	spc_dsp_write(Dsp, GLOBAL_pmon, 0b00000000);
	spc_dsp_write(Dsp, GLOBAL_non, 0b00000000);
	spc_dsp_write(Dsp, GLOBAL_eon, 0b00000000);
}

void SnesAPUHandler::APU_Update(spc_sample_t* Output, int BufferSize)
{
	//spc_set_output(Spc, Output, BufferSize);

	int ClockCycleRound = (BufferSize / 44100.0) * MAX_CLOCK_DSP;
	int InterbufSize = (ClockCycleRound / 16);
	if (InterbufSize % 2 == 1) InterbufSize++;
	std::vector<spc_sample_t> InterBuf(InterbufSize);

	spc_dsp_set_output(Dsp, InterBuf.data(), InterBuf.size());

	spc_dsp_run(Spc, ClockBase);

	float StepCount = 0;

	for (int x = 0; x < BufferSize; x++)
	{
		Output[x] = InterBuf[(int)round(StepCount)];
		StepCount += InterBuf.size() / (float)BufferSize;
	}

	spc_filter_run(Filter, Output, BufferSize);
}

void SnesAPUHandler::APU_Grab_Channel_Status(Channel* ch, Instrument* inst, int ypos)
{
	int currentnote = ch->Rows[ypos].note* ch->Rows[ypos].octave;
	int currentinst = ch->Rows[ypos].instrument;
	int currentvol = ch->Rows[ypos].volume;
	int currenteffect = ch->Rows[ypos].effect;
	int currentvalue = ch->Rows[ypos].effectvalue;
	int id = ch->Index;

	unsigned char KONResult = 0;
	if (currentnote < 256)//Assuming it's not a reserved note
	{
		KON_arr[id] = true;

		//Do some fuckery with the pitch register
		spc_dsp_write(Dsp, ChannelRegs[id].pit_l, 0x1000);//Just a test at C-4
		spc_dsp_write(Dsp, ChannelRegs[id].pit_h, 0x0000);


		if (currentinst < 256)
		{
			spc_dsp_write(Dsp, ChannelRegs[id].vol_l, 127);
			spc_dsp_write(Dsp, ChannelRegs[id].vol_r, 127);
			spc_dsp_write(Dsp, ChannelRegs[id].scrn, inst[currentinst].SampleIndex);
		}
	}
	else if (currentnote == 257)//Assuming this is an OFF command
	{
		KON_arr[id] = false;
	}

	KONResult += KON_arr[id] << id;

	spc_dsp_write(Dsp, GLOBAL_kon, KONResult);
	spc_dsp_write(Dsp, GLOBAL_kof, ~KONResult);
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

void SnesAPUHandler::APU_Set_Sample_Memory(std::vector<Sample>& samp)
{
	int AddrOff = 0;
	for (int i = 0; i < samp.size(); i++)//Total samples
	{
		samp[i].brr.SampleDir = Sample_Mem_Page + AddrOff;
		for (int j = 0; j < samp[i].brr.DBlocks.size(); j++)//BRR Block Index
		{
			if (samp[i].LoopStart / 16 == j)
			{
				samp[i].LoopStartAddr = Sample_Mem_Page + AddrOff;
			}

			DSP_MEMORY[Sample_Mem_Page + AddrOff] = samp[i].brr.DBlocks[j].HeaderByte;
			AddrOff++;

			for (int k = 0; k < 8; k++)//BRR Data blocks
			{
				DSP_MEMORY[Sample_Mem_Page + AddrOff] = samp[i].brr.DBlocks[j].DataByte[k];
				AddrOff++;
			}
		}
	}
}

void SnesAPUHandler::APU_Set_Sample_Directory(std::vector<Sample>& samp)
{
	//sets the sample directory at 0xDD00
	int CurrentDir = 0;
	for (int i = 0; i < samp.size(); i++)
	{
		int DirSize = 4;
		samp[i].brr.SampleDir = CurrentDir + DirSize;

		DSP_MEMORY[Sample_Dir_Page + CurrentDir] = samp[i].brr.SampleDir & 0xFF;//Low byte of start
		DSP_MEMORY[Sample_Dir_Page + CurrentDir + 1] = samp[i].brr.SampleDir << 8 & 0xFF;//High byte of the start

		DSP_MEMORY[Sample_Dir_Page + CurrentDir + 2] = samp[i].LoopStartAddr & 0xFF;//High byte of the start
		DSP_MEMORY[Sample_Dir_Page + CurrentDir + 3] = samp[i].LoopStartAddr << 8 & 0xFF;//High byte of the start

		CurrentDir += DirSize;
	}
}

void SnesAPUHandler::APU_Evaluate_BRR_Loop(Sample* sample, int LoopPoint)
{
	//Sets the loop
	int BlockPos = LoopPoint >> 4;
	for (int x = BlockPos -1; x < BlockPos + 1; x++)
	{
		if (x <= sample->brr.DBlocks.size() && x > 0)
		{
			if (x == BlockPos)sample->brr.DBlocks[x].HeaderByte |= sample->LoopFlag;
			else sample->brr.DBlocks[x].HeaderByte &= sample->LoopFlag;
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
			if (x == BlockPos)sample->brr.DBlocks[x].HeaderByte |= sample->EndFlag;
			else sample->brr.DBlocks[x].HeaderByte &= sample->EndFlag;
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

void SnesAPUHandler::APU_Set_Echo(unsigned char dtime, int* coef)
{
	spc_dsp_write(Dsp, GLOBAL_edl, dtime);
	spc_dsp_write(Dsp, GLOBAL_c0, coef[0]);
	spc_dsp_write(Dsp, GLOBAL_c1, coef[1]);
	spc_dsp_write(Dsp, GLOBAL_c2, coef[2]);
	spc_dsp_write(Dsp, GLOBAL_c3, coef[3]);
	spc_dsp_write(Dsp, GLOBAL_c4, coef[4]);
	spc_dsp_write(Dsp, GLOBAL_c5, coef[5]);
	spc_dsp_write(Dsp, GLOBAL_c6, coef[6]);
	spc_dsp_write(Dsp, GLOBAL_c7, coef[7]);
}
