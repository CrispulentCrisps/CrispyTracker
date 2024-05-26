#include "SnesAPUHandler.h"

//Boots up the emulation core
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

	//Setting up the per channel registers for the SPC
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

	//Reset KON
	for (int i = 0; i < 8; i++)
	{
		KON_arr[i] = false;
	}
	//Setup the registers
	spc_dsp_write(Dsp, GLOBAL_dir, Sample_Dir_Page >> 8);

	spc_dsp_write(Dsp, GLOBAL_kon, 0x00);
	spc_dsp_write(Dsp, GLOBAL_kof, 0x00);

	//Reset volume
	spc_dsp_write(Dsp, GLOBAL_mvol_l, 0x7F);
	spc_dsp_write(Dsp, GLOBAL_mvol_r, 0x7F);

	spc_dsp_write(Dsp, GLOBAL_esa, 0x00);
	spc_dsp_write(Dsp, GLOBAL_edl, 0x00);
	spc_dsp_write(Dsp, GLOBAL_flg, 0b00000000);

	spc_dsp_write(Dsp, GLOBAL_pmon, 0b00000000);
	spc_dsp_write(Dsp, GLOBAL_non, 0b00000000);
	spc_dsp_write(Dsp, GLOBAL_eon, 0b00000000);
	spc_dsp_write(Dsp, GLOBAL_endx, 0x00);
	spc_dsp_write(Dsp, GLOBAL_efb, 0x00);

}
//Update loop for the DSP
void SnesAPUHandler::APU_Update(spc_sample_t* Output, int BufferSize)
{
	//spc_set_output(Spc, Output, BufferSize);

	//This is to match the 32KHz the SNES outputs compared to the 44.1KHz the tracker outputs
	int ClockCycleRound = (BufferSize / 44100.0) * MAX_CLOCK_DSP;
	int InterbufSize = (ClockCycleRound / 16);
	if (InterbufSize % 2 == 1) InterbufSize++;
	std::vector<spc_sample_t> InterBuf(InterbufSize * 2);

	//spc_dsp_write(Dsp, GLOBAL_kon, 0xFF);

	spc_dsp_set_output(Dsp, InterBuf.data(), InterBuf.size());

	spc_dsp_run(Dsp, ClockCycleRound);

	//cout << "\nSample Count: " << spc_dsp_sample_count(Dsp);

	float StepCount = 0;
	float MaxBufNeeded = spc_dsp_sample_count(Dsp);
	for (int x = 0; x < BufferSize; x++)
	{
		Output[x] = InterBuf[(int)round(StepCount)];
		StepCount += MaxBufNeeded / (float)BufferSize;
	}

	spc_filter_run(Filter, Output, BufferSize);
}

//Updating the registers of the DSP to reflect the changes in the track as it goes by
void SnesAPUHandler::APU_Grab_Channel_Status(Channel* ch, Instrument* inst, int ypos)
{
	int currentnote = ch->Rows[ypos].note * ch->Rows[ypos].octave;
	int currentinst = ch->Rows[ypos].instrument;
	int currentvol = ch->Rows[ypos].volume;
	int currenteffect = ch->Rows[ypos].effect;
	int currentvalue = ch->Rows[ypos].effectvalue;
	int id = ch->Index;

	if (currentnote < 256 && currentnote != 0)//Assuming it's not a reserved note
	{
		unsigned char KONResult = 0;
		KON_arr[id] = true;

		int ADSR1 = 0;
		int ADSR2 = 0;

		if (inst->EnvelopeUsed)
		{
			ADSR1 += (1<<7);
			ADSR1 += (inst->Decay << 4);
			ADSR1 += (inst->Attack);

			ADSR2 += (inst->Sustain << 5);
			ADSR2 += (inst->Release);
		}

		spc_dsp_write(Dsp, ChannelRegs[id].adsr_1, ADSR1);
		spc_dsp_write(Dsp, ChannelRegs[id].adsr_2, ADSR2);
		spc_dsp_write(Dsp, ChannelRegs[id].gain,0x7F);

		if (currentinst < 256)
		{
			//spc_dsp_write(Dsp, ChannelRegs[id].vol_l, (inst->LPan) * (inst->Volume / 127.0));
			//spc_dsp_write(Dsp, ChannelRegs[id].vol_r, (inst->RPan) * (inst->Volume / 127.0));
			spc_dsp_write(Dsp, ChannelRegs[id].vol_l, inst->Volume);
			spc_dsp_write(Dsp, ChannelRegs[id].vol_r, inst->Volume);
			spc_dsp_write(Dsp, ChannelRegs[id].scrn, inst->CurrentSample.SampleADDR);//Issues arising from incorrect data pointing in sample memory
			spc_dsp_write(Dsp, GLOBAL_eon, (int)inst->Echo << id);

			//Do some fuckery with the pitch register
			if (!inst->Noise)//We don't need to calculate pitches if we are playing noise
			{			
				int Pitch = inst->BRR_Pitch(StartValues[(currentnote % 12) << ch->Rows[ypos].octave]);
				spc_dsp_write(Dsp, ChannelRegs[id].pit_l, Pitch);
				spc_dsp_write(Dsp, ChannelRegs[id].pit_h, Pitch >> 8);
			}
			else
			{
				spc_dsp_write(Dsp, GLOBAL_non, inst->Noise << id);
				spc_dsp_write(Dsp, GLOBAL_flg, inst->NoiseFreq);
			}

		}

		KONResult = KON_arr[id] << id;
		if (KONResult != 0)//Assuming we've hit a key
		{
			spc_dsp_write(Dsp, GLOBAL_kon, KONResult);
		}
		else//Assuming we haven't hit a key
		{
			if (ch->Rows[ypos].note == RELEASE_COMMAND)
			{
				spc_dsp_write(Dsp, GLOBAL_kof, (1 << id));
			}
			else if (spc_dsp_read(Dsp, GLOBAL_endx) << id && !inst->CurrentSample.Loop)//Assuming the sample has hit the end flag WITHOUT the loop flag
			{
				spc_dsp_write(Dsp, GLOBAL_kof, (1 << id));
			}
		}
	}
	else if (currentnote == 257)//Assuming this is an OFF command
	{
		KON_arr[id] = false;
	}
}

//Intended to do general writes to the CPU independent of the current channel
void SnesAPUHandler::APU_Evaluate_Channel_Regs(Channel* ch)
{
	//spc_dsp_write(Dsp, GLOBAL_eon, (int)ECHO_arr);
}

//Delete SPC & DSP instance
void SnesAPUHandler::APU_Kill()
{
	spc_delete(Spc);
	spc_dsp_delete(Dsp);
}

//Writes all sample data into memory from [Sample_Mem_Page]
void SnesAPUHandler::APU_Set_Sample_Memory(std::vector<Sample>& samp)
{
	int AddrOff = 0;
	for (int i = 0; i < samp.size(); i++)//Total samples
	{
		samp[i].brr.SampleDir = Sample_Mem_Page + AddrOff;
		for (int j = 0; j < samp[i].brr.DBlocks.size(); j++)//BRR Block Index
		{
			if (AddrOff < IPL_ROM_Page)
			{
				if (samp[i].LoopStart / 16 == j)
				{
					samp[i].LoopStartAddr = Sample_Mem_Page + AddrOff;
				}

				if (j == samp[i].brr.DBlocks.size() - 1)
				{
					cout << "\n\nHallo :D\n";
				}

				DSP_MEMORY[Sample_Mem_Page + AddrOff] = samp[i].brr.DBlocks[j].HeaderByte;
				AddrOff++;

				for (int k = 0; k < 8; k++)//BRR Data blocks
				{
					DSP_MEMORY[Sample_Mem_Page + AddrOff] = samp[i].brr.DBlocks[j].DataByte[k];
					AddrOff++;
				}
			}
			else
			{
				cout << "\nERROR: ATTEMPTING TO OVERWRITE IPL ROM\nADDR-OFF: " << AddrOff << "\nBRR BLOCK: " << j;
			}
		}
	}
	LastSamplePoint = AddrOff;
}

//Sets up the DIR page for interfacing with samples
void SnesAPUHandler::APU_Set_Sample_Directory(std::vector<Sample>& samp)
{
	//sets the sample directory at 0xDD00
	int CurrentDir = 0;
	for (int i = 0; i < samp.size(); i++)
	{
		int DirSize = 4;

		DSP_MEMORY[Sample_Dir_Page + CurrentDir] = samp[i].brr.SampleDir & 0xFF;//Low byte of directory
		DSP_MEMORY[Sample_Dir_Page + CurrentDir + 1] = (samp[i].brr.SampleDir >> 8) & 0xFF;//High byte of the directory

		DSP_MEMORY[Sample_Dir_Page + CurrentDir + 2] = samp[i].LoopStartAddr & 0xFF;//High byte of the start
		DSP_MEMORY[Sample_Dir_Page + CurrentDir + 3] = (samp[i].LoopStartAddr >> 8) & 0xFF;//High byte of the start

		samp[i].SampleADDR = i;

		CurrentDir += DirSize;
	}
}
//Formatting the BRR END and LOOP flags
void SnesAPUHandler::APU_Evaluate_BRR_Loop(Sample* sample, int LoopPoint)
{
	int LoopBlockPos = LoopPoint / 16;
	for (int x = 0; x < sample->brr.DBlocks.size(); x++)
	{
		if (x == LoopBlockPos && LoopBlockPos != 0)//Assuming we've hit the loop point
		{
			sample->brr.DBlocks[x].HeaderByte |= sample->LoopFlag | sample->EndFlag;
		}
		else if (x == sample->brr.DBlocks.size()-1)//Assuming we've hit the end of the BRR blocks
		{
			sample->brr.DBlocks[x].HeaderByte |= sample->EndFlag;
		}
		else //Assume it's a standard block that needs no flags attached
		{
			sample->brr.DBlocks[x].HeaderByte &= ~(sample->LoopFlag | sample->EndFlag);
		}
	}

	APU_Evaluate_BRR_Loop_Start(sample);
}

//Writes the loop start point to the DIR page
void SnesAPUHandler::APU_Evaluate_BRR_Loop_Start(Sample* sample)
{
	sample->LoopStartAddr = (sample->brr.SampleDir + (sample->LoopStart/16) * 9);
	DSP_MEMORY[Sample_Dir_Page + (sample->SampleIndex * 4) + 2] = sample->LoopStartAddr & 0xFF;
	DSP_MEMORY[Sample_Dir_Page + (sample->SampleIndex * 4) + 3] = (sample->LoopStartAddr >> 8) & 0xFF;
}

//Sets master volume of the track
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

//Update the echo registers
void SnesAPUHandler::APU_Set_Echo(unsigned int dtime, int* coef, signed int dfb, signed int dvol)
{
	//int EchoAddr = (0xFFFF - (dtime * 0x0800)) >> 8;
	int EchoAddr = (0xFF00 - (dtime * 0x0800)) >> 8;
	char buf[10];
	sprintf_s(buf, "%04X", EchoAddr);
	cout << "\nEcho Addr: " << buf;
	spc_dsp_write(Dsp, GLOBAL_evol_l, dvol);
	spc_dsp_write(Dsp, GLOBAL_evol_r, dvol);
	spc_dsp_write(Dsp, GLOBAL_edl, dtime);
	spc_dsp_write(Dsp, GLOBAL_efb, dfb);
	spc_dsp_write(Dsp, GLOBAL_esa, EchoAddr);//Sets the location of the echo buffer
	spc_dsp_write(Dsp, GLOBAL_c0, coef[0]);
	spc_dsp_write(Dsp, GLOBAL_c1, coef[1]);
	spc_dsp_write(Dsp, GLOBAL_c2, coef[2]);
	spc_dsp_write(Dsp, GLOBAL_c3, coef[3]);
	spc_dsp_write(Dsp, GLOBAL_c4, coef[4]);
	spc_dsp_write(Dsp, GLOBAL_c5, coef[5]);
	spc_dsp_write(Dsp, GLOBAL_c6, coef[6]);
	spc_dsp_write(Dsp, GLOBAL_c7, coef[7]);
	spc_dsp_soft_reset(Dsp);
	spc_dsp_write(Dsp, GLOBAL_flg, 0);//Flag used to update the EDL and ESA regs
}

//Initialises the echo values
void SnesAPUHandler::APU_Init_Echo()
{
	spc_dsp_write(Dsp, GLOBAL_evol_l, 96);
	spc_dsp_write(Dsp, GLOBAL_evol_r, 96);
	spc_dsp_write(Dsp, GLOBAL_edl, 0);
	spc_dsp_write(Dsp, GLOBAL_efb, 64);
	spc_dsp_write(Dsp, GLOBAL_esa, (0xFFFF - 0x0800) >> 8);//Sets the location of the echo buffer
	spc_dsp_write(Dsp, GLOBAL_c0, 127);
	spc_dsp_write(Dsp, GLOBAL_c1, 0);
	spc_dsp_write(Dsp, GLOBAL_c2, 0);
	spc_dsp_write(Dsp, GLOBAL_c3, 0);
	spc_dsp_write(Dsp, GLOBAL_c4, 0);
	spc_dsp_write(Dsp, GLOBAL_c5, 0);
	spc_dsp_write(Dsp, GLOBAL_c6, 0);
	spc_dsp_write(Dsp, GLOBAL_c7, 0);
	spc_dsp_soft_reset(Dsp);
	spc_dsp_write(Dsp, GLOBAL_flg, 0);//Flag used to update the EDL and ESA regs
}

void SnesAPUHandler::APU_AudioStop()
{
	for (int i = 0; i < 8; i++)
	{
		spc_dsp_write(Dsp, GLOBAL_kof, 1 << i);
	}
}
