/*
 * Copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 *
 * See .c file for documentation.
 */

#ifndef _AUDIO_EFFECT_MULTIBAND_COMPRESSOR_H
#define _AUDIO_EFFECT_MULTIBAND_COMPRESSOR_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <math.h>

#include "../audio_elements/audio_elements_common.h"

#include "../audio_elements/compressor.h"
#include "../audio_elements/biquad_filter.h"
#include "../audio_elements/audio_utilities.h"

// Result enumerations
typedef enum {
	MULTIBAND_COMP_OK,
	MULTIBAND_COMP_INVALID_INSTANCE_POINTER,
	MULTIBAND_COMP_INVALID_CROSSOVER_FREQ,
	MULTIBAND_COMP_INVALID_THRESHOLD,
	MULTIBAND_COMP_INVALID_GAIN

} RESULT_MULTIBAND_COMP;

// C struct with parameters and state information
typedef struct {

	bool initialized;

	float thresh_lpf;
	float thresh_hpf;
	float thresh_last;

	float gain_low;
	float gain_high;

	float gain_out;

	BIQUAD_FILTER lpf;
	BIQUAD_FILTER hpf;

	COMPRESSOR compressor_low;
	COMPRESSOR compressor_high;

	float lpf_filter_coeffs[6];
	float hpf_filter_coeffs[6];

} MULTIBAND_COMPRESSOR;

#if __cplusplus
extern "C" {
#endif

RESULT_MULTIBAND_COMP multiband_comp_setup(MULTIBAND_COMPRESSOR * c,
		float cross_over, float threshold, float audio_sample_rate);

RESULT_MULTIBAND_COMP multiband_comp_change_xover(MULTIBAND_COMPRESSOR * c,
		float crossover_freq_new);

RESULT_MULTIBAND_COMP multiband_comp_change_thresh(MULTIBAND_COMPRESSOR * c,
		float threshold_db_new);

RESULT_MULTIBAND_COMP multiband_comp_change_gain(MULTIBAND_COMPRESSOR * c,
		float gain_new);

void multiband_comp_read(MULTIBAND_COMPRESSOR * c, float * audio_in,
		float * audio_out, uint32_t audio_block_size);

#if __cplusplus
}
#endif

#endif  // _AUDIO_EFFECT_MULTIBAND_COMPRESSOR_H
