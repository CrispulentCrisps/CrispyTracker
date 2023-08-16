#include "Channel.h"

void Channel::SetUp(int Length)
{
	for (char i = 0; i < Length; i++)
	{
		Rows.push_back("--- -- ---");
	}
}

void Channel::TickCheck()
{

}
