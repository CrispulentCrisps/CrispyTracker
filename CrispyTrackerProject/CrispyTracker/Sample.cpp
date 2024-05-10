#include "Sample.h"

//thank you Blumba for the bit shift help :D
void Sample::BRRConvert()
{
    printf("\n SAMPLE CONVERSION BEGIN\n");
    int BlockAmount = (SampleData.size()) / 16;

    printf("/n BLOCKS ALLOCATED:");
    printf(std::to_string(BlockAmount).data());
    signed char CombinedBit = 0;
    for (int i = 0; i < BlockAmount; i++)
    {
        brr.DBlocks.push_back(brr.BlankBlock);

        if (BlockAmount == i-1)//Assume we're on the last sample point
        {
            brr.DBlocks[i].HeaderByte = EndFlag;//Assign end flag
            brr.DBlocks[i].HeaderByte |= 0b00010000;//Shift value should be 1 to avoid the single R-shift that can remove audio data
        }

        for (int x = 0; x < 8; x++)
        {
            //Combined bit is taking 2 4bit vales, shifts the second 4 bits to fit in the byte and not overwrite the first 4 bits
            CombinedBit = ((SampleData[16 * i + 2 * x + 0] >> 12) & 0x0f) | ((SampleData[16 * i + 2 * x + 1] >> 8) & 0xf0);
            brr.DBlocks[i].DataByte[x] = CombinedBit;
        }
    }
    printf("\nSAMPLE CONVERSION ENDED\n");
}

void Sample::LargestPoint()
{
    for (int i = 0; i < SampleData.size(); i++)
    {
        if (SampleData[i] > HPoint)
        {
            HPoint = SampleData[i];
        }
        if (SampleData[i] < LPoint)
        {
            LPoint = SampleData[i];
        }
    }
}
