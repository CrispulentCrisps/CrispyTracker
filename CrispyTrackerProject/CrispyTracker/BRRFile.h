#pragma once
#include <vector>
//https://www.youtube.com/watch?v=bgh5_gxT2eg BRR format source
//https://snes.nesdev.org/wiki/BRR_samples

using namespace std;

class BRRFile
{
public:
	unsigned short SampleDir = 0; //Defines space in memory

	struct DataBlock{
		//0000 - Shift Values [quantisation step]
		//00 Differential filter mode
		//0 Loop flag
		//0 End flag
		unsigned char HeaderByte = 0;
		
		//Holds the BRR data per sample block
		signed char DataByte[8] = { 0,0,0,0,0,0,0,0 };
	};

	vector<DataBlock> DBlocks;
};