#include "Channel.h"

string Channel::Row_View(int index)
{
	if (Rows[index].note == MAX_VALUE)
	{
		return "---" + to_string(Rows[index].instrument) + " " + to_string(Rows[index].volume) + " " + to_string(Rows[index].effect);
	}
	else
	{
		cout << "\n" << index;
		return NoteNames[Rows[index].note] + to_string(Rows[index].octave) + " " + to_string(Rows[index].instrument) + " " + to_string(Rows[index].volume) + " " + to_string(Rows[index].effect);
	}
}

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
		return "--";
	}
	else if (Rows[index].volume < 16)
	{
		char buf[10];
		sprintf_s(buf, "%X", Rows[index].volume);
		return buf;
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
		return "--";
	}
	else if (Rows[index].instrument < 16)
	{
		char buf[10];
		sprintf_s(buf, "%X", Rows[index].instrument);
		return buf;
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
		return "--";
	}
	else if (Rows[index].effect < 16)
	{
		char buf[10];
		sprintf_s(buf, "%X", Rows[index].effect);
		return buf;
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
		return "--";
	}
	else if (Rows[index].effectvalue < 16)
	{
		char buf[10];
		sprintf_s(buf, "%X", Rows[index].effectvalue);
		return buf;
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

	if (input == 0 && ModValue > max)
	{
		surrogate = 0;
	}
	else
	{
		surrogate = ((ModValue & 0x0f) << 4) + input;
	}

	//Clamp input
	/*
	if (surrogate > max && surrogate != 256)
	{
		surrogate = 127;
	}
	*/
	return surrogate;
}

void Channel::SetUp(int Length)
{
	for (char i = 0; i < Length; i++)
	{
		Row row;
		Rows.push_back(row);
		Rows[i].note = MAX_VALUE;
		Rows[i].instrument = MAX_VALUE;
	}
}

void Channel::TickCheck()
{

}