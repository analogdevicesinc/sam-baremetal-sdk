/*
 * Copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 *
 * See .c file for documentation.
 */

#ifndef _AUDIO_EFFECT_FLANGER_H
#define _AUDIO_EFFECT_FLANGER_H

#include "../audio_elements/variable_delay.h"
#include "../audio_elements/oscillators.h"

#include <stdint.h>
#include <stdbool.h>

// Result enumerations
typedef enum {
	FLANGER_OK,
	FLANGER_INVALID_INSTANCE_POINTER,
	FLANGER_INVALID_RATE,
	FLANGER_INVALID_DEPTH,
	FLANGER_INVALID_FEEDBACK
} RESULT_FLANGER;

// C struct with parameters and state information
typedef struct {

	bool initialized;

	VARIABLE_DELAY var_del_left;
	VARIABLE_DELAY var_del_right;
	float depth;
	float rate_hz;
	float feedback;

	float lfo_t_left;
	float lfo_t_right;
	float inc;
	float audio_sample_rate;

} STEREO_FLANGER;

// Wrapper allows C code to be called from C++ files
#if __cplusplus
extern "C" {
#endif

RESULT_FLANGER flanger_setup(STEREO_FLANGER * c, float depth, float rate_hz,
		float feedback, float audio_sample_rate);

RESULT_FLANGER flanger_modify_rate(STEREO_FLANGER * c, float new_rate_hz);

RESULT_FLANGER flanger_modify_depth(STEREO_FLANGER * c, float new_depth);

RESULT_FLANGER flanger_modify_feedback(STEREO_FLANGER * c, float new_feedback);

void flanger_read(STEREO_FLANGER * c, float * audio_in, float * audio_out_left,
		float * audio_out_right, uint32_t audio_block_size);

// Wrapper allows C code to be called from C++ files
#if __cplusplus
}
#endif

#endif  // _AUDIO_EFFECT_FLANGER_H

