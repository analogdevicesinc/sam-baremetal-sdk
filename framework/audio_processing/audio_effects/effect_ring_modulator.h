/*
 * Copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 *
 * See .c file for documentation.
 */

#ifndef _AUDIO_EFFECT_RING_MODULATOR_H
#define _AUDIO_EFFECT_RING_MODULATOR_H

#include  <stdint.h>

#include "../audio_elements/biquad_filter.h"
#include "../audio_elements/audio_elements_common.h"

// Result enumerations
typedef enum {
	RING_MOD_OK,
	RING_MOD_INVALID_INSTANCE_POINTER,
	RING_MOD_INVALID_FREQ,
	RING_MOD_INVALID_DEPTH
} RESULT_RING_MOD;

// C struct with parameters and state information
typedef struct {

	bool initialized;

	float t;
	float t_inc;
	float depth;
	float audio_sample_rate;

} RING_MODULATOR;

// Wrapper allows C code to be called from C++ files
#if __cplusplus
extern "C" {
#endif

RESULT_RING_MOD ring_modulator_setup(RING_MODULATOR * c, float freq,
		float depth, float audio_sample_rate);

RESULT_RING_MOD ring_modulator_modify_freq(RING_MODULATOR * c, float freq_new);

RESULT_RING_MOD ring_modulator_modify_depth(RING_MODULATOR * c, float depth_new);

void ring_modulator_read(RING_MODULATOR * c, float * audio_in,
		float * audio_out, uint32_t audio_block_size);

// Wrapper allows C code to be called from C++ files
#ifdef __cplusplus
}
#endif

#endif  // _AUDIO_EFFECT_RING_MODULATOR_H
