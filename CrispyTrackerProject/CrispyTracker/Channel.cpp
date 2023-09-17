#include "Channel.h"

string Channel::Row_View(int index)
{
	if (Rows[index].note == 255)
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
	if (Rows[index].note == 255)
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
	if (Rows[index].volume == 255)
	{
		return "--";
	}
	else if (Rows[index].volume < 16)
	{
		char buf[10];
		std::string s = "0";
		sprintf_s(buf, "%X", Rows[index].volume);
		return buf;
	}
	else
	{
		char buf[10];
		std::string s = "";
		sprintf_s(buf, "%X", Rows[index].volume);
		return buf;
	}
}

string Channel::InstrumentView(int index)
{
	if (Rows[index].instrument == 255)
	{
		return "--";
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
		return "--";
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
		return "--";
	}
	else
	{
		return to_string(Rows[index].effectvalue);
	}
}

int Channel::EvaluateHexInput(int input, int index)
{
	int surrogate = 0;
	
	surrogate = ((Rows[index].volume & 0x0f) << 4) + input;

	if (surrogate > 127)
	{
		surrogate = (127 & 0x0f) << 4;
	}

	return surrogate;
}

void Channel::SetUp(int Length)
{
	for (char i = 0; i < Length; i++)
	{
		Row row;
		Rows.push_back(row);
		Rows[i].note = 255;
		Rows[i].instrument = 255;
	}
}

void Channel::TickCheck()
{

}