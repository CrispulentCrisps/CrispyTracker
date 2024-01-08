#include "Sample.h"

void Sample::BRRConvert()
{
	printf("\n SAMPLE CONVERSION BEGIN\n");
	int BlockAmount = ceil(sizeof(SampleData) / (float)16);
	// += 2 because we're filling in 2 data blocks at the same time
	for (int i = 0; i < BlockAmount; i += 2)
	{		
		//Fill data block
		for (int y = 0; y < 2; y++)
		{
			//Create new data block
			brr.DBlocks.push_back(brr.BlankBlock);
		
			printf("\nDATA BLOCK: ");
			printf(std::to_string(BlockAmount).data());
			printf("\nDATA COMPARE:");
			for (int x = 0; x < 8; x++)
			{
				//One instance of the sample data can fill in 2 DataBlocks
				//Shift operation gets the seperate bytes from the short and the 0x0F does some tomfuckery that I've not a clue of
				brr.DBlocks[i + y].DataByte[x] = (SampleData[x] >> (8*y)) & 0x0F;

				printf("\n DATA BYTE: ");
				printf(std::to_string(brr.DBlocks[i + y].DataByte[x]).data());
				
				printf("\n SAMPLE BYTE: ");
				printf(std::to_string(SampleData[x]).data());
			}
		}
	}
	printf("\nSAMPLE CONVERSION ENDED\n");
}