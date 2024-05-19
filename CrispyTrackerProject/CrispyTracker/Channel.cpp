#include "Channel.h"

#if CT_UNIX
#define sprintf_s sprintf
#endif

string Channel::NoteView(int index)
{
	if (Rows[index].note == MAX_VALUE)
	{
		return "---";
	}
	else
	{
		switch (NoteType)
		{
		case 0://Sharp
			return NoteNames[Rows[index].note % 12] + to_string(Rows[index].octave);
			break;
		case 1://Flat
			return NoteNames_FL[Rows[index].note % 12] + to_string(Rows[index].octave);
			break;
		case 2://German
			return NoteNames_GR[Rows[index].note % 12] + to_string(Rows[index].octave);
			break;
		default:
			return NoteNames[Rows[index].note % 12] + to_string(Rows[index].octave);
			break;
		}
	}
}

string Channel::VolumeView(int index)
{
	if (Rows[index].volume == MAX_VALUE)
	{
		return "..";
	}
	else
	{
		char buf[10];
		sprintf_s(buf, "%02X", Rows[index].volume);
		return buf;
	}
}

string Channel::InstrumentView(int index)
{
	if (Rows[index].instrument == MAX_VALUE)
	{
		return "..";
	}
	else
	{
		char buf[10];
		sprintf_s(buf, "%02X", Rows[index].instrument);
		return buf;
	}
}

string Channel::EffectView(int index)
{
	if (Rows[index].effect == MAX_VALUE)
	{
		return "..";
	}
	else
	{
		char buf[10];
		sprintf_s(buf, "%02X", Rows[index].effect);
		return buf;
	}
}

string Channel::Effectvalue(int index)
{
	if (Rows[index].effectvalue == MAX_VALUE)
	{
		return "..";
	}
	else
	{
		char buf[10];
		sprintf_s(buf, "%02X", Rows[index].effectvalue);
		return buf;
	}
}

int Channel::EvaluateHexInput(int input, int index, int max, int valuetype)
{
	//Value type gets the subcolumn in the given row
	int surrogate = 0;
	int ModValue = 0;

	switch (valuetype)
	{
	case 1:
		ModValue = Rows[index].instrument;
		break;
	case 2:
		ModValue = Rows[index].volume;
		break;
	case 3:
		ModValue = Rows[index].effect;
		break;
	case 4:
		ModValue = Rows[index].effectvalue;
		break;
	}

	if (input == 0 || ModValue > max)
	{
		surrogate = 0;
	}
	else
	{
		surrogate = ((ModValue & 0x0f) << 4) + input;
	}

	//Clamp input
	
	if (surrogate > max && surrogate != 256)
	{
		surrogate = 127;
	}
	return surrogate;
}

float Channel::Resample(vector<Sint16>& SampleData)
{
	//https://en.wikipedia.org/wiki/Lanczos_resampling using this for the resample method
	//<3 Wikipedia my beloved <3

	float s = 0;
	int a = RESAMPLE_QUALITY;
	float x = CurrentSamplePointIndex;

	for (int i = (int)x - a; i <= (int)x + a; i++)
	{
		if (i > 0 && i < SampleData.size())
		{
			s += (float)SampleData[i] * sinc(x - i);
		}
	}

	return s;
}

void Channel::SetUp(int Length)
{
	Row row = Row();
	row.note = MAX_VALUE;
	row.instrument = MAX_VALUE;
	for (int i = 0; i < Length; i++)
	{
		Rows.push_back(row);
	}
}

void Channel::TickCheck(int RowIndex, vector<Instrument>& instruments, vector<Sample>& samples)
{
	Tickcheck = true;
	int CurrentSample = instruments[CurrentInstrument].SampleIndex;//Dictates the current sample [Here to make the code look cleaner]
	if (Rows[RowIndex].note != MAX_VALUE && Rows[RowIndex].instrument != MAX_VALUE)
	{
		//This is when a note should be played
		CurrentInstrument = instruments[Rows[RowIndex].instrument].Index;
		CurrentSamplePointIndex = 0;
		PlayingNote = true;
		CurrentPlayedNote = (float)Rows[RowIndex].note - 60.0 + samples[CurrentSample].NoteOffset;//the - 60 is for the offset needed to center the pitch
		cout << "\nChannel Hit" << "\nCurrent Note: " << CurrentPlayedNote << "\nCurrent Row " << RowIndex;
	}
}
	
void Channel::UpdateChannel(vector<Instrument>& instruments, vector<Sample>& samples)
{
	int CurrentSample = instruments[CurrentInstrument].SampleIndex;//Dictates the current sample [Here to make the code look cleaner]

	if (CurrentInstrument > 0 && samples[CurrentSample].SampleData.size() > 0)
	{
		AudioDataL = Resample(samples[CurrentSample].SampleData);
		AudioDataR = Resample(samples[CurrentSample].SampleData);
	}
}