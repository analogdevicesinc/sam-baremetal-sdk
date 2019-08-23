/*
 * Copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 *
 * See .c file for documentation.
 */

#ifndef _BIQUAD_FILTER_H
#define _BIQUAD_FILTER_H

#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "audio_elements_common.h"

// Types of biquad filters
typedef enum {
	BIQUAD_TYPE_LPF,
	BIQUAD_TYPE_HPF,
	BIQUAD_TYPE_BPF,
	BIQUAD_TYPE_NOTCH,
	BIQUAD_TYPE_PEAKING,
	BIQUAD_TYPE_L_SHELF,
	BIQUAD_TYPE_H_SHELF
} BIQUAD_FILTER_TYPE;

// Transition speed while changing filter parameters
typedef enum {
	BIQUAD_TRANS_VERY_FAST = (2),
	BIQUAD_TRANS_FAST = (5),
	BIQUAD_TRANS_MED = (10),
	BIQUAD_TRANS_SLOW = (20),
	BIQUAD_TRANS_VERY_SLOW = (30)
} BIQUAD_FILTER_TRANSITION_SPEED;

// Result enumerations
typedef enum {
	BIQUAD_OK,
	BIQUAD_INVALID_INSTANCE_POINTER,
	BIQUAD_INVALID_Q,
	BIQUAD_INVALID_FREQ,
	BIQUAD_INVALID_GAIN
} RESULT_BIQUAD;

// Instance struct with parameters and state information
typedef struct {

	bool initialized;

	BIQUAD_FILTER_TYPE filter_type;
	BIQUAD_FILTER_TRANSITION_SPEED transition_speed;

	float audio_sample_rate;

	float freq;
	float freq_last;
	float freq_dest;
	float freq_inc;
	uint32_t freq_steps;

	float q;
	float q_last;
	float q_dest;
	float q_inc;
	uint32_t q_steps;

	float gain_db;

	float scaling_factor;
	float scaling_factor_dest;
	float scaling_factor_inc;

	float pm * sos_coeffs;
	float sos_state[3];
	float sos_coeffs_dest[4];
	float sos_coeffs_inc[4];
	uint32_t sos_coeffs_steps;

} BIQUAD_FILTER;

#ifdef __cplusplus
extern "C" {
#endif

RESULT_BIQUAD filter_setup(BIQUAD_FILTER * c, BIQUAD_FILTER_TYPE type,
		BIQUAD_FILTER_TRANSITION_SPEED transition_speed, float pm * sos_coeffs,
		float freq, float q, float gain_db, float audio_sample_rate);

RESULT_BIQUAD filter_modify_q(BIQUAD_FILTER * c, float new_q);

RESULT_BIQUAD filter_modify_freq(BIQUAD_FILTER * c, float new_freq);

void filter_read(BIQUAD_FILTER * c, float * audio_in, float * audio_out,
		uint32_t audio_block_size);

#ifdef __cplusplus
}
#endif

#endif // _BIQUAD_FILTER_H
