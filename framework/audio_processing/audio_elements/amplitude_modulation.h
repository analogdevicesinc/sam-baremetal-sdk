/*
 * Copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 *
 * See .c file for documentation.
 */

#ifndef _AMPLITUDE_MODULATION_H
#define _AMPLITUDE_MODULATION_H

#include <stdint.h>
#include "audio_elements_common.h"

// Result enumerations
typedef enum {
	AMPLITUDE_MOD_OK,
	AMPLITUDE_MOD_INVALID_INSTANCE_POINTER,
	AMPLITUDE_MOD_INVALID_RATE,
	AMPLITUDE_MOD_INVALID_DEPTH,
	AMPLITUDE_MOD_PARAMETER_NOT_CHANGED
} RESULT_AMPLITUDE_MOD;

// Supported LFO types
typedef enum {
	AMP_MOD_SIN, AMP_MOD_TRI, AMP_MOD_SQR, AMP_MOD_RAMP, AMP_MOD_EXT_LFO
} AMPLITUDE_MOD_TYPE;

// Instance struct with parameters and state information
typedef struct {

	bool initialized;
	AMPLITUDE_MOD_TYPE type;
	float mod_rate_hz;
	float mod_depth;

	float audio_sample_rate;
	float t;
	float inc;
} AMPLITUDE_MODULATION;

// Wrapper allows C code to be called from C++ files
#ifdef __cplusplus
extern "C" {
#endif

RESULT_AMPLITUDE_MOD amplitude_modulation_setup(AMPLITUDE_MODULATION * c,
		float depth, float rate_hz, AMPLITUDE_MOD_TYPE type,
		float audio_sample_rate);

RESULT_AMPLITUDE_MOD amplitude_modulation_modify_depth(AMPLITUDE_MODULATION * c,
		float new_depth);

RESULT_AMPLITUDE_MOD amplitude_modulation_modify_rate(AMPLITUDE_MODULATION * c,
		float new_rate_hz);

void amplitude_modulation_read(AMPLITUDE_MODULATION * c, float * audio_in,
		float * audio_out, float * ext_mod, uint32_t audio_block_size);

// Wrapper allows C code to be called from C++ files
#ifdef __cplusplus
}
#endif

#endif  // _AMPLITUDE_MODULATION_H
