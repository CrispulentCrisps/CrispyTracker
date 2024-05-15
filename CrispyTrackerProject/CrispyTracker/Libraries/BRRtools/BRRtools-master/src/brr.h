#ifndef BRR_H
#define BRR_H

#define SKIP_SAFE_MALLOC
#include "common.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
	// Global variables for prediction of filter
	extern pcm_t p1, p2;
	// Buffer for a single BRR data block (9 bytes)
	extern u8 BRR[9];

	typedef signed int _Sample;		// -rb and -g causes signed shorts to overflow and wrap around.

	void print_note_info(const unsigned int loopsize, const unsigned int samplerate);

	void print_loop_info(unsigned int loopcount, pcm_t oldp1[], pcm_t oldp2[]);

	//void generate_wave_file(FILE *outwav, unsigned int samplerate, pcm_t *buffer, size_t k);

	int get_brr_prediction(u8 filter, pcm_t p1, pcm_t p2);

	void decodeBRR(pcm_t* out);

	void apply_gauss_filter(pcm_t* buffer, size_t length);

	void ADPCMBlockMash(const _Sample PCM_data[16], bool is_loop_point, bool is_end_point);

	_Sample* resample(_Sample* samples, size_t samples_length, size_t out_length, char type);
#ifdef __cplusplus
}
#endif

#undef SKIP_SAFE_MALLOC
#endif
