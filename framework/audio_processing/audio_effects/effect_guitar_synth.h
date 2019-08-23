/*
 * Copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 *
 * See .c file for documentation.
 */

#ifndef _AUDIO_EFFECT_GUITAR_SYNTH_H
#define _AUDIO_EFFECT_GUITAR_SYNTH_H

#include "../audio_elements/audio_elements_common.h"

#include "../audio_elements/zero_crossing_detector.h"
#include "../audio_elements/simple_synth.h"
#include "../audio_elements/audio_utilities.h"

#include <stdint.h>
#include <stdbool.h>

// Result enumerations
typedef enum {
	GUITAR_SYNTH_OK,
	GUITAR_SYNTH_INVALID_INSTANCE_POINTER,
	GUITAR_SYNTH_INVALID_CLEAN_MIX,
	GUITAR_SYNTH_INVALID_SYNTH_MIX
} RESULT_GUITAR_SYNTH;

typedef struct {

	bool initialized;
	ZERO_CROSSING_DETECTOR zc_detect;

	BIQUAD_FILTER env_filter;
	float env_filter_coeffs[6];

	SIMPLE_SYNTH synth;
	SIMPLE_SYNTH synth_octave_low_1;
	SIMPLE_SYNTH synth_octave_low_2;

	float clean_mix;
	float synth_mix;
	float synth_volume;

	uint32_t synth_attack;
	uint32_t synth_decay;
	uint32_t synth_sustain;
	uint32_t synth_release;

	bool last_lock;
	bool current_lock;

	float detected_frequency;
	float measured_ampitude;

	float audio_sample_rate;
	uint32_t audio_block_size;

	uint32_t lock_cntr;

} GUITAR_SYNTH;

#if __cplusplus
extern "C" {
#endif

RESULT_GUITAR_SYNTH guitar_synth_setup(GUITAR_SYNTH * c, float clean_mix,
		float synth_mix, float audio_sample_rate);

RESULT_GUITAR_SYNTH guitar_synth_modify_clean_mix(GUITAR_SYNTH * c,
		float clean_mix_new);

RESULT_GUITAR_SYNTH guitar_synth_modify_synth_mix(GUITAR_SYNTH * c,
		float synth_mix_new);

void guitar_synth_read(GUITAR_SYNTH * c, float * audio_in, float * audio_out,
		uint32_t audio_block_size);

#if __cplusplus
}
#endif

#endif  // _AUDIO_EFFECT_GUITAR_SYNTH_H

