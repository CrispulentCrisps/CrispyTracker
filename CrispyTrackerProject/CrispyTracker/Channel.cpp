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
			s += SampleData[i] * sinc(x - i);
			//cout << "\nResample Accum: " << s;
		}
	}

	return s;
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

void Channel::TickCheck(int RowIndex, vector<Instrument>& inst)
{
	if (Rows[RowIndex].note != MAX_VALUE && Rows[RowIndex].instrument != MAX_VALUE)
	{
		//This is when a note should be played
		CurrentInstrument = inst[Rows[RowIndex].instrument].Index;
		CurrentSamplePointIndex = 0;
		PlayingNote = true;
		CurrentPlayedNote = Rows[RowIndex].note - 60;//the - 60 is for the offset needed to center the pitch
		//cout << "\nChannel Hit" << " - Current Note: " << CurrentPlayedNote << " - Actual Note: " << Rows[RowIndex].note;
	}
}
	
void Channel::UpdateChannel(vector<Instrument>& inst, vector<Sample>& samples)
{
	if (CurrentInstrument > 0 && samples[inst[CurrentInstrument].SampleIndex].SampleData.size() > 0)
	{
		if (CurrentSamplePointIndex >= samples[inst[CurrentInstrument].SampleIndex].SampleData.size())
		{
			CurrentSamplePointIndex = 0;
			PlayingNote = false;
		}
		//Vector hell
		AudioDataL = Resample(samples[inst[CurrentInstrument].SampleIndex].SampleData);
		AudioDataR = Resample(samples[inst[CurrentInstrument].SampleIndex].SampleData);
	}
}