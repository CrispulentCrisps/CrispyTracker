#include "Channel.h"

void Channel::SetUp(int Length)
{
	for (size_t i = 0; i < Length; i++)
	{
		Rows.push_back(i + "--- -- ---");
	}
}

void Channel::TickCheck()
{

}
