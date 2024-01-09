#include "Sample.h"

void Sample::BRRConvert()
{
    printf("\n SAMPLE CONVERSION BEGIN\n");
    int BlockAmount = (SampleData.size()) / 16;

    printf("\n BLOCKS ALLOCATED:");
    printf(std::to_string(BlockAmount).data());
    signed char FourBitValues[4];
    signed char CombinedBit = 0;
    for (int i = 0; i < BlockAmount; i++)
    {
        brr.DBlocks.push_back(brr.BlankBlock);

        for (int x = 0; x < 8; x++)
        {
            CombinedBit = ((SampleData[16 * i + 2 * x + 0] >> 12) & 0x0f) | ((SampleData[16 * i + 2 * x + 1] >> 8) & 0xf0);
            brr.DBlocks[i].DataByte[x] = CombinedBit;
        }
    }
    printf("\nSAMPLE CONVERSION ENDED\n");
}
