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
		return "~~~";
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

string Channel::EffectView2(int index)
{
	if (Rows[index].effect2 == NULL_COMMAND)
	{
		return "..";
	}
	else
	{
		char buf[10];
		sprintf_s(buf, "%02X", Rows[index].effect2);
		return buf;
	}
}

string Channel::Effectvalue2(int index)
{
	if (Rows[index].effectvalue2 == NULL_COMMAND)
	{
		return "..";
	}
	else
	{
		char buf[10];
		sprintf_s(buf, "%02X", Rows[index].effectvalue2);
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
	case 5:
		CurrentValue = Rows[index].effect2;
		break;
	case 6:
		CurrentValue = Rows[index].effectvalue2;
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
void Channel::SetUp(int Length)
{
	Row row = Row();
	row.note = NULL_COMMAND;
	row.octave = NULL_COMMAND;
	row.instrument = NULL_COMMAND;
	row.effect = NULL_COMMAND;
	row.effectvalue = NULL_COMMAND;
	row.effect2 = NULL_COMMAND;
	row.effectvalue2 = NULL_COMMAND;
	for (int i = 0; i < Length; i++)
	{
		Rows.push_back(row);
	}
}

void Channel::TickCheck(int RowIndex, vector<Instrument>& instruments, vector<Sample>& samples)
{
	Tickcheck = true;
	if (Rows[RowIndex].note != MAX_VALUE && Rows[RowIndex].instrument < NULL_COMMAND && Rows[RowIndex].instrument < instruments.size())
	{
		//This is when a note should be played
		CurrentInstrument = instruments[Rows[RowIndex].instrument].Index;
		PlayingNote = true;
	}
}