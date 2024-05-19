#include "Sample.h"

//Converts samples into BRR format
void Sample::BRRConvert()
{
    BRRFile::DataBlock BlankBlock = {0};
    printf("\n SAMPLE CONVERSION BEGIN\n");
    int BlockAmount = floor((SampleData.size()) / 16.0);

    printf("/n BLOCKS ALLOCATED:");
    printf(std::to_string(BlockAmount).data());
    signed char CombinedBit = 0;
    for (int i = 0; i < BlockAmount; i++)
    {
        int ShiftValue = 0;
        brr.DBlocks.push_back(BlankBlock);

        memset(BRR, 0, 9);
        signed int SampleInter[16];

        for (int z = 0; z < 16; z++)
        {
            SampleInter[z] = SampleData[16 * i + z];
        }

        ADPCMBlockMash(SampleInter, false, BlockAmount == i - 1);
        brr.DBlocks[i].HeaderByte = BRR[0];
        for (int x = 0; x < 8; x++)
        {
            //Combined bit is taking 2 4bit vales, shifts the second 4 bits to fit in the byte and not overwrite the first 4 bits
            int S0 = SampleData[16 * i + 2 * x + 0];
            int S1 = SampleData[16 * i + 2 * x + 1];
            S0 >>= ShiftValue;
            S1 >>= ShiftValue;
            CombinedBit = ((S0) & 0x0f) | ((S1 << 4) & 0xf0);
            brr.DBlocks[i].DataByte[x] = BRR[x+1];
        }
    }
    printf("\nSAMPLE CONVERSION ENDED\n");
}