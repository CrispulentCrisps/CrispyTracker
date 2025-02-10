#include "FileHandler.h"

template<typename T> T AlignData(void* base, size_t offset) 
{
	T returnval;
	memcpy(&returnval, (char*)base+offset, sizeof(T));
	return returnval;
}

int GetStringLength(long filelen, void* compbuf, int addroff)
{
	int stringlength = 0;
	while (stringlength < filelen)
	{
		if (AlignData<uint8_t>(compbuf, addroff + stringlength) == 0)
		{
			break;
		}
		stringlength++;
	}
	return stringlength;
}

bool FileHandler::LoadModule(string path)
{
	FILE* file = fopen(path.c_str(), "rb");
	//Decompression
	fseek(file, 0, SEEK_END);
	long filelen = ftell(file);
	rewind(file);
	void* compfiledat = malloc(filelen);
	if (compfiledat != 0)
	{
		fread(compfiledat, 1, filelen, file);
	}
	fclose(file);
	mz_ulong bufsize = mz_compressBound(filelen);
	bufsize = AlignData<uint32_t>(compfiledat, 0);
	void* compbuf = malloc(bufsize);
	mz_uncompress((unsigned char*)compbuf, &bufsize, (const unsigned char*)compfiledat+4, (mz_ulong)filelen-4);
	free(compfiledat);

	std::vector<int> ord;
	std::vector<Patterns> ordvec[8];
	int addroff = 0;
	if (AlignData<uint32_t>(compbuf, addroff) == FILE_HEAD)
	{
		addroff += 4;
		if (AlignData<uint16_t>(compbuf, addroff) != VERSION_100)
		{
			cout << "\n\nWARNING: FILE VERSIONS MISMATCH, VERSION LOADED WAS: " << AlignData<uint16_t>(compbuf, addroff);
			return false;
		}
		addroff += 2;
		uint8_t SubtuneCount = AlignData<uint8_t>(compbuf, addroff++);
		mod.subtune.resize(SubtuneCount);
		for (uint8_t z = 0; z < SubtuneCount; z++)
		{
			mod.subtune[z].AuthorName = std::string((char*)compbuf + addroff, GetStringLength(filelen, compbuf, addroff));
			addroff += GetStringLength(filelen, compbuf, addroff) + 1;
			mod.subtune[z].TrackName = std::string((char*)compbuf + addroff, GetStringLength(filelen, compbuf, addroff));
			addroff += GetStringLength(filelen, compbuf, addroff) + 1;
			mod.subtune[z].TrackDesc = std::string((char*)compbuf + addroff, GetStringLength(filelen, compbuf, addroff));
			addroff += GetStringLength(filelen, compbuf, addroff) + 1;

			mod.subtune[z].TrackLength = AlignData<uint8_t>(compbuf, addroff++);
			mod.subtune[z].SongLength = AlignData<uint8_t>(compbuf, addroff++);
			mod.subtune[z].Speed1 = AlignData<uint8_t>(compbuf, addroff++);
			mod.subtune[z].TempoDivider = AlignData<uint8_t>(compbuf, addroff++);
			mod.subtune[z].Highlight1 = AlignData<uint8_t>(compbuf, addroff++);
			mod.subtune[z].Highlight2 = AlignData<uint8_t>(compbuf, addroff++);
			mod.subtune[z].SFXFlag = AlignData<uint8_t>(compbuf, addroff++);

			for (int x = 0; x < 8; x++)
			{
				mod.subtune[z].Orders[x].clear();
				mod.subtune[z].Orders[x].resize(mod.subtune[z].SongLength);
				ordvec[x].resize(mod.subtune[z].SongLength);
			}

			for (int y = 0; y < mod.subtune[z].SongLength; y++)
			{
				for (int x = 0; x < 8; x++)
				{
					ordvec[x][y].Index = AlignData<uint8_t>(compbuf, addroff++);
					mod.subtune[z].Orders[x][y] = ordvec[x][y].Index;
				}
			}

			mod.SelectedRegion = AlignData<uint8_t>(compbuf, addroff++);
			mod.subtune[z].EchoVol = AlignData<int8_t>(compbuf, addroff++);
			mod.subtune[z].Delay = AlignData<uint8_t>(compbuf, addroff++);
			mod.subtune[z].Feedback = AlignData<uint8_t>(compbuf, addroff++);
			for (int x = 0; x < 8; x++)
			{
				mod.subtune[z].EchoFilter[x] = AlignData<int8_t>(compbuf, addroff++);
			}
		}

		int samplelen = AlignData<uint8_t>(compbuf, addroff++);

		std::vector<Sample> sampvec;
		sampvec.resize(samplelen);
		for (int x = 0; x < samplelen; x++)
		{
			sampvec[x].SampleIndex = AlignData<uint8_t>(compbuf, addroff++);
			sampvec[x].SampleADDR = AlignData<uint16_t>(compbuf, addroff);
			addroff += 2;
			sampvec[x].SampleName = std::string((char*)compbuf + addroff, GetStringLength(filelen, compbuf, addroff));
			addroff += GetStringLength(filelen, compbuf, addroff) + 1;
			int sampdatalen = AlignData<uint32_t>(compbuf, addroff);
			addroff += 4;
			sampvec[x].SampleData.resize(sampdatalen);
			memcpy(sampvec[x].SampleData.data(), (char*)compbuf + addroff, sampdatalen * 2);
			addroff += sampdatalen * 2;
			sampvec[x].SampleRate = AlignData<uint32_t>(compbuf, addroff);
			addroff += 4;
			sampvec[x].FineTune = AlignData<uint32_t>(compbuf, addroff);
			addroff += 4;
			sampvec[x].Loop = !!AlignData<uint8_t>(compbuf, addroff++);
			sampvec[x].LoopStart = AlignData<uint32_t>(compbuf, addroff);
			addroff += 4;
			sampvec[x].LoopEnd = AlignData<uint32_t>(compbuf, addroff);
			addroff += 4;
		}
		mod.samples = sampvec;


		std::vector<Instrument> inst;

		int instlen = AlignData<uint8_t>(compbuf, addroff++);
		std::vector<Instrument> instvec;
		instvec.resize(instlen);

		for (int x = 0; x < instlen; x++)
		{
			instvec[x].Index = AlignData<uint8_t>(compbuf, addroff++);
			instvec[x].Name = std::string((char*)compbuf + addroff, GetStringLength(filelen, compbuf, addroff));
			addroff += GetStringLength(filelen, compbuf, addroff) + 1;
			instvec[x].SampleIndex = AlignData<uint8_t>(compbuf, addroff++);
			instvec[x].Volume = AlignData<int8_t>(compbuf, addroff++);
			instvec[x].LPan = AlignData<int8_t>(compbuf, addroff++);
			instvec[x].RPan = AlignData<int8_t>(compbuf, addroff++);
			instvec[x].Gain = AlignData<uint8_t>(compbuf, addroff++);
			instvec[x].InvL = !!AlignData<uint8_t>(compbuf, addroff++);
			instvec[x].InvR = !!AlignData<uint8_t>(compbuf, addroff++);
			instvec[x].PitchMod = !!AlignData<uint8_t>(compbuf, addroff++);
			instvec[x].Echo = !!AlignData<uint8_t>(compbuf, addroff++);
			instvec[x].Noise = !!AlignData<uint8_t>(compbuf, addroff++);
			instvec[x].EnvelopeUsed = !!AlignData<uint8_t>(compbuf, addroff++);
			instvec[x].Attack = AlignData<uint8_t>(compbuf, addroff++);
			instvec[x].Decay = AlignData<uint8_t>(compbuf, addroff++);
			instvec[x].Sustain = AlignData<uint8_t>(compbuf, addroff++);
			instvec[x].Release = AlignData<uint8_t>(compbuf, addroff++);
			instvec[x].NoteOff = AlignData<int8_t>(compbuf, addroff++);
			instvec[x].InstADDR = AlignData<uint16_t>(compbuf, addroff);
			addroff += 2;
		}
		mod.inst = instvec;

		std::vector<Patterns> pat;

		int patlen = AlignData<uint8_t>(compbuf, addroff++);
		std::vector<Patterns> patvec;
		patvec.resize(patlen);
		for (int x = 0; x < patlen; x++)
		{
			patvec[x].Index = AlignData<uint8_t>(compbuf, addroff++);
			memcpy(patvec[x].SavedRows, (char*)compbuf + addroff, sizeof(patvec[x].SavedRows));
			addroff += sizeof(patvec[x].SavedRows);
		}
		mod.patterns = patvec;

		return true;
	}
	else
	{
		cout << "ERROR: WRONG FILE LOADED!!!";
		return false;
	}
}

bool FileHandler::SaveModule(string path)
{
	cout << "\n" << path;
	FILE* file = fopen(path.c_str(), "wb+");

	if (file != 0)
	{
		//Tracker specific
		uint32_t head = FILE_HEAD;
		fwrite(&head, 4, 1, file);
		uint16_t vers = VERSION_100;
		fwrite(&vers, 2, 1, file);
		uint8_t SubtuneCount = mod.subtune.size();
		fwrite(&SubtuneCount, 1, 1, file);
		for (int x = 0; x < mod.subtune.size(); x++)
		{
			fwrite(mod.subtune[x].AuthorName.c_str(), 1, mod.subtune[x].AuthorName.length(), file);
			fputc(0, file);
			fwrite(mod.subtune[x].TrackName.c_str(), 1, mod.subtune[x].TrackName.length(), file);
			fputc(0, file);
			fwrite(mod.subtune[x].TrackDesc.c_str(), 1, mod.subtune[x].TrackDesc.length(), file);
			fputc(0, file);
			fwrite(&mod.subtune[x].TrackLength, 1, 1, file);
			fwrite(&mod.subtune[x].SongLength, 1, 1, file);
			fwrite(&mod.subtune[x].Speed1, 1, 1, file);
			fwrite(&mod.subtune[x].TempoDivider, 1, 1, file);
			fwrite(&mod.subtune[x].Highlight1, 1, 1, file);
			fwrite(&mod.subtune[x].Highlight2, 1, 1, file);
			fwrite(&mod.subtune[x].SFXFlag, 1, 1, file);

			cout << "\nOrder memory starts at: " << ftell(file);
			//Orders
			for (int y = 0; y < mod.subtune[x].SongLength; y++)
			{
				for (int z = 0; z < 8; z++)
				{
					uint8_t ord = mod.subtune[x].Orders[z][y];
					fwrite(&ord, 1, 1, file);
				}
			}

			fwrite(&mod.SelectedRegion, 1, 1, file);
			fwrite(&mod.subtune[x].EchoVol, 1, 1, file);
			fwrite(&mod.subtune[x].Delay, 1, 1, file);
			fwrite(&mod.subtune[x].Feedback, 1, 1, file);
			for (int z = 0; z < 8; z++)
			{
				fwrite(&mod.subtune[x].EchoFilter[z], 1, 1, file);
			}
			cout << "\nEcho memory starts at: " << ftell(file);
		}

		//Samples
		int sampsize = mod.samples.size() - 1;
		assert(sampsize < 256);
		fwrite(&sampsize, 1, 1, file);

		for (int s = 1; s < mod.samples.size(); s++)
		{
			fwrite(&mod.samples[s].SampleIndex, 1, 1, file);
			fwrite(&mod.samples[s].SampleADDR, 2, 1, file);
			fwrite(mod.samples[s].SampleName.c_str(), 1, mod.samples[s].SampleName.length(), file);
			fputc(0, file);

			uint32_t sampdatasize = mod.samples[s].SampleData.size();
			fwrite(&sampdatasize, sizeof(sampdatasize), 1, file);

			fwrite(mod.samples[s].SampleData.data(), 2, mod.samples[s].SampleData.size(), file);

			fwrite(&mod.samples[s].SampleRate, 4, 1, file);
			fwrite(&mod.samples[s].FineTune, 4, 1, file);
			fputc(mod.samples[s].Loop ? 1 : 0, file);
			fwrite(&mod.samples[s].LoopStart, 4, 1, file);
			fwrite(&mod.samples[s].LoopEnd, 4, 1, file);

		}

		cout << "\nInstrument memory starts at: " << ftell(file);
		//Instruments
		int instsize = mod.inst.size() - 1;
		fwrite(&instsize, 1, 1, file);
		for (int i = 1; i < instsize+1; i++)
		{
			Instrument& in = mod.inst[i];
			fwrite(&in.Index, 1, 1, file);
			fwrite(in.Name.c_str(), 1, in.Name.length(), file);
			fputc(0, file);
			fwrite(&in.SampleIndex, 1, 1, file);
			int8_t vol = in.Volume;
			fwrite(&vol, 1, 1, file);
			int8_t lpan = in.Volume;
			fwrite(&lpan, 1, 1, file);
			int8_t rpan = in.Volume;
			fwrite(&rpan, 1, 1, file);

			fwrite(&in.Gain, 1, 1, file);

			fputc(in.InvL ? 1 : 0, file);
			fputc(in.InvR ? 1 : 0, file);
			fputc(in.PitchMod ? 1 : 0, file);
			fputc(in.Echo ? 1 : 0, file);
			fputc(in.Noise ? 1 : 0, file);
			fputc(in.EnvelopeUsed ? 1 : 0, file);
			fwrite(&in.Attack, 1, 1, file);
			fwrite(&in.Decay, 1, 1, file);
			fwrite(&in.Sustain, 1, 1, file);
			fwrite(&in.Release, 1, 1, file);
			uint8_t noff = in.NoteOff;
			fwrite(&noff, 1, 1, file);
			fwrite(&in.InstADDR, 2, 1, file);

		}

		//SNES specific
		cout << "\nPattern memory starts at: " << ftell(file);

		uint8_t patternslen = mod.patterns.size();
		fwrite(&patternslen, 1, 1, file);
		//Patterns
		for (Patterns& pat : mod.patterns) 
		{
			fwrite(&pat.Index, 1, 1, file);
			fwrite(pat.SavedRows, sizeof(Row), 256, file);
		}

		if (Compress)
		{
			//Compression
			int byteamount = ftell(file);
			rewind(file);
			void* mpoint = malloc(byteamount);
			if (mpoint != NULL)
			{
				fread(mpoint, 1, byteamount, file);
			}
			else
			{
				cout << "\nFILE ERROR: " << ferror;
				return false;
			}
			void* compbuf = malloc(mz_compressBound(byteamount));
			mz_ulong bufsize = mz_compressBound(byteamount);
			mz_compress((unsigned char*)compbuf, &bufsize, (const unsigned char*)mpoint, (mz_ulong)byteamount);
			free(mpoint);
			fclose(file);
			file = fopen(path.c_str(), "wb");
			if (compbuf != 0)
			{
				fwrite(&byteamount, 4, 1, file);
				fwrite(compbuf, 1, bufsize, file);
			}
			else
			{
				cout << "\nFILE ERROR: " << ferror;
				return false;
			}
			fclose(file);
			free(compbuf);
			file = 0;
		}
		else
		{
			fclose(file);
		}
		return true;
	}
	else
	{
		return false;
	}
}
