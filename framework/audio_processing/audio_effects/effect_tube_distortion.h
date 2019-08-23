/*
 * Copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 *
 * See .c file for documentation.
 */

#ifndef _AUDIO_EFFECT_TUBE_DISTORTION_H
#define _AUDIO_EFFECT_TUBE_DISTORTION_H
#include <stdlib.h>

#include "../audio_elements/clipper.h"
#include "../audio_elements/biquad_filter.h"
#include "../audio_elements/audio_elements_common.h"

// Result enumerations
typedef enum {
	TUBE_DISTORTION_OK,
	TUBE_DISTORTION_INVALID_INSTANCE_POINTER,
	TUBE_DISTORTION_INVALID_CONTOUR,
	TUBE_DISTORTION_INVALID_DRIVE,
	TUBE_DISTORTION_INVALID_THRESHOLD,
	TUBE_DISTORTION_INVALID_GAIN
} RESULT_TUBE_DISTORTION;

typedef struct {
	bool initialized;

	CLIPPER clipper;

	BIQUAD_FILTER input_filter;
	BIQUAD_FILTER output_filter;
	float input_filter_coeffs[6];
	float output_filter_coeffs[6];

	float gain;
	float drive;
	float threshold;
} TUBE_DISTORTION;

// Wrapper allows C code to be called from C++ files
#if __cplusplus
extern "C" {
#endif

RESULT_TUBE_DISTORTION tube_distortion_setup(TUBE_DISTORTION * c, float drive,
		float gain, float contour, float audio_sample_rate);

void tube_distortion_read(TUBE_DISTORTION * c, float * audio_in,
		float * audio_out, uint32_t audio_block_size);

RESULT_TUBE_DISTORTION tube_distortion_modify_gain(TUBE_DISTORTION * c,
		float gain);

RESULT_TUBE_DISTORTION tube_distortion_modify_drive(TUBE_DISTORTION * c,
		float drive);
RESULT_TUBE_DISTORTION tube_distortion_modify_threshold(TUBE_DISTORTION * c,
		float threshold);

RESULT_TUBE_DISTORTION tube_distortion_modify_contour(TUBE_DISTORTION * c,
		float contour);

// Wrapper allows C code to be called from C++ files
#if __cplusplus
}
#endif

#endif  // _AUDIO_EFFECT_TUBE_DISTORTION_H
