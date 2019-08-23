/*
 * Copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 *
 * See .c file for documentation.
 */

#ifndef _AUDIO_EFFECT_REVERB_H
#define _AUDIO_EFFECT_REVERB_H

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>

#include "../audio_elements/audio_elements_common.h"

#include "../audio_elements/integer_delay_lpf.h"
#include "../audio_elements/allpass_filter.h"
#include "../audio_elements/audio_utilities.h"

#define REVERB_MAX_DELAY_SIZE   1700
#define REVERB_MAX_ALLPASS_SIZE 556

#define REVERB_ALLPASS_ELEMENTS (4)
#define REVERB_DELAY_ELEMENTS   (8)

// Result enumerations
typedef enum {
	REVERB_OK,
	REVERB_INVALID_INSTANCE_POINTER,
	REVERB_INVALID_WET_MIX,
	REVERB_INVALID_DRY_MIX,
	REVERB_INVALID_FEEDBACK,
	REVERB_INVALID_LP_DAMP
} RESULT_STEREO_REVERB;

// C struct with parameters and state information
typedef struct {

	bool initialized;

	float feedback;
	float lp_damp;
	float wet_mix;
	float dry_mix;

	ALLPASS_FILTER allpass_outputs_left[REVERB_ALLPASS_ELEMENTS];
	ALLPASS_FILTER allpass_outputs_right[REVERB_ALLPASS_ELEMENTS];
	float allpass_buffers_left[REVERB_ALLPASS_ELEMENTS][REVERB_MAX_ALLPASS_SIZE];
	float allpass_buffers_right[REVERB_ALLPASS_ELEMENTS][REVERB_MAX_ALLPASS_SIZE];

	DELAY_LPF lpcf_left[REVERB_DELAY_ELEMENTS];
	DELAY_LPF lpcf_right[REVERB_DELAY_ELEMENTS];
	float delay_buffers_left[REVERB_DELAY_ELEMENTS][REVERB_MAX_DELAY_SIZE];
	float delay_buffers_right[REVERB_DELAY_ELEMENTS][REVERB_MAX_DELAY_SIZE];

} STEREO_REVERB;

// Wrapper allows C code to be called from C++ files
#if __cplusplus
extern "C" {
#endif

RESULT_STEREO_REVERB reverb_setup(STEREO_REVERB * c, float wet_mix,
		float dry_mix, float feedback, float lp_damp);

RESULT_STEREO_REVERB reverb_change_wet_mix(STEREO_REVERB * c, float wet_mix_new);

RESULT_STEREO_REVERB reverb_change_dry_mix(STEREO_REVERB * c, float dry_mix_new);

RESULT_STEREO_REVERB reverb_change_feedback(STEREO_REVERB * c,
		float feedback_new);

RESULT_STEREO_REVERB reverb_change_lp_damp_coeff(STEREO_REVERB * c,
		float lp_damp_new);

void reverb_read(STEREO_REVERB * c, float * audio_in, float * audio_out_left,
		float * audio_out_right, uint32_t audio_block_size);

// Wrapper allows C code to be called from C++ files
#if __cplusplus
}
#endif

#endif // _AUDIO_EFFECT_REVERB_H
