/*
 * Copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 *
 * See .c file for documentation.
 */

#ifndef _ZERO_CROSSING_DETECTOR_H_
#define _ZERO_CROSSING_DETECTOR_H_

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#include "audio_elements_common.h"
#include "biquad_filter.h"

// Effect definitions
#define FREQ_HIST_LEN           (3)
#define LOWPASS_FIR_TAPS        (31)
#define ZC_DEFAULT_THRESHOLD    (0.02)
#define ZC_FREQ_LOCK_CNTR       (150)

// Result enumerations
typedef enum {
	ZERO_CROSS_OK,
	ZERO_CROSS_INVALID_INSTANCE_POINTER,
	ZERO_CROSS_INVALID_THRESHOLD
} RESULT_ZERO_CROSSING;

// C struct with parameters and state information
typedef struct {

	bool initialized;

	BIQUAD_FILTER lpf;
	float lpf_coeffs[6];

	BIQUAD_FILTER zero_block;
	float zb_coeffs[6];

	float dc_last_y;
	float dc_coeff;

	float peak_amplitude_pos;
	float peak_amplitude_neg;
	float threshold;

	uint32_t period_counter;
	uint32_t wave_state;

	bool freq_lock;
	uint32_t freq_lock_cntr;
	float mean_freq;

	float freq_history[FREQ_HIST_LEN];
	uint32_t freq_ptr;

	float varf_val;

	float audio_sample_rate;

} ZERO_CROSSING_DETECTOR;

// Wrapper allows C code to be called from C++ files
#ifdef __cplusplus
extern "C" {
#endif

RESULT_ZERO_CROSSING zero_cross_setup(ZERO_CROSSING_DETECTOR * c,
		float threshold, float audio_sample_rate);

bool zero_crossing_read(ZERO_CROSSING_DETECTOR * c, float * audio_in,
		uint32_t audio_block_size, float * detected_frequency);

// Wrapper allows C code to be called from C++ files
#if __cplusplus
}
#endif

#endif  // _ZERO_CROSSING_DETECTOR_H_
