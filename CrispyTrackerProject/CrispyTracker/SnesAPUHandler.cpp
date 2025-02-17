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

	//Write IPL rom and base
	FILE* driver = fopen(DRIVER_PATH, "rb");
	fseek(driver, DRIVER_ROM_ADDR, SEEK_SET);
	for (int x = 0; x < DRIVER_END; x++)
	{
		Spc->m.ram.ram[x + 0x0200] = fgetc(driver);
	}

	fclose(driver);

	Spc->m.cpu_regs.pc = DRIVER_CODE;

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
	InstMem.push_back(InstEntry());
}
//Update loop for the DSP
void SnesAPUHandler::APU_Update(spc_sample_t* Output, int BufferSize)
{
	//This is to match the 32KHz the SNES outputs compared to the 44.1KHz the tracker outputs
	int ClockCycleRound = (BufferSize / 44100.0) * MAX_CLOCK_DSP;
	int InterbufSize = (ClockCycleRound / 16);
	if (InterbufSize % 2 == 1) InterbufSize++;
	vector<array<spc_sample_t, 2>> InterBuf(InterbufSize * 2);

	spc_set_output(Spc, (spc_sample_t*)InterBuf.data(), InterBuf.size());

	spc_end_frame(Spc, ClockCycleRound);
	//spc_dsp_run(Dsp, ClockCycleRound);
	//spc_end_frame(Spc, ClockCycleRound);

	float StepCount = 0;
	float MaxBufNeeded = spc_sample_count(Spc);
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

void SnesAPUHandler::APU_Play_Note_Editor(Channel* ch, Instrument* inst, int note, bool IsOn)
{
}

//Delete SPC & DSP instance
void SnesAPUHandler::APU_Kill()
{
	spc_delete(Spc);
}

void SnesAPUHandler::SPCWrite(u8 byte)
{
	Spc->m.ram.ram[SPCPtr++] = byte;
	//cout << "\nDSP ADDR: " << SPCPtr << " | Written data: " << (int)byte;
}

void SnesAPUHandler::WriteCommand(Command com)
{
	SPCWrite((com.type) & 0xFF);
	if (com.type < com_PlayNote && com.type != com_ReleaseNote)
	{
		SPCWrite((com.val) & 0xFF);
		if (com.type == com_PlayPitch || com.type == com_ChannelVol)
		{
			SPCWrite((com.val >> 8) & 0xFF);
		}
	}
}

void SnesAPUHandler::APU_UpdateTuneMemory(vector<Instrument>& inst, vector<Sample>& sample, vector<Subtune>& sub, vector<Patterns>& pat, int subind)
{
	SPCPtr = DATA_START;
	MusicOrders.clear();
	SfxOrders.clear();
	APU_Rebuild_Sample_Memory(sample);
	APU_Update_Instrument_Memory(pat, inst, sub[subind].TrackLength);
	APU_EvaluateSequenceData(pat, inst, sub[subind].TrackLength);
	APU_Write_Music_Orders(pat, sub);
	APU_Write_Subtunes();


	//DSP_MEMORY[DRIVER_INSTPTR] =		(DRIVER_INSTPTR) & 0xFF;
	//DSP_MEMORY[DRIVER_INSTPTR + 1] =	(DRIVER_INSTPTR >> 8) & 0xFF;
	//DSP_MEMORY[DRIVER_ORDERPTR] =		(DRIVER_ORDERPTR) & 0xFF;
	//DSP_MEMORY[DRIVER_ORDERPTR + 1] =	(DRIVER_ORDERPTR >> 8) & 0xFF;
	//DSP_MEMORY[DRIVER_SFXPATPTR] =		(DRIVER_SFXPATPTR) & 0xFF;
	//DSP_MEMORY[DRIVER_SFXPATPTR + 1] =	(DRIVER_SFXPATPTR >> 8) & 0xFF;
	//DSP_MEMORY[DRIVER_SUBPTR] =			(DRIVER_SUBPTR) & 0xFF;
	//DSP_MEMORY[DRIVER_SUBPTR + 1] =		(DRIVER_SUBPTR >> 8) & 0xFF;
	//DSP_MEMORY[DRIVER_PITCHPTR] =		(DRIVER_PITCHPTR) & 0xFF;
	//DSP_MEMORY[DRIVER_PITCHPTR + 1] =	(DRIVER_PITCHPTR >> 8) & 0xFF;
}
//
// 
//	NOTES:
// 
//		Commands that must be placed at the end of the given sequence chunk:
//			Sleep
//			Stop
//			Break
//			Goto
//
void SnesAPUHandler::APU_EvaluateSequenceData(vector<Patterns>& pat, vector<Instrument>& inst, int rowsize)
{
	for (int x = 0; x < pat.size(); x++)
	{
		cout << "\nPAT: " << x << " AT: " << std::hex << SPCPtr;
		pat[x].Addr = SPCPtr;

		PatternState PState = PatternState();
		PState.IsEmpty = true;
		PState.LastEmpty = PState.IsEmpty;
		PState.lastvolume = 0x7F;

		for (int y = 0; y < rowsize; y++)
		{
			RowState currow = RowState();
			PState.IsEmpty = true;

			//Write Effects
			if (pat[x].SavedRows[y].effect != NULL_COMMAND)
			{
				//Check for exlcusive commands
				bool isex = false;
				for (int x = 0; x < EXCOM_SIZE; x++)
				{
					if (pat[x].SavedRows[y].effect == ExCom[x])
					{
						isex = true;
						break;
					}
				}
			}

			//Write Instrument
			if (PState.lastinst != pat[x].SavedRows[y].instrument && pat[x].SavedRows[y].instrument != NULL_COMMAND)
			{
				PState.IsEmpty = false;
				WriteCommand(Command{ com_SetInstrument, pat[x].SavedRows[y].instrument });
				PState.lastinst = pat[x].SavedRows[y].instrument;
			}

			//Write volume
			if (PState.lastvolume != pat[x].SavedRows[y].volume && pat[x].SavedRows[y].volume != NULL_COMMAND)
			{
				PState.IsEmpty = false;
				int16_t Lvol;
				int8_t LPan;
				int16_t Rvol;
				int8_t RPan;
				LPan = (inst[PState.lastinst].Volume * inst[PState.lastinst].LPan) / (u8)127;
				RPan = (inst[PState.lastinst].Volume * inst[PState.lastinst].RPan) / (u8)127;
				Lvol = (pat[x].SavedRows[y].volume * LPan) / 127;
				Rvol = (pat[x].SavedRows[y].volume * RPan) / 127;
				uint16_t outvol = (Lvol << 8) | Rvol;
				WriteCommand(Command{ com_ChannelVol, outvol });
				PState.lastvolume = pat[x].SavedRows[y].volume;
			}

			//Write pitches 
			if (pat[x].SavedRows[y].note != NULL_COMMAND)
			{
				PState.IsEmpty = false;
				uint16_t noteval = inst[PState.lastinst].BRR_Pitch(pow(2.0, (pat[x].SavedRows[y].note - 48 + inst[PState.lastinst].NoteOff) / 12.0));
				WriteCommand(Command{ com_PlayPitch, noteval });
			}

			if (PState.LastEmpty != PState.IsEmpty && PState.SleepCount != 0)
			{
				WriteCommand(Command{ com_Sleep, (unsigned short)(PState.SleepCount) });
				PState.SleepCount = 0;
			}
			PState.LastEmpty = PState.IsEmpty;

			if (PState.IsEmpty)
			{
				PState.SleepCount++;
			}
		}

		if (PState.IsEmpty)
		{
			WriteCommand(Command{ com_Sleep, (unsigned short)(PState.SleepCount) });
		}
	}
}

//Writes all sample data into memory from [Sample_Mem_Page]
void SnesAPUHandler::APU_Set_Sample_Memory(std::vector<Sample>& samp)
{
	for (int i = 1; i < samp.size(); i++)//Total samples
	{
		samp[i].SampleIndex = i;
		samp[i].brr.SampleDir = SPCPtr;
		for (int j = 0; j < samp[i].brr.DBlocks.size(); j++)//BRR Block Index
		{
			if (SPCPtr < Echo_Buffer_Addr)
			{
				if (samp[i].LoopStart / 16 == j)
				{
					samp[i].LoopStartAddr = SPCPtr;
				}

				SPCWrite(samp[i].brr.DBlocks[j].HeaderByte);

				for (int k = 0; k < 8; k++)//BRR Data blocks
				{
					SPCWrite(samp[i].brr.DBlocks[j].DataByte[k]);
				}
			}
			else
			{
				std::cout << "\nERROR: SAMPLE TOO LARGE\nADDR-OFF: " << SPCPtr << "\nBRR BLOCK: " << j;
				break;
			}
		}
	}
	LastSamplePoint = SPCPtr;
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
}

void SnesAPUHandler::APU_Write_Music_Orders(vector<Patterns>& pat, vector<Subtune>& sub)
{
	MusicOrderAddr = SPCPtr;
	for (int x = 0; x < sub.size(); x++)
	{
		if (!sub[x].SFXFlag)
		{
			MusicOrders.push_back(SPCPtr);
			for (int y = 0; y < sub[x].Orders[0].size(); y++)
			{
				for (int z = 0; z < 8; z++)
				{
					int currentpat = sub[x].Orders[z][y];
					u16 pataddr = pat[currentpat].Addr;
					SPCWrite((pataddr) & 0xFF);
					SPCWrite((pataddr>>8) & 0xFF);
				}
			}
		}
	}

	SfxOrderAddr = SPCPtr;
	for (int x = 0; x < sub.size(); x++)
	{
		if (sub[x].SFXFlag)
		{
			SfxOrders.push_back(SPCPtr);
			for (int y = 0; y < sub[x].Orders[0].size(); y++)
			{
				for (int z = 0; z < 8; z++)
				{
					int currentpat = sub[x].Orders[z][y];
					u16 pataddr = pat[currentpat].Addr;
					SPCWrite((pataddr) & 0xFF);
					SPCWrite((pataddr >> 8) & 0xFF);
				}
			}
		}
	}
}

void SnesAPUHandler::APU_Write_Subtunes()
{
	for (int x = 0; x < MusicOrders.size(); x++)
	{
		cout << "\nMusic Order: " << x << " | " << std::hex << MusicOrders[x];
		SPCWrite((MusicOrders[x]) & 0xFF);
		SPCWrite((MusicOrders[x] >> 8) & 0xFF);
	}

	for (int x = 0; x < SfxOrders.size(); x++)
	{
		cout << "\nSfx Order: " << x << " | " << std::hex << SfxOrders[x];
		SPCWrite((SfxOrders[x]) & 0xFF);
		SPCWrite((SfxOrders[x] >> 8) & 0xFF);
	}
}

//Sets up the DIR page for interfacing with samples
void SnesAPUHandler::APU_Set_Sample_Directory(std::vector<Sample>& samp)
{
	int CurrentDir = 0;
	int DirSize = 4;
	for (int i = 1; i < samp.size(); i++)
	{
		Spc->m.ram.ram[Sample_Dir_Page + CurrentDir] = samp[i].brr.SampleDir & 0xFF;			//Lo byte of directory
		Spc->m.ram.ram[Sample_Dir_Page + CurrentDir + 1] = (samp[i].brr.SampleDir >> 8) & 0xFF;	//Hi byte of directory
		Spc->m.ram.ram[Sample_Dir_Page + CurrentDir + 2] = samp[i].LoopStartAddr & 0xFF;		//Lo byte of the start
		Spc->m.ram.ram[Sample_Dir_Page + CurrentDir + 3] = (samp[i].LoopStartAddr >> 8) & 0xFF;	//Hi byte of the start
		samp[i].SampleADDR = i - 1;
		CurrentDir += DirSize;
	}
}

//Writes page for instruments
void SnesAPUHandler::APU_Update_Instrument_Memory(std::vector<Patterns>& pat, std::vector<Instrument>& inst, int TrackSize)
{
	InstAddr = SPCPtr;
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
		ADSR1 |= ((int)inst[x].EnvelopeUsed << 7);
		ADSR1 |= (inst[x].Decay << 4);
		ADSR1 |= (inst[x].Attack);
		ADSR2 |= (inst[x].Sustain << 5);
		ADSR2 |= (inst[x].Release);
		
		i_ent.ADSR1 = ADSR1;
		i_ent.ADSR2 = ADSR2;
		i_ent.Gain = inst[x].Gain;
		i_ent.SampleIndex = inst[x].CurrentSample.SampleIndex;

		i_ent.EffectState |= ((int)inst[x].PitchMod << 0) | ((int)inst[x].Noise << 1) | ((int)inst[x].Echo << 2);

		InstMem.push_back(i_ent);

		SPCWrite(InstMem[x].SampleIndex);
		SPCWrite(InstMem[x].ADSR1);
		SPCWrite(InstMem[x].ADSR2);
		SPCWrite(InstMem[x].Gain);
		SPCWrite(InstMem[x].EffectState);
	}
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
			
			Spc->m.ram.ram[SequenceAddr + addroff] = (entry.Pitch) & 0xFF;
			Spc->m.ram.ram[SequenceAddr + addroff + 1] = (entry.Pitch >> 8) & 0xFF;
			Spc->m.ram.ram[SequenceAddr + addroff + 2] = entry.Volume_L;
			Spc->m.ram.ram[SequenceAddr + addroff + 3] = entry.Volume_R;
			Spc->m.ram.ram[SequenceAddr + addroff + 4] = (entry.instADDR) & 0xFF;
			Spc->m.ram.ram[SequenceAddr + addroff + 5] = (entry.instADDR >> 8) & 0xFF;
			Spc->m.ram.ram[SequenceAddr + addroff + 6] = entry.EffectsState;
			Spc->m.ram.ram[SequenceAddr + addroff + 7] = entry.EffectsValue;
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

		Spc->m.ram.ram[PatternAddr + addroff] = entry.PatternIndex;
		Spc->m.ram.ram[PatternAddr + addroff + 1] = entry.SequenceAmount;
		addroff += 2;
		for (int w = 0; w < entry.SequenceAmount; w++)
		{
			Spc->m.ram.ram[PatternAddr + addroff] = (entry.SequenceList[w]) & 0xFF;
			Spc->m.ram.ram[PatternAddr + addroff + 1] = (entry.SequenceList[w] >> 8) & 0xFF;
			addroff += 2;
		}
	}
}

//Sets master volume of the track
bool SnesAPUHandler::APU_Set_Master_Vol(signed char vol)
{
	if (vol >= -128 && vol <= 127)
	{
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
}

//Initialises the echo values
void SnesAPUHandler::APU_Init_Echo()
{
}

void SnesAPUHandler::APU_Start_Tune(int subind)
{
	spc_write_port(Spc, Spc->m.spc_time, 0x00, 0);
	spc_write_port(Spc, Spc->m.spc_time, 0x02, subind);
	spc_write_port(Spc, Spc->m.spc_time, 0x01, Handshake);
	Handshake++;
	spc_time_t timer = Spc->m.spc_time;
	while (Handshake != spc_read_port(Spc, timer, 0x01))
	{
		timer = Spc->m.spc_time + 32;
		//spc_end_frame(Spc, timer);
		cout << std::hex << "\nPortVal: " << spc_read_port(Spc, timer, 0x01);
		cout << std::hex << "\nPC: " << Spc->m.cpu_regs.pc;
		cout << std::hex << "\nTime: " << Spc->m.spc_time;
	}
}

void SnesAPUHandler::APU_Audio_Stop()
{
}

void SnesAPUHandler::APU_Audio_Start()
{
	KONState = 0;
	KOFState = 0;
}

void SnesAPUHandler::APU_SoftReset()
{
}

int SnesAPUHandler::APU_Return_Cycle_Since_Last_Frame()
{
	return spc_sample_count(Spc) * 32;//32 samples per cycle
}

void SnesAPUHandler::APU_Rebuild_Sample_Memory(std::vector<Sample>& samp)
{
	APU_Set_Sample_Memory(samp);
	for (int x = 1; x < samp.size(); x++)
	{
		APU_Evaluate_BRR_Loop(&samp[x], samp[x].LoopEnd);
	}
	APU_Set_Sample_Directory(samp);
}

void SnesAPUHandler::APU_Debug_Dump_BRR()
{
	string filename = "BRR_Dump.bin";
	ofstream BRRFile(filename, ios::binary);
	for (int x = Sample_Mem_Page; x < LastSamplePoint; x++)
	{
		BRRFile << Spc->m.ram.ram[x];
	}
	BRRFile.close();
}

void SnesAPUHandler::APU_Debug_Dump_DIR()
{
	string filename = "DIR_Dump.bin";
	ofstream BRRFile(filename, ios::binary);
	for (int x = Sample_Dir_Page; x < 0xFFFF; x++)
	{
		BRRFile << Spc->m.ram.ram[x];
	}
	BRRFile.close();
}

void SnesAPUHandler::APU_Debug_Dump_SPC()
{
	string filename = "SPC_Dump.bin";
	ofstream BRRFile(filename, ios::binary);
	for (int x = 0; x < 65536; x++)
	{
		BRRFile << Spc->m.ram.ram[x];
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
		BRRFile << Spc->m.ram.ram[x];
	}
	BRRFile.close();
}

int SnesAPUHandler::APU_Debug_KON_State()
{
	return 0xFFFF;
}

int SnesAPUHandler::APU_Debug_KOF_State()
{
	return 0xFFFF;
}

int SnesAPUHandler::APU_Debug_PIT_State(int index, int byte)
{
	return 0xFFFF;
}

int SnesAPUHandler::APU_Debug_VOL_State(int index, int byte)
{
	return 0xFFFF;
}
