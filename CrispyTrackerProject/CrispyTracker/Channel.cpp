#include "Channel.h"

string Channel::Row_View(int index)
{
	if (Rows[index].note == 255)
	{
		return "--- " + to_string(Rows[index].instrument) + " " + to_string(Rows[index].volume) + " " + to_string(Rows[index].effect);
	}
	else
	{
		return NoteNames[Rows[index].note] + to_string(Rows[index].octave) + " " + to_string(Rows[index].instrument) + " " + to_string(Rows[index].volume) + " " + to_string(Rows[index].effect);
	}
}

string Channel::NoteView(int index)
{
	if (Rows[index].note == 255)
	{
		return "--- ";
	}
	else
	{
		return NoteNames[Rows[index].note];
	}
}

string Channel::VolumeView(int index)
{
	if (Rows[index].volume == 255)
	{
		return "-- ";
	}
	else
	{
		return to_string(Rows[index].volume);
	}
}

string Channel::InstrumentView(int index)
{
	if (Rows[index].instrument == 255)
	{
		return "-- ";
	}
	else
	{
		return to_string(Rows[index].instrument);
	}
}

string Channel::EffectView(int index)
{
	if (Rows[index].effect == 255)
	{
		return "-- ";
	}
	else
	{
		return to_string(Rows[index].effect);
	}
}

string Channel::Effectvalue(int index)
{
	if (Rows[index].effectvalue == 255)
	{
		return "---";
	}
	else
	{
		return to_string(Rows[index].effectvalue);
	}
}

void Channel::SetUp(int Length)
{
	for (char i = 0; i < Length; i++)
	{
		Row row;
		row.note = 255;
		row.instrument = 255;
		row.effect = 255;
		row.effectvalue = 255;
		row.octave = 4;
		Rows.push_back(row);
	}
}

void Channel::TickCheck()
{

}