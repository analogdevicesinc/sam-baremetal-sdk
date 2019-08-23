/*
 * Copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 *
 * See .c file for documentation.
 */

#ifndef _INTEGER_DELAY_LPF_H
#define _INTEGER_DELAY_LPF_H

#include <stdint.h>
#include <stdbool.h>

#include "audio_elements_common.h"

// Result enumerations
typedef enum {
	DELAY_OK,
	DELAY_INVALID_INSTANCE_POINTER,
	DELAY_INVALID_DELAY_LINE_POINTER,
	DELAY_LENGTH_EXCEEDS_BUF_SIZE,
	DELAY_INVALID_FEEDBACK,
	DELAY_INVALID_FEEDTHROUGH,
	DELAY_INVALID_DAMPENING_COEFF
} RESULT_DELAY;

// C struct with parameters and state information
typedef struct {

	bool initialized;

	float * delay_line;
	uint32_t delay_line_size;
	uint32_t write_ptr;
	int32_t read_tap;
	float read_tap_f;
	int32_t target_read_tap;
	float read_tap_inc;
	uint32_t read_tap_steps;

	float feedback;
	float feedthrough;
	float lpf_a;
	float lpf_hist;
} DELAY_LPF;

#if __cplusplus
extern "C" {
#endif

RESULT_DELAY delay_setup(DELAY_LPF * c, float * delay_buffer,
		uint32_t delay_buffer_size, uint32_t delay_initial_length,
		float feedback, float feedthrough, float a_coeff);

RESULT_DELAY delay_modify_dampening(DELAY_LPF * c, float coeff);
RESULT_DELAY delay_modify_length(DELAY_LPF * c, uint32_t new_delay_length);
RESULT_DELAY delay_modify_feedback(DELAY_LPF * c, float new_feedback);
RESULT_DELAY delay_modify_feedthrough(DELAY_LPF * c, float new_feedthrough);

void delay_read(DELAY_LPF * c, float * input, float * output,
		uint32_t audio_block_size);

#if __cplusplus
}
#endif

#endif  // _INTEGER_DELAY_LPF_H
