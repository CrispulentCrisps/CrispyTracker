#pragma once

//Macros here define the reserved positions in the tracker commands
#define STOP_COMMAND 256		//Fully stops the audio, most likely done with KOF and setting the volume to 0
#define RELEASE_COMMAND 257		//Puts the instrument in the release state
#define NULL_COMMAND 258		//Empty =section in rows, denoted with .. or ...