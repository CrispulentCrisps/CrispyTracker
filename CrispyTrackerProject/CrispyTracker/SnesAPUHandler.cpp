#include "SnesAPUHandler.h"

SnesAPUHandler* apu;				//Used to interface with SPC_CPU.h for debugging purposes

SnesAPUHandler::SnesAPUHandler()
{
	apu = this;
}

//Boots up the emulation core
void SnesAPUHandler::APU_Startup()
{
	//Starting up the DSP and SPC that the emu will use
	spc_init_rom(Spc, IPL_ROM);
	spc_reset(Spc);
	spc_clear_echo(Spc);
	spc_filter_clear(Filter);

	Spc->mute_voices(0x00);

	//Write IPL rom and base

	FILE* driver = fopen(DRIVER_PATH, "rb");
	for (int x = 0; x < DRIVER_CODE; x++)		//Clear out zeropage and stack
	{
		Spc->m.ram.ram[x] = 0;
	}
	fseek(driver, DRIVER_ROM_ADDR, SEEK_SET);
	for (int x = DRIVER_CODE; x < DRIVER_END; x++) //Load driver data into RAM
	{
		Spc->m.ram.ram[x] = fgetc(driver);
	}
	for (int x = DRIVER_END; x < DATA_START; x++)	//Clear out DIR page
	{
		Spc->m.ram.ram[x] = 0;
	}
	fclose(driver);
	Spc->m.ram.ram[DRIVER_LOAD_FLAG] = 1;
	Spc->m.cpu_regs.pc = DRIVER_CODE+0x28;//Remove 0x28 after debugging
	Spc->write_port(Spc->m.spc_time, 0, 0x00);
	Spc->write_port(Spc->m.spc_time, 1, 0x01);
	Spc->write_port(Spc->m.spc_time, 2, 0x00);
	Spc->write_port(Spc->m.spc_time, 3, 0x00);
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

	if (RunCPU) { spc_end_frame(Spc, ClockCycleRound * (EmuSpeed / DEFAULT_EMU_SPEED)); }
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

	APU_Handle_Emergencies();
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

u16 SnesAPUHandler::GetPitch(int index) {
	index = min((float)index, (float)MAX_PITCH_IND);
	index = max((float)index, 0.f);
	uint16_t out = Spc->m.ram.ram[PitchPtr + (index * 2)] | (Spc->m.ram.ram[PitchPtr + 1 + (index * 2)] << 8);
	return out;
}

//Writes to channel address for various , true width for 2 byte operations
void SnesAPUHandler::WriteChannelReg(int chan, short val, uint16_t addr, bool sfx, bool width)
{
	u16 address = addr + (8 * sfx);
	address += chan;
	if (width)
	{
		address += chan;
		Spc->m.ram.ram[address] = (val) & 0xFF;
		Spc->m.ram.ram[address + 1] = (val >> 8) & 0xFF;
	}
	else { Spc->m.ram.ram[address] = val; }
}

void SnesAPUHandler::APU_UpdateTuneMemory(vector<Instrument>& inst, vector<Sample>& sample, vector<Subtune>& sub, vector<Patterns>& pat, int subind)
{
	SPCPtr = DATA_START;
	MusicOrders.clear();
	SfxOrders.clear();
	APU_Generate_Pitch_Table();
	APU_Rebuild_Sample_Memory(sample);
	APU_Update_Instrument_Memory(pat, inst, sub[subind].TrackLength);
	APU_EvaluateSequenceData(pat, inst, sub[subind].TrackLength);
	APU_Write_Music_Orders(pat, sub);
	APU_Write_Subtunes();

	u16 cursubaddr = SubPtr + (subind * 2);
	u16 cursfxsubaddr = SfxPatPtr + (subind * 2);

	SPCPtr = DRIVER_INSTPTR;
	SPCWrite((InstPtr)				& 0xFF);
	SPCWrite((InstPtr >> 8)			& 0xFF);
	SPCWrite((OrderPtr)				& 0xFF);
	SPCWrite((OrderPtr >> 8)		& 0xFF);
	SPCWrite((SfxListPtr)			& 0xFF);
	SPCWrite((SfxListPtr >> 8)		& 0xFF);
	SPCWrite((SfxPatPtr)			& 0xFF);
	SPCWrite((SfxPatPtr >> 8)		& 0xFF);
	SPCWrite((SubPtr)				& 0xFF);
	SPCWrite((SubPtr >> 8)			& 0xFF);
	SPCWrite((PitchPtr)				& 0xFF);
	SPCWrite((PitchPtr >> 8)		& 0xFF);
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
		//cout << "\nPAT: " << x << " AT: " << std::hex << SPCPtr;
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

		if (x == pat.size() - 1)
		{
			WriteCommand(Command{ com_Stop, 0x0000 });
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
				//std::cout << "\nERROR: SAMPLE TOO LARGE\nADDR-OFF: " << SPCPtr << "\nBRR BLOCK: " << j;
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
	OrderPtr = SPCPtr;
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

	SfxPatPtr = SPCPtr;
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
	SubPtr = SPCPtr;
	for (int x = 0; x < MusicOrders.size(); x++)
	{
		//cout << "\nMusic Order: " << x << " | " << std::hex << MusicOrders[x];
		SPCWrite((MusicOrders[x]) & 0xFF);
		SPCWrite((MusicOrders[x] >> 8) & 0xFF);
	}

	SfxListPtr = SPCPtr;
	for (int x = 0; x < SfxOrders.size(); x++)
	{
		//cout << "\nSfx Order: " << x << " | " << std::hex << SfxOrders[x];
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
		Spc->m.ram.ram[Sample_Dir_Page + CurrentDir] =		samp[i].brr.SampleDir & 0xFF;			//Lo byte of directory
		Spc->m.ram.ram[Sample_Dir_Page + CurrentDir + 1] =	(samp[i].brr.SampleDir >> 8) & 0xFF;	//Hi byte of directory
		Spc->m.ram.ram[Sample_Dir_Page + CurrentDir + 2] =	samp[i].LoopStartAddr & 0xFF;			//Lo byte of the start
		Spc->m.ram.ram[Sample_Dir_Page + CurrentDir + 3] =	(samp[i].LoopStartAddr >> 8) & 0xFF;	//Hi byte of the start
		samp[i].SampleADDR = i - 1;
		CurrentDir += DirSize;
	}
}

//Writes page for instruments
void SnesAPUHandler::APU_Update_Instrument_Memory(std::vector<Patterns>& pat, std::vector<Instrument>& inst, int TrackSize)
{
	InstPtr = SPCPtr;
	//Write instrument table
	InstMem.clear();
	InstMem.push_back(InstEntry());//This one isn't counted, reason it's here is to mirror the instrument list having the first entry as a "Default" one
	//if (LastSamplePoint != 0) InstPtr = LastSamplePoint;//Assuming we have no samples in memory
	//else InstPtr = Sample_Mem_Page;
	//char buf[10];
	//sprintf_s(buf, "%04X", InstPtr);
	//std::cout << "\nInstADDR: " << buf;
	for (int x = 0; x < inst.size(); x++)
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
		i_ent.SampleIndex = inst[x].CurrentSample.SampleIndex-1;

		i_ent.EffectState |= ((int)inst[x].PitchMod << 0) | ((int)inst[x].Noise << 1) | ((int)inst[x].Echo << 2);

		InstMem.push_back(i_ent);
	}

	for (int x = 1; x < InstMem.size(); x++)
	{
		SPCWrite(InstMem[x].SampleIndex);
		SPCWrite(InstMem[x].ADSR1);
		SPCWrite(InstMem[x].ADSR2);
		SPCWrite(InstMem[x].Gain);
		SPCWrite(InstMem[x].EffectState);
	}
}
void SnesAPUHandler::APU_Generate_Pitch_Table()
{
	PitchPtr = SPCPtr;
	for (float x = 0; x < MAX_PITCH_IND; x++)
	{
		float basepit = ((pow(2.0, ((x - (MAX_PITCH_IND/2)) / 12.0)) * BASE_PITCH_RATE * 16.0) / 125.0);
		basepit = max(basepit, (float)0);
		basepit = min(basepit, (float)0x3FFF);
		u16 pitval = (u16)basepit;
		SPCWrite((pitval) & 0xFF);
		SPCWrite((pitval >> 8) & 0xFF);
		//cout << "\nPitch: " << x << " | " << std::hex << (u16)basepit;
	}
}
//Sets master volume of the track
bool SnesAPUHandler::APU_Set_Master_Vol(signed char vol)
{
	if (vol >= -128 && vol <= 127) return true;
	else
	{
		//std::cout << "\nEMU ERROR: VOL NOT WITHIN -128 & 127: VOL --> " << vol;
		return false;
	}
}

//Update the echo registers
void SnesAPUHandler::APU_Set_Echo(unsigned int dtime, int* coef, signed int dfb, signed int dvol)
{
	//int EchoAddr = (0xFFFF - (dtime * 0x0800)) >> 8;	
	uint8_t EchoAddr = (0xFF00 - (dtime * 0x0800)) >> 8;
	Spc->dsp.write(Spc->dsp.r_esa, EchoAddr);
}

//Initialises the echo values
void SnesAPUHandler::APU_Init_Echo()
{

}

//Read row values in for tracker and jam into specific registers based on channel index
void SnesAPUHandler::APU_ReadRows(Row* rows, ChannelState* cs)
{
	uint8_t konstate = Spc->dsp.read(KON_REG);
	uint8_t koffstate = Spc->dsp.read(KOFF_REG);
	for (int x = 0; x < 8; x++)
	{
		u16 pit = cs[x].pit;
		u16 vol = cs[x].vol;
		u8 inst = cs[x].inst;

		//Instruments
		if (rows[x].instrument < NULL_COMMAND)
		{
			inst = rows[x].instrument;
			cs[x].inst = inst;
		}

		//Notes
		if (rows[x].note < NULL_COMMAND)
		{
			if (Spc->dsp.read(NON_REG) & (1 << x))
			{
				u8 flag = Spc->dsp.read(FLG_REG);
				flag &= 0xE0;
				flag |= (rows[x].note & 0x1F);
				Spc->dsp.write(FLG_REG, flag);
			}
			pit = GetPitch(rows[x].note);
			cs[x].pit = pit;
			konstate |= (1 << x);
			koffstate &= (1 << x) ^ 0xFF;
		}
		else if (rows[x].note == RELEASE_COMMAND)
		{
			konstate &= (1 << x) ^ 0xFF;
			koffstate |= (1 << x);
		}
		else { konstate &= (1 << x) ^ 0xFF; }

		//Volume
		if (rows[x].volume < NULL_COMMAND)
		{
			vol = rows[x].volume | (rows[x].volume << 8);
			cs[x].vol = vol;
		}
		
		//Effects
		if (rows[x].effect < NULL_COMMAND) 
		{

		}

		if (rows[x].effect2 < NULL_COMMAND) 
		{

		}

		WriteChannelReg(x,	pit,	DRIVER_PITCHES, false,	true);
		WriteChannelReg(x,	vol,	DRIVER_VOLUME,	false,	true);
		WriteChannelReg(x,	inst,	DRIVER_INST,	false,	false);
	}

	Spc->m.ram.ram[DRIVER_KON_STATE] = konstate;
	Spc->dsp.write(KOFF_REG, koffstate);
}

void SnesAPUHandler::APU_Start_Tune(int subind, ChannelState* cs)
{
	Spc->m.ram.ram[DRIVER_MASTER_VOL] = 0x7F;
	Spc->dsp.write(KON_REG,		0x00);
	Spc->dsp.write(KOFF_REG,	0x00);
	Spc->dsp.write(PMON_REG,	0x00);
	Spc->dsp.write(EON_REG,		0x00);
	Spc->dsp.write(NON_REG,		0x00);
	Spc->dsp.write(MASTERVOL_L,	0x7F);
	Spc->dsp.write(MASTERVOL_R,	0x7F);
	for (int x = 0; x < 8; x++)
	{
		cs[x].vol =		0x7F7F;
		cs[x].pit =		0x0000;
		cs[x].inst =	0x0000;
	}
}

void SnesAPUHandler::APU_Audio_Stop()
{
	Spc->dsp.write(KON_REG, 0x00);
	Spc->dsp.write(KOFF_REG, 0xFF);
}

void SnesAPUHandler::APU_Audio_Start()
{
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

void SnesAPUHandler::APU_Handle_Emergencies()
{
	if (Spc->m.cpu_regs.pc < 0x1FF)
	{
		cout << "\nERROR: PC < $01FF, POTENTIAL OVERFLOW OR BAD POINTER TO MEMORY!";
		RunCPU = false;
	}
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
	for (int x = 0; x < 0xFFFF; x++)
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
	for (int x = InstPtr; x < OrderPtr; x++)
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

int SnesAPUHandler::APU_Debug_Read_Port(int index) { return Spc->read_port(Spc->m.spc_time, index); }
