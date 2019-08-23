/*
 * Copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 *
 * See .c file for documentation.
 */

#ifndef _AUDIO_EFFECT_TREMELO_H
#define _AUDIO_EFFECT_TREMELO_H

#include "../audio_elements/amplitude_modulation.h"

// Result enumerations
typedef enum {
	TREMELO_OK,
	TREMELO_INVALID_INSTANCE_POINTER,
	TREMELO_INVALID_RATE,
	TREMELO_INVALID_DEPTH
} RESULT_TREMELO;

typedef struct {

	bool initialized;

	AMPLITUDE_MODULATION modulator;

	float depth;
	float rate_hz;

	float lfo_t;
	float lfo_t_inc;
	float audio_sample_rate;

} TREMELO;

// Wrapper allows C code to be called from C++ files
#if __cplusplus
extern "C" {
#endif

RESULT_TREMELO tremelo_setup(TREMELO * c, float depth, float rate_hz,
		float audio_sample_rate);

RESULT_TREMELO tremelo_modify_rate(TREMELO * c, float new_rate_hz);
RESULT_TREMELO tremelo_modify_depth(TREMELO * c, float new_depth);

void tremelo_read(TREMELO * c, float * audio_in, float * audio_out,
		uint32_t audio_block_size);

// Wrapper allows C code to be called from C++ files
#if __cplusplus
}
#endif

#endif  // _AUDIO_EFFECT_TREMELO_H
