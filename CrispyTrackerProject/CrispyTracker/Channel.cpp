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
	else if (Rows[index].note == STOP_COMMAND)
	{
		return "OFF";
	}
	else if (Rows[index].note == RELEASE_COMMAND)
	{
		return "!!!";
	}
	else
	{
		switch (NoteType)
		{
		default:
		case 0://Sharp
			return NoteNames[Rows[index].note % 12] + to_string(Rows[index].octave);
			break;
		case 1://Flat
			return NoteNames_FL[Rows[index].note % 12] + to_string(Rows[index].octave);
			break;
		case 2://German
			return NoteNames_GR[Rows[index].note % 12] + to_string(Rows[index].octave);
			break;
		}
	}
}

string Channel::VolumeView(int index)
{
	if (Rows[index].volume == NULL_COMMAND)
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
	if (Rows[index].instrument == NULL_COMMAND)
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
	if (Rows[index].effect == NULL_COMMAND)
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
	if (Rows[index].effectvalue == NULL_COMMAND)
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
	int NewValue = 0;
	int CurrentValue = 0;

	switch (valuetype)
	{
	case 1:
		CurrentValue = Rows[index].instrument;
		break;
	case 2:
		CurrentValue = Rows[index].volume;
		break;
	case 3:
		CurrentValue = Rows[index].effect;
		break;
	case 4:
		CurrentValue = Rows[index].effectvalue;
		break;
	}

	if (CurrentValue <= max)
	{
		NewValue = ((CurrentValue & 0x0f) << 4) + input;
	}
	//Clamp input
	else if (NewValue != 256)
	{
		NewValue = input;
	}
	return NewValue;
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
		cout << "\nChannel Hit!" << "\nCurrent Note: " << Rows[RowIndex].note << "\nCurrent Row " << RowIndex;
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