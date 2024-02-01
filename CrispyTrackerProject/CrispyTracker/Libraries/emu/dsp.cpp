// snes_spc 0.9.0. http://www.slack.net/~ant/

#include "dsp.h"

#include "SPC_DSP.h"

/* Copyright (C) 2007 Shay Green. This module is free software; you
can redistribute it and/or modify it under the terms of the GNU Lesser
General Public License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version. This
module is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
details. You should have received a copy of the GNU Lesser General Public
License along with this module; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA */

#include "blargg_source.h"

spc_dsp_t* spc_dsp_new( void )
{
	// be sure constants match
	assert( spc_dsp_voice_count     == (int) SPC_DSP::voice_count );
	assert( spc_dsp_register_count  == (int) SPC_DSP::register_count );
	#if !SPC_NO_COPY_STATE_FUNCS
	assert( spc_dsp_state_size      == (int) SPC_DSP::state_size );
	#endif
	
	return new SPC_DSP;
}

void spc_dsp_delete      ( spc_dsp_t* s )                                 { delete reinterpret_cast<SPC_DSP*>(s); }
void spc_dsp_init        ( spc_dsp_t* s, void* ram_64k )                  { reinterpret_cast<SPC_DSP*>(s)->init( ram_64k ); }
void spc_dsp_set_output  ( spc_dsp_t* s, spc_dsp_sample_t* p, int n )     { reinterpret_cast<SPC_DSP*>(s)->set_output( p, n ); }
int  spc_dsp_sample_count( spc_dsp_t const* s )                           { return reinterpret_cast<const SPC_DSP*>(s)->sample_count(); }
void spc_dsp_reset       ( spc_dsp_t* s )                                 { reinterpret_cast<SPC_DSP*>(s)->reset(); }
void spc_dsp_soft_reset  ( spc_dsp_t* s )                                 { reinterpret_cast<SPC_DSP*>(s)->soft_reset(); }
int  spc_dsp_read        ( spc_dsp_t const* s, int addr )                 { return reinterpret_cast<const SPC_DSP*>(s)->read( addr ); }
void spc_dsp_write       ( spc_dsp_t* s, int addr, int data )             { reinterpret_cast<SPC_DSP*>(s)->write( addr, data ); }
void spc_dsp_run         ( spc_dsp_t* s, int clock_count )                { reinterpret_cast<SPC_DSP*>(s)->run( clock_count ); }
void spc_dsp_mute_voices ( spc_dsp_t* s, int mask )                       { reinterpret_cast<SPC_DSP*>(s)->mute_voices( mask ); }
void spc_dsp_disable_surround( spc_dsp_t* s, int disable )                { reinterpret_cast<SPC_DSP*>(s)->disable_surround( disable ); }
void spc_dsp_load        ( spc_dsp_t* s, unsigned char const regs [spc_dsp_register_count] ) { reinterpret_cast<SPC_DSP*>(s)->load( regs ); }

#if !SPC_NO_COPY_STATE_FUNCS
void spc_dsp_copy_state  ( spc_dsp_t* s, unsigned char** p, spc_dsp_copy_func_t f ) { reinterpret_cast<SPC_DSP*>(s)->copy_state( p, f ); }
int  spc_dsp_check_kon   ( spc_dsp_t* s )                                 { return reinterpret_cast<SPC_DSP*>(s)->check_kon(); }
#endif
