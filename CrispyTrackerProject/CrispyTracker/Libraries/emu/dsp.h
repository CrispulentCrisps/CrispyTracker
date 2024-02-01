#ifndef DSP_H
#define DSP_H

/* SNES SPC-700 DSP emulator C interface (also usable from C++) */
/* snes_spc 0.9.0 */

#include <stddef.h>

#ifdef __cplusplus
	extern "C" {
#endif

typedef void spc_dsp_t;

/* Creates new DSP emulator. NULL if out of memory. */
spc_dsp_t* spc_dsp_new( void );

/* Frees DSP emulator */
void spc_dsp_delete( spc_dsp_t* );

/* Initializes DSP and has it use the 64K RAM provided */
void spc_dsp_init( spc_dsp_t*, void* ram_64k );

/* Sets destination for output samples. If out is NULL or out_size is 0,
doesn't generate any. */
typedef short spc_dsp_sample_t;
void spc_dsp_set_output( spc_dsp_t*, spc_dsp_sample_t* out, int out_size );

/* Number of samples written to output since it was last set, always
a multiple of 2. Undefined if more samples were generated than
output buffer could hold. */
int spc_dsp_sample_count( spc_dsp_t const* );


/**** Emulation *****/

/* Resets DSP to power-on state */
void spc_dsp_reset( spc_dsp_t* );

/* Emulates pressing reset switch on SNES */
void spc_dsp_soft_reset( spc_dsp_t* );

/* Reads/writes DSP registers. For accuracy, you must first call spc_dsp_run() */
/* to catch the DSP up to present. */
int  spc_dsp_read ( spc_dsp_t const*, int addr );
void spc_dsp_write( spc_dsp_t*, int addr, int data );

/* Runs DSP for specified number of clocks (~1024000 per second). Every 32 clocks */
/* a pair of samples is be generated. */
void spc_dsp_run( spc_dsp_t*, int clock_count );


/**** Sound control *****/

/* Mutes voices corresponding to non-zero bits in mask. Reduces emulation accuracy. */
enum { spc_dsp_voice_count = 8 };
void spc_dsp_mute_voices( spc_dsp_t*, int mask );

/* If true, prevents channels and global volumes from being phase-negated.
Only supported by fast DSP; has no effect on accurate DSP. */
void spc_dsp_disable_surround( spc_dsp_t*, int disable );


/**** State save/load *****/

/* Resets DSP and uses supplied values to initialize registers */
enum { spc_dsp_register_count = 128 };
void spc_dsp_load( spc_dsp_t*, unsigned char const regs [spc_dsp_register_count] );

/* Saves/loads exact emulator state (accurate DSP only) */
enum { spc_dsp_state_size = 640 }; /* maximum space needed when saving */
typedef void (*spc_dsp_copy_func_t)( unsigned char** io, void* state, size_t );
void spc_dsp_copy_state( spc_dsp_t*, unsigned char** io, spc_dsp_copy_func_t );

/* Returns non-zero if new key-on events occurred since last call (accurate DSP only) */
int spc_dsp_check_kon( spc_dsp_t* );


#ifdef __cplusplus
	}
#endif

#endif
