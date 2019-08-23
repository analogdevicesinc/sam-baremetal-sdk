/*
 * Copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 *
 * See .c file for documentation.
 */

#ifndef _CLICKLESS_VOLUME_CTRL_H
#define _CLICKLESS_VOLUME_CTRL_H

#include <stdint.h>
#include <stdbool.h>

#include "audio_elements_common.h"

// Result enumerations
typedef enum {
	VOLUME_OK, VOLUME_INVALID_GAIN, VOLUME_INVALID_INSTANCE_POINTER
} RESULT_VOLUME_CTRL;

typedef enum {
	VOLUME_TRANSITION_VERY_SLOW = 30000,
	VOLUME_TRANSITION_SLOW = 15000,
	VOLUME_TRANSITION_MEDIUM = 5000,
	VOLUME_TRANSITION_FAST = 2500,
	VOLUME_TRANSITION_VERY_FAST = 500
} VOLUME_CTRL_TRANSITION_SPEED;

// Instance struct with parameters and state information
typedef struct {

	bool initialized;

	float gain_last;
	float target_gain;
	float current_gain;
	float gain_transition_rate;   // speed at which we transition to new gain
	float delta; //
	uint32_t remaining_steps;
} VOLUME_CTRL;

// Wrapper allows C code to be called from C++ files
#if __cplusplus
extern "C" {
#endif
RESULT_VOLUME_CTRL volume_control_setup(VOLUME_CTRL * C, float initial_gain);

RESULT_VOLUME_CTRL volume_control_set_gain(VOLUME_CTRL * C, float new_gain,
		VOLUME_CTRL_TRANSITION_SPEED speed);

void volume_control_read(VOLUME_CTRL * c, float * audio_in, float * audio_out,
		uint32_t audio_block_size);

// Wrapper allows C code to be called from C++ files
#if __cplusplus
}
#endif

#endif //_CLICKLESS_VOLUME_CTRL_H
