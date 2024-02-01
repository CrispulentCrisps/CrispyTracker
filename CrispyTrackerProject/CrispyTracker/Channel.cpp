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
		return NoteNames[Rows[index].note%12] + to_string(Rows[index].octave);
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
		sprintf_s(buf, "%X", Rows[index].volume);
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
		sprintf_s(buf, "%X", Rows[index].instrument);
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
		sprintf_s(buf, "%X", Rows[index].effect);
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
		sprintf_s(buf, "%X", Rows[index].effectvalue);
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

void Channel::SetUp(int Length)
{
	Row row;
	row.note = MAX_VALUE;
	row.instrument = MAX_VALUE;
	for (int i = 0; i < Length; i++)
	{
		Rows.push_back(row);
	}
}

void Channel::TickCheck(int RowIndex, vector<Instrument> inst)
{
	if (Rows[RowIndex].note != MAX_VALUE && Rows[RowIndex].instrument != MAX_VALUE)
	{
		//This is when a note should be played
		CurrentInstrument = inst[Rows[RowIndex].instrument].Index;
		CurrentSamplePointIndex = 0;
	}
}

void Channel::UpdateChannel(vector<Instrument>& inst, vector<Sample>& samples)
{
	//Vector hell
	if (CurrentInstrument != 0)
	{
		AudioDataL = samples[inst[CurrentInstrument].SampleIndex].SampleData[CurrentSamplePointIndex];
		AudioDataR = samples[inst[CurrentInstrument].SampleIndex].SampleData[CurrentSamplePointIndex];
	}
}
