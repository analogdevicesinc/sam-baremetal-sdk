/*
 * Copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 *
 * See .c file for documentation.
 */

#ifndef _VARIABLE_DELAY_H
#define _VARIABLE_DELAY_H

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include "audio_elements_common.h"

#define VARIABLE_DELAY_MAX_DEPTH        (1024)
#define VARIABLE_DELAY_PRE_DELAY        (100)
// Result enumerations
typedef enum {
	VARIABLE_DELAY_OK,
	VARIABLE_DELAY_INVALID_INSTANCE_POINTER,
	VARIABLE_DELAY_INVALID_FEEDBACK,
	VARIABLE_DELAY_INVALID_DEPTH,
	VARIABLE_DELAY_INVALID_RATE,

} RESULT_VARIABLE_DELAY;

typedef enum {
	VARIABLE_DELAY_SIN,
	VARIABLE_DELAY_TRI,
	VARIABLE_DELAY_SQR,
	VARIABLE_DELAY_EXT_LFO
} VARIABLE_DELAY_TYPE;

// C struct with parameters and state information
typedef struct {

	bool initialized;

	float feedback;
	float mod_depth;
	float mod_rate_hz;
	VARIABLE_DELAY_TYPE mod_type;

	float audio_sample_rate;

	float feedback_lastsamp;

	float delay_buffer[VARIABLE_DELAY_MAX_DEPTH + VARIABLE_DELAY_PRE_DELAY];
	int delay_index;

	float t;
	float inc;

} VARIABLE_DELAY;

// Wrapper allows C code to be called from C++ files
#if __cplusplus
extern "C" {
#endif

RESULT_VARIABLE_DELAY variable_delay_setup(VARIABLE_DELAY * c, float depth,
		float feedback, float rate_hz, float audio_sample_rate,
		VARIABLE_DELAY_TYPE type);

RESULT_VARIABLE_DELAY variable_delay_modify_feedback(VARIABLE_DELAY * c,
		float new_feedback);
RESULT_VARIABLE_DELAY variable_delay_modify_depth(VARIABLE_DELAY * c,
		float new_depth);
RESULT_VARIABLE_DELAY variable_delay_modify_rate(VARIABLE_DELAY * c,
		float new_rate);

void variable_delay_read(VARIABLE_DELAY * c, float * audio_in,
		float * audio_out, float * ext_mod, uint32_t audio_block_size);

// Wrapper allows C code to be called from C++ files
#if __cplusplus
}
#endif

#endif  // _VARIABLE_DELAY_H
