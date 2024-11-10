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

		//Set the channel volume to max
		ChannelVolume_L[i] = 127;
		ChannelVolume_R[i] = 127;
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

	//Calculate pitch table
	//Current issue is the tuning seems to be off C and closer to G (b/2)
	int addroff = 0;
	int index = 0;
	for (int x = 0; x < 9; x++)
	{
		std::cout << "\n Octave: " << x << " ";
		for (int y = 0; y < 12; y++)
		{
			float exactpitch = StartValues[y] * (1 << x);
			PitchTable.push_back((uint16_t)(exactpitch));
			float pit = pow(2.0, (index-48) / 12.0);
			std::cout << "\ndb " << (int)((pit * 16000 * 16.0) / 125.0);
			//Write pitch table to memory as reference
			//DSP_MEMORY[Pitch_Table_Page + addroff] = PitchTable[y + (12 * x)] & 0xFF;
			//DSP_MEMORY[Pitch_Table_Page + addroff + 1] = (PitchTable[y + (12 * x)] >> 8) & 0xFF;
			//addroff += 2;
			index++;
		}
		std::cout << "\n";
	}

	InstMem.push_back(InstEntry());
}
//Update loop for the DSP
void SnesAPUHandler::APU_Update(spc_sample_t* Output, int BufferSize)
{
	//spc_set_output(Spc, Output, BufferSize);

	//This is to match the 32KHz the SNES outputs compared to the 44.1KHz the tracker outputs
	int ClockCycleRound = (BufferSize / 44100.0) * MAX_CLOCK_DSP;
	int InterbufSize = (ClockCycleRound / 16);
	if (InterbufSize % 2 == 1) InterbufSize++;
	vector<array<spc_sample_t, 2>> InterBuf(InterbufSize * 2);

	spc_dsp_set_output(Dsp, (spc_sample_t*)InterBuf.data(), InterBuf.size());

	spc_dsp_run(Dsp, ClockCycleRound);
	//spc_end_frame(Spc, ClockCycleRound);

	float StepCount = 0;
	float MaxBufNeeded = spc_dsp_sample_count(Dsp);
	for (int x = 0; x < BufferSize; x += 2)
	{
		for (int y = 0; y < 2; y++)
		{
			Output[x + y] = InterBuf[(int)round(StepCount)][y];
		}
		StepCount += MaxBufNeeded / (float)BufferSize;
	}

	spc_filter_run(Filter, Output, BufferSize);
}

//Updating the registers of the DSP to reflect the changes in the track as it goes by
void SnesAPUHandler::APU_Grab_Channel_Status(Channel* ch, Instrument* inst, int ypos)
{
	int currentnote = ch->Rows[ypos].note;
	int currentoctave = ch->Rows[ypos].octave;
	int currentinst = ch->Rows[ypos].instrument;
	int currentvol = ch->Rows[ypos].volume;
	int currenteffect = ch->Rows[ypos].effect;
	int currentvalue = ch->Rows[ypos].effectvalue;
	int id = ch->Index;
	if (currentvol < NULL_COMMAND)
	{
		ChannelVolume_L[id] = currentvol * inst->SetVolume(1);
		ChannelVolume_R[id] = currentvol * inst->SetVolume(-1);
	}

	if (currentnote < STOP_COMMAND && currentnote != 0 && currentnote != NULL_COMMAND)//Assuming it's not a reserved note
	{
		int ADSR1 = 0;
		int ADSR2 = 0;

		ADSR1 += ((int)inst->EnvelopeUsed << 7);
		ADSR1 += (inst->Decay << 4);
		ADSR1 += (inst->Attack);

		ADSR2 += (inst->Sustain << 5);
		ADSR2 += (inst->Release);

		spc_dsp_write(Dsp, ChannelRegs[id].adsr_1, ADSR1);
		spc_dsp_write(Dsp, ChannelRegs[id].adsr_2, ADSR2);
		spc_dsp_write(Dsp, ChannelRegs[id].gain, inst->Gain);

		spc_dsp_write(Dsp, GLOBAL_pmon, inst->PitchMod << id);

		if (currentinst < 256 && currentinst != 0)
		{
			spc_dsp_write(Dsp, ChannelRegs[id].vol_l, ChannelVolume_L[id] * inst->SetVolume(1));
			spc_dsp_write(Dsp, ChannelRegs[id].vol_r, ChannelVolume_R[id] * inst->SetVolume(-1));
			spc_dsp_write(Dsp, ChannelRegs[id].scrn, inst->CurrentSample.SampleADDR);
			spc_dsp_write(Dsp, GLOBAL_eon, (int)inst->Echo << id);

			spc_dsp_write(Dsp, GLOBAL_non, inst->Noise << id);

			if (!inst->Noise)//Do some fuckery with the pitch register
			{
				uint16_t Pitch = inst->BRR_Pitch(pow(2.0, (currentnote - 48 + inst->NoteOff) / 12.0));

				spc_dsp_write(Dsp, ChannelRegs[id].pit_l, Pitch & 0xFF);
				spc_dsp_write(Dsp, ChannelRegs[id].pit_h, (Pitch >> 8) & 0xFF);
			}
			else //We don't need to calculate pitches if we are playing noise
			{
				spc_dsp_write(Dsp, GLOBAL_flg, currentnote % 32);
			}
		}

		if (ch->Rows[ypos].note != RELEASE_COMMAND && ch->Rows[ypos].note != STOP_COMMAND)//Assuming we've hit a key
		{
			KONState |= (1 << id);
			spc_dsp_write(Dsp, GLOBAL_kon, KONState);
			int kofcheck = spc_dsp_read(Dsp, GLOBAL_kof);
			kofcheck &= ~(1 << id);
			spc_dsp_write(Dsp, GLOBAL_kof, kofcheck);
		}
		else if (ch->Rows[ypos].note != NULL_COMMAND)//Assuming we haven't hit a key
		{
			if (ch->Rows[ypos].note == RELEASE_COMMAND)
			{
				spc_dsp_write(Dsp, GLOBAL_kof, 1 << id);
			}
			else if (spc_dsp_read(Dsp, GLOBAL_endx) << id && !inst->CurrentSample.Loop)//Assuming the sample has hit the end flag WITHOUT the loop flag
			{
				spc_dsp_write(Dsp, GLOBAL_kof, 1 << id);
			}
		}
	}
	else if (currentnote == STOP_COMMAND)//Assuming this is an OFF command
	{
		spc_dsp_write(Dsp, ChannelRegs[id].vol_l, 0);
		spc_dsp_write(Dsp, ChannelRegs[id].vol_r, 0);
	}
	else
	{
		KONState &= ~(1 << id);
	}
}

void SnesAPUHandler::APU_Play_Note_Editor(Channel* ch, Instrument* inst, int note, bool IsOn)
{
	int currentnote = note;
	int id = ch->Index;

	if (currentnote < STOP_COMMAND && currentnote != 0)//Assuming it's not a reserved note
	{
		int ADSR1 = 0;
		int ADSR2 = 0;

		ADSR1 += ((int)inst->EnvelopeUsed << 7);
		ADSR1 += (inst->Decay << 4);
		ADSR1 += (inst->Attack);

		ADSR2 += (inst->Sustain << 5);
		ADSR2 += (inst->Release);

		spc_dsp_write(Dsp, ChannelRegs[id].adsr_1, ADSR1);
		spc_dsp_write(Dsp, ChannelRegs[id].adsr_2, ADSR2);
		spc_dsp_write(Dsp, ChannelRegs[id].gain, inst->Gain);

		spc_dsp_write(Dsp, GLOBAL_pmon, inst->PitchMod << id);

		spc_dsp_write(Dsp, ChannelRegs[id].vol_l, ChannelVolume_L[id] * inst->SetVolume(1));
		spc_dsp_write(Dsp, ChannelRegs[id].vol_r, ChannelVolume_R[id] * inst->SetVolume(-1));
		spc_dsp_write(Dsp, ChannelRegs[id].scrn, inst->CurrentSample.SampleADDR);
		spc_dsp_write(Dsp, GLOBAL_eon, (int)inst->Echo << id);

		spc_dsp_write(Dsp, GLOBAL_non, inst->Noise << id);
		if (!inst->Noise)//Do some fuckery with the pitch register
		{
			uint16_t Pitch = inst->BRR_Pitch(pow(2.0, (currentnote - 48 + inst->NoteOff) / 12.0));

			spc_dsp_write(Dsp, ChannelRegs[id].pit_l, Pitch & 0xFF);
			spc_dsp_write(Dsp, ChannelRegs[id].pit_h, (Pitch >> 8) & 0xFF);
		}
		else //We don't need to calculate pitches if we are playing noise
		{
			spc_dsp_write(Dsp, GLOBAL_flg, currentnote%32);
		}

		if (IsOn)
		{
			spc_dsp_write(Dsp, GLOBAL_kon, 1 << id);
			int kofcheck = spc_dsp_read(Dsp, GLOBAL_kof);
			kofcheck &= ~(1 << id);
			spc_dsp_write(Dsp, GLOBAL_kof, kofcheck);
		}
		else
		{
			spc_dsp_write(Dsp, GLOBAL_kof, 1 << id);
			int kofcheck = spc_dsp_read(Dsp, GLOBAL_kon);
			kofcheck &= ~(1 << id);
			spc_dsp_write(Dsp, GLOBAL_kon, kofcheck);
		}
	}
}

//Handling effects
void SnesAPUHandler::APU_Process_Effects(Channel* ch, Instrument* inst, int ypos, int& speed, int& patindex, int currenttick)
{
	int note = ch->Rows[ypos].note;
	int effect = ch->Rows[ypos].effect;
	int value = ch->Rows[ypos].effectvalue;
	int id = ch->Index;

	if (note < NULL_COMMAND) {
		EffectHandle.Base_Note[id] = note;
		EffectHandle.Base_Pit[id] = inst->BRR_Pitch(pow(2.0, (EffectHandle.Base_Note[id] - 48 + inst->NoteOff) / 12.0));
	}

	if (value < NULL_COMMAND)
	{
		switch (effect)
		{
		case ARPEGGIO:
			value != 0 ? EffectHandle.Effect_Flags[id] |= arp_flag : EffectHandle.Effect_Flags[id] &= ~arp_flag;
			EffectHandle.Arp_Value[id] = value;
			break;
		case PORT_UP:
			value != 0 ? EffectHandle.Effect_Flags[id] |= port_flag : EffectHandle.Effect_Flags[id] &= ~port_flag;
			EffectHandle.Port_Value[id] = value;
			break;
		case PORT_DOWN:
			value != 0 ? EffectHandle.Effect_Flags[id] |= port_flag : EffectHandle.Effect_Flags[id] &= ~port_flag;
			EffectHandle.Port_Value[id] = -value;
			break;
		case PORT_TO:
			value != 0 ? EffectHandle.Effect_Flags[id] |= port_to_flag : EffectHandle.Effect_Flags[id] &= ~port_to_flag;
			if (EffectHandle.Effect_Flags[id] |= port_to_flag & 1) { EffectHandle.Effect_Flags[id] &= ~port_flag; }
			EffectHandle.Port_Value[id] = value;
			break;
		case VIBRATO:
			value != 0 ? EffectHandle.Effect_Flags[id] |= vibrato_flag : EffectHandle.Effect_Flags[id] &= ~vibrato_flag;
			EffectHandle.Vibrato_Value[id] = value;
			EffectHandle.Sine_Index_Vib[id] = 0;
			break;
		case TREMOLANDO:
			value != 0 ? EffectHandle.Effect_Flags[id] |= tremo_flag : EffectHandle.Effect_Flags[id] &= ~tremo_flag;
			EffectHandle.Tremo_Value[id] = value;
			EffectHandle.Base_Vol_L[id] = ChannelVolume_L[id] * inst->SetVolume(1);
			EffectHandle.Base_Vol_R[id] = ChannelVolume_R[id] * inst->SetVolume(-1);
			EffectHandle.Sine_Index_Trem[id] = 0;
			break;
		case PANNING:
			break;
		case SPEED:
			value != 0 ? speed = value : speed = speed;
			break;
		case VOLSLIDE:
			break;
		case GOTO:
			break;
		case RETRIGGER:
			if(currenttick % value == 0 && value != 0) APU_Grab_Channel_Status(ch, inst, ypos);
			break;
		case BREAK:
			break;
		case PANBRELLO:
			value != 0 ? EffectHandle.Effect_Flags[id] |= panbrello_flag : EffectHandle.Effect_Flags[id] &= ~panbrello_flag;
			EffectHandle.Panbrello_Value[id] = value;
			EffectHandle.Base_Vol_L[id] = ChannelVolume_L[id] * inst->SetVolume(1);
			EffectHandle.Base_Vol_R[id] = ChannelVolume_R[id] * inst->SetVolume(-1);
			EffectHandle.Sine_Index_PnBr[id] = 0;
			break;
		case ECHO_DEL:
			break;
		case ECHO_FDB:
			break;
		case ECHO_L:
			break;
		case ECHO_R:
			break;
		case ECHO_FIL_1:
			break;
		case ECHO_FIL_2:
			break;
		case ECHO_FIL_3:
			break;
		case ECHO_FIL_4:
			break;
		case ECHO_FIL_5:
			break;
		case ECHO_FIL_6:
			break;
		case ECHO_FIL_7:
			break;
		case ECHO_FIL_8:
			break;
		case FLAG_0:
			break;
		case FLAG_1:
			break;
		case FLAG_2:
			break;
		case FLAG_3:
			break;
		case FLAG_4:
			break;
		case FLAG_5:
			break;
		case FLAG_6:
			break;
		case FLAG_7:
			break;
		case FLAG_8:
			break;
		case FLAG_9:
			break;
		case FLAG_A:
			break;
		case FLAG_B:
			break;
		case FLAG_C:
			break;
		case FLAG_D:
			break;
		case FLAG_E:
			break;
		case FLAG_F:
			break;
		case ARP_SPEED:
			EffectHandle.Arp_Control = value;
			break;
		case PORT_UP_CTRL:
			break;
		case PORT_DOWN_CTRL:
			break;
		case GLOABL_PAN_L:
			break;
		case GLOABL_PAN_R:
			break;
		case GLOBAL_VOL:
			break;
		case END:
			break;
		}
	}

	//Arpeggio
	if (EffectHandle.Effect_Flags[id] & 1)
	{
		uint16_t Pitch = 0;
		uint8_t Note1 = (EffectHandle.Arp_Value[id]) & 0x0F;
		uint8_t Note2 = (EffectHandle.Arp_Value[id] >> 4) & 0x0F;	
		EffectHandle.ArpCounter[id]++;
		if (EffectHandle.ArpCounter[id] >= EffectHandle.Arp_Control)
		{
			if (EffectHandle.ArpState[id] == 0)
			{
				Pitch = inst->BRR_Pitch(pow(2.0, (EffectHandle.Base_Note[id] - 48 + inst->NoteOff + Note2) / 12.0));
			}
			else if (EffectHandle.ArpState[id] == 1)
			{
				Pitch = inst->BRR_Pitch(pow(2.0, (EffectHandle.Base_Note[id] - 48 + inst->NoteOff + Note1) / 12.0));
			}
			else
			{
				Pitch = inst->BRR_Pitch(pow(2.0, (EffectHandle.Base_Note[id] - 48 + inst->NoteOff) / 12.0));
			}
			EffectHandle.ArpState[id] += 2;
			EffectHandle.ArpState[id] = EffectHandle.ArpState[id]%3;
			EffectHandle.ArpCounter[id] = 0;
		}

		if (Pitch != 0)
		{
			spc_dsp_write(Dsp, ChannelRegs[id].pit_l, Pitch & 0xFF);
			spc_dsp_write(Dsp, ChannelRegs[id].pit_h, (Pitch >> 8) & 0xFF);
		}
	}

	//Portamento
	if ((EffectHandle.Effect_Flags[id] >> 1) & 1)//Check if the effect flag is on
	{
		uint16_t pit = spc_dsp_read(Dsp, ChannelRegs[id].pit_l) + (spc_dsp_read(Dsp, ChannelRegs[id].pit_h) << 8);
		if (pit + value > 0 && pit + value < 0x3FFF)
		{
			pit += EffectHandle.Port_Value[id];
		}
		else
		{
			EffectHandle.Effect_Flags[id] &= ~port_flag;//We can just set the flag off to not impede with the CPU
		}
		spc_dsp_write(Dsp, ChannelRegs[id].pit_l, pit & 0xFF);
		spc_dsp_write(Dsp, ChannelRegs[id].pit_h, (pit >> 8) & 0xFF);
	}

	//Vibrato
	if ((EffectHandle.Effect_Flags[id] >> 2) & 1)//Check if the effect flag is on
	{
		uint8_t depth = EffectHandle.Vibrato_Value[id] & 0x0F;
		uint8_t speed = (EffectHandle.Vibrato_Value[id] >> 4) & 0x0F;
		uint16_t pit = EffectHandle.Base_Pit[id];
		pit += ((SineTable[EffectHandle.Sine_Index_Vib[id]]) / 16) * depth;
		EffectHandle.Sine_Index_Vib[id] += speed;
		spc_dsp_write(Dsp, ChannelRegs[id].pit_l, pit & 0xFF);
		spc_dsp_write(Dsp, ChannelRegs[id].pit_h, (pit >> 8) & 0xFF);
	}
	
	//Tremolando
	if ((EffectHandle.Effect_Flags[id] >> 3) & 1)//Check if the effect flag is on
	{
		uint8_t depth = EffectHandle.Tremo_Value[id] & 0x0F;
		uint8_t speed = (EffectHandle.Tremo_Value[id] >> 4) & 0x0F;
		int8_t vol_l = EffectHandle.Base_Vol_L[id];
		int8_t vol_r = EffectHandle.Base_Vol_R[id];
		int8_t dif_l = (SineTable[EffectHandle.Sine_Index_Trem[id]] / 16) * ((double)depth * inst->SetVolume(1));
		int8_t dif_r = (SineTable[EffectHandle.Sine_Index_Trem[id]] / 16) * ((double)depth * inst->SetVolume(-1));

		if (vol_l + dif_l >= -128 && vol_l + dif_l <= 127) vol_l += dif_l;
		if (vol_r + dif_r >= -128 && vol_r + dif_r <= 127) vol_r += dif_r;
		
		spc_dsp_write(Dsp, ChannelRegs[id].vol_l, vol_l);
		spc_dsp_write(Dsp, ChannelRegs[id].vol_r, vol_r);
		
		EffectHandle.Sine_Index_Trem[id] += speed;
	}

	//Panbrello	
	if ((EffectHandle.Effect_Flags[id] >> 4) & 1)//Check if the effect flag is on
	{
		uint8_t depth = EffectHandle.Panbrello_Value[id] & 0x0F;
		uint8_t speed = (EffectHandle.Panbrello_Value[id] >> 4) & 0x0F;
		int8_t vol_l = EffectHandle.Base_Vol_L[id];
		int8_t vol_r = EffectHandle.Base_Vol_R[id];
		int8_t dif_l = (SineTable[EffectHandle.Sine_Index_PnBr[id]] / 16) * ((double)depth * inst->SetVolume(1));
		int8_t dif_r = (SineTable[EffectHandle.Sine_Index_PnBr[id]+127] / 16) * ((double)depth * inst->SetVolume(-1));
		cout << "\ndir_l" << (int)dif_l << "\ndir_r" << (int)dif_r;
		if (vol_l + dif_l >= -128 && vol_l + dif_l <= 127) vol_l += dif_l;
		if (vol_r + dif_r >= -128 && vol_r + dif_r <= 127) vol_r += dif_r;

		spc_dsp_write(Dsp, ChannelRegs[id].vol_l, vol_l);
		spc_dsp_write(Dsp, ChannelRegs[id].vol_r, vol_r);

		EffectHandle.Sine_Index_PnBr[id] += speed;
	}
	
	//VolSlide
	if ((EffectHandle.Effect_Flags[id] >> 5) & 1)//Check if the effect flag is on
	{

	}
	
	//Portamento to
	if ((EffectHandle.Effect_Flags[id] >> 6) & 1)//Check if the effect flag is on
	{
		uint16_t val = EffectHandle.Port_Value[id];
		uint16_t pit = spc_dsp_read(Dsp, ChannelRegs[id].pit_l) + (spc_dsp_read(Dsp, ChannelRegs[id].pit_h) << 8);
		uint16_t target = inst->BRR_Pitch(pow(2.0, (EffectHandle.Base_Note[id] - 48 + inst->NoteOff) / 12.0));
		if (target < pit)
		{
			pit += val;
			if (pit + val > target)
			{
				pit = target;
				EffectHandle.Effect_Flags[id] &= ~port_to_flag;
			}
		}
		else
		{
			pit -= val;
			if (pit - val < target)
			{
				pit = target;
				EffectHandle.Effect_Flags[id] &= ~port_to_flag;
			}
		}
		spc_dsp_write(Dsp, ChannelRegs[id].pit_l, pit & 0xFF);
		spc_dsp_write(Dsp, ChannelRegs[id].pit_h, (pit >> 8) & 0xFF);
	}
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
	for (int i = 1; i < samp.size(); i++)//Total samples
	{
		samp[i].SampleIndex = i;
		samp[i].brr.SampleDir = Sample_Mem_Page + AddrOff;
		for (int j = 0; j < samp[i].brr.DBlocks.size(); j++)//BRR Block Index
		{
			if (AddrOff < Sample_Dir_Page)
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
			else
			{
				std::cout << "\nERROR: SAMPLE TOO LARGE\nADDR-OFF: " << AddrOff << "\nBRR BLOCK: " << j;
				break;
			}
		}
	}
	LastSamplePoint = Sample_Mem_Page + AddrOff;
	for (int x = LastSamplePoint; x < 0xFFFF; x++)
	{
		DSP_MEMORY[x] = 0;
	}
}

//Sets up the DIR page for interfacing with samples
void SnesAPUHandler::APU_Set_Sample_Directory(std::vector<Sample>& samp)
{
	int CurrentDir = 0;
	for (int i = 1; i < samp.size(); i++)
	{
		int DirSize = 4;

		DSP_MEMORY[Sample_Dir_Page + CurrentDir] = samp[i].brr.SampleDir & 0xFF;//Low byte of directory
		DSP_MEMORY[Sample_Dir_Page + CurrentDir + 1] = (samp[i].brr.SampleDir >> 8) & 0xFF;//High byte of the directory

		DSP_MEMORY[Sample_Dir_Page + CurrentDir + 2] = samp[i].LoopStartAddr & 0xFF;//High byte of the start
		DSP_MEMORY[Sample_Dir_Page + CurrentDir + 3] = (samp[i].LoopStartAddr >> 8) & 0xFF;//High byte of the start

		samp[i].SampleADDR = i-1;

		CurrentDir += DirSize;
	}
}

//Formatting the BRR END and LOOP flags
void SnesAPUHandler::APU_Evaluate_BRR_Loop(Sample* sample, int LoopPoint)
{
	int LoopBlockPos = LoopPoint / 16;
	for (int x = 0; x < sample->brr.DBlocks.size(); x++)
	{
		if (x == LoopBlockPos - 1 && sample->Loop)//Assuming we've hit the loop point
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

//Writes page for instruments
void SnesAPUHandler::APU_Update_Instrument_Memory(std::vector<Patterns>& pat, std::vector<Instrument>& inst, int TrackSize)
{
	int addroff = 0;
	//Write instrument table
	InstMem.clear();
	InstMem.push_back(InstEntry());//This one isn't counted, reason it's here is to mirror the instrument list having the first entry as a "Default" one
	if (LastSamplePoint != 0) InstAddr = LastSamplePoint;//Assuming we have no samples in memory
	else InstAddr = Sample_Mem_Page;
	char buf[10];
	sprintf_s(buf, "%04X", InstAddr);
	std::cout << "\nInstADDR: " << buf;
	for (int x = 1; x < inst.size(); x++)
	{
		InstEntry i_ent = InstEntry();
		
		uint8_t ADSR1 = 0;
		uint8_t ADSR2 = 0;
		ADSR1 += ((int)inst[x].EnvelopeUsed << 7);
		ADSR1 += (inst[x].Decay << 4);
		ADSR1 += (inst[x].Attack);
		ADSR2 += (inst[x].Sustain << 5);
		ADSR2 += (inst[x].Release);
		
		i_ent.ADSR1 = ADSR1;
		i_ent.ADSR2 = ADSR2;
		i_ent.Gain = inst[x].Gain;
		i_ent.Vol_L = 127 * inst[x].SetVolume(1);
		i_ent.Vol_R = 127 * inst[x].SetVolume(-1);
		i_ent.SampleIndex = inst[x].CurrentSample.SampleIndex;

		i_ent.EffectState |= ((int)inst[x].InvL << 0);
		i_ent.EffectState |= ((int)inst[x].InvR << 1);
		i_ent.EffectState |= ((int)inst[x].PitchMod << 2);
		i_ent.EffectState |= ((int)inst[x].Noise << 3);
		i_ent.EffectState |= ((int)inst[x].Echo << 4);

		InstMem.push_back(i_ent);

		DSP_MEMORY[InstAddr + addroff] = InstMem[x].Vol_L;
		DSP_MEMORY[InstAddr + addroff + 1] = InstMem[x].Vol_R;
		DSP_MEMORY[InstAddr + addroff + 2] = InstMem[x].ADSR1;
		DSP_MEMORY[InstAddr + addroff + 3] = InstMem[x].ADSR2;
		DSP_MEMORY[InstAddr + addroff + 4] = InstMem[x].Gain;
		DSP_MEMORY[InstAddr + addroff + 5] = InstMem[x].EffectState;
		DSP_MEMORY[InstAddr + addroff + 6] = InstMem[x].SampleIndex;
		addroff += 7;
	}
	SequenceAddr = InstAddr + addroff;
	sprintf_s(buf, "%04X", SequenceAddr);
	std::cout << "\nSequenceADDR: " << buf;
	//APU_Update_Sequence_Memory(pat, inst, TrackSize);
}

//Writes page for sequence entries
void SnesAPUHandler::APU_Update_Sequence_Memory(std::vector<Patterns>& pat, std::vector<Instrument>& inst, int TrackSize)
{
	SeqMem.clear();
	//Find every unique row entry
	uniquerows.clear();
	Row BlankRow = Row();
	BlankRow.note = NULL_COMMAND;
	BlankRow.volume = NULL_COMMAND;
	BlankRow.octave = 0;
	BlankRow.effect = NULL_COMMAND;
	BlankRow.effectvalue = NULL_COMMAND;
	BlankRow.instrument = NULL_COMMAND;
	uniquerows.push_back(BlankRow);

	//Find all unique row entries
	for (int x = 0; x < pat.size(); x++)
	{
		int waitcount = 0;
		for (int y = 0; y < TrackSize; y++)
		{
			bool isunique = true;
			for (int z = 0; z < uniquerows.size(); z++)//Checks if we can find a unique tro
			{
				Row currentrow = pat[x].SavedRows[y];
				if (currentrow.note == uniquerows[z].note && currentrow.octave == uniquerows[z].octave && currentrow.volume == uniquerows[z].volume && currentrow.effect == uniquerows[z].effect && currentrow.effectvalue == uniquerows[z].effectvalue)
				{
					isunique = false;
					break;
				}
			}

			if (isunique)
			{
				uniquerows.push_back(pat[x].SavedRows[y]);
			}
		}
	}

	int addroff = 0;
	//Write rows to memory
	for (int x = 0; x < uniquerows.size(); x++)
	{
		SequenceEntry entry = SequenceEntry();
		if (uniquerows[x].instrument != NULL_COMMAND && uniquerows[x].instrument <= inst.size())
		{
			Instrument currentinst = inst[uniquerows[x].instrument];

			entry.Pitch = currentinst.BRR_Pitch(pow(2.0, (uniquerows[x].note - 48 + currentinst.NoteOff) / 12.0));
			entry.Volume_L = uniquerows[x].volume;
			entry.Volume_R = uniquerows[x].volume;
			entry.instADDR = InstAddr + ((uniquerows[x].instrument-1) * 7);
			entry.EffectsState = uniquerows[x].effect;
			entry.EffectsValue = uniquerows[x].effectvalue;
			
			std::cout << "\nIndex: " << x << "\nentry pitch " << entry.Pitch << "\nentry vol l " << (int)entry.Volume_L << "\nentry vol r " << (int)entry.Volume_R << "\nentry inst addr " << (int)entry.instADDR << "\nentry effect state " << (int)entry.EffectsState << "\nentry effect value " << (int)entry.EffectsValue;

			SeqMem.push_back(entry);
			
			DSP_MEMORY[SequenceAddr + addroff] = (entry.Pitch) & 0xFF;
			DSP_MEMORY[SequenceAddr + addroff + 1] = (entry.Pitch >> 8) & 0xFF;
			DSP_MEMORY[SequenceAddr + addroff + 2] = entry.Volume_L;
			DSP_MEMORY[SequenceAddr + addroff + 3] = entry.Volume_R;
			DSP_MEMORY[SequenceAddr + addroff + 4] = (entry.instADDR) & 0xFF;
			DSP_MEMORY[SequenceAddr + addroff + 5] = (entry.instADDR >> 8) & 0xFF;
			DSP_MEMORY[SequenceAddr + addroff + 6] = entry.EffectsState;
			DSP_MEMORY[SequenceAddr + addroff + 7] = entry.EffectsValue;
			addroff += 8;
		}
	}
	PatternAddr = SequenceAddr + addroff;
	char buf[10];
	sprintf_s(buf, "%04X", PatternAddr);
	std::cout << "\nPatternADDR: " << buf;
	APU_Update_Pattern_Memory(pat, inst, TrackSize);
}

//Writes page for patterns
void SnesAPUHandler::APU_Update_Pattern_Memory(std::vector<Patterns>& pat, std::vector<Instrument>& inst, int TrackSize)
{
	int addroff = 0;
	for (int x = 0; x < pat.size(); x++)
	{
		PatternEntry entry = PatternEntry();
		entry.PatternIndex = x;
		int SeqAmount = 0;
		for (int y = 0; y < TrackSize; y++)
		{
			for (int z = 0; z < SeqMem.size(); z++)//Check over any sequences to find a match
			{
				Row currentrow = pat[x].SavedRows[y];
				if (currentrow.note == uniquerows[z].note && currentrow.octave == uniquerows[z].octave && currentrow.volume == uniquerows[z].volume && currentrow.effect == uniquerows[z].effect && currentrow.effectvalue == uniquerows[z].effectvalue)
				{
					entry.SequenceList.push_back(SequenceAddr + (z*8));
					break;
				}
			}
		}
		entry.SequenceAmount = entry.SequenceList.size();

		DSP_MEMORY[PatternAddr + addroff] = entry.PatternIndex;
		DSP_MEMORY[PatternAddr + addroff + 1] = entry.SequenceAmount;
		addroff += 2;
		for (int w = 0; w < entry.SequenceAmount; w++)
		{
			DSP_MEMORY[PatternAddr + addroff] = (entry.SequenceList[w]) & 0xFF;
			DSP_MEMORY[PatternAddr + addroff + 1] = (entry.SequenceList[w] >> 8) & 0xFF;
			addroff += 2;
		}
	}
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
		std::cout << "\nEMU ERROR: VOL NOT WITHIN -128 & 127: VOL --> " << vol;
		return false;
	}
}

//Update the echo registers
void SnesAPUHandler::APU_Set_Echo(unsigned int dtime, int* coef, signed int dfb, signed int dvol)
{
	//int EchoAddr = (0xFFFF - (dtime * 0x0800)) >> 8;
	
	int EchoAddr = (0xFF00 - (dtime * 0x0800)) >> 8;
	if (dtime == 0)
	{
		EchoAddr = 0x1;
	}

	char buf[10];
	sprintf_s(buf, "%04X", EchoAddr);
	std::cout << "\nEcho Addr: " << buf;
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
	spc_dsp_write(Dsp, GLOBAL_esa, (0xFF00 - 0x0800) >> 8);//Sets the location of the echo buffer
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

void SnesAPUHandler::APU_Audio_Stop()
{
	spc_dsp_write(Dsp, GLOBAL_kof, 0xFF);
	spc_dsp_write(Dsp, GLOBAL_kon, 0x00);
}

void SnesAPUHandler::APU_Audio_Start()
{
	KONState = 0;
	KOFState = 0;
	spc_dsp_write(Dsp, GLOBAL_kof, 0x00);
	spc_dsp_write(Dsp, GLOBAL_kon, 0x00);
}

void SnesAPUHandler::APU_SoftReset()
{
	spc_dsp_soft_reset(Dsp);
	spc_dsp_write(Dsp, GLOBAL_flg, 0);
}

int SnesAPUHandler::APU_Return_Cycle_Since_Last_Frame()
{
	return spc_dsp_sample_count(Dsp) * 32;//32 samples per cycle
}

void SnesAPUHandler::APU_Rebuild_Sample_Memory(std::vector<Sample>& samp)
{
	APU_Set_Sample_Memory(samp);
	APU_Set_Sample_Directory(samp);
}

void SnesAPUHandler::APU_Debug_Dump_BRR()
{
	string filename = "BRR_Dump.bin";
	ofstream BRRFile(filename, ios::binary);
	for (int x = Sample_Mem_Page; x < LastSamplePoint; x++)
	{
		BRRFile << DSP_MEMORY[x];
	}
	BRRFile.close();
}

void SnesAPUHandler::APU_Debug_Dump_DIR()
{
	string filename = "DIR_Dump.bin";
	ofstream BRRFile(filename, ios::binary);
	for (int x = Sample_Dir_Page; x < 0xFFFF; x++)
	{
		BRRFile << DSP_MEMORY[x];
	}
	BRRFile.close();
}

void SnesAPUHandler::APU_Debug_Dump_SPC()
{
	string filename = "SPC_Dump.bin";
	ofstream BRRFile(filename, ios::binary);
	for (int x = 0; x < 65536; x++)
	{
		BRRFile << DSP_MEMORY[x];
	}
	BRRFile.close();
}

void SnesAPUHandler::APU_Debug_Dump_FLG()
{
	/*
	string filename = "FLG_Dump.bin";
	ofstream BRRFile(filename, ios::binary);
	for (int x = Flag_Effect_Page; x < Sample_Mem_Page; x++)
	{
		BRRFile << DSP_MEMORY[x];
	}
	BRRFile.close();
	*/
}

void SnesAPUHandler::APU_Debug_Dump_INST()
{
	string filename = "INST_Dump.bin";
	ofstream BRRFile(filename, ios::binary);
	for (int x = InstAddr; x < SequenceAddr; x++)
	{
		BRRFile << DSP_MEMORY[x];
	}
	BRRFile.close();
}

int SnesAPUHandler::APU_Debug_KON_State()
{
	return spc_dsp_read(Dsp, GLOBAL_kon);
}

int SnesAPUHandler::APU_Debug_KOF_State()
{
	return spc_dsp_read(Dsp, GLOBAL_kof);
}

int SnesAPUHandler::APU_Debug_PIT_State(int index, int byte)
{
	if (!byte) return spc_dsp_read(Dsp, ChannelRegs[index].pit_l);
	else  return spc_dsp_read(Dsp, ChannelRegs[index].pit_h);
}

int SnesAPUHandler::APU_Debug_VOL_State(int index, int byte)
{
	if (!byte) return spc_dsp_read(Dsp, ChannelRegs[index].vol_l);
	else  return spc_dsp_read(Dsp, ChannelRegs[index].vol_r);
}
