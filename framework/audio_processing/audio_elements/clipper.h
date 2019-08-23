/*
 * Copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 *
 * See .c file for documentation.
 */

#ifndef _CLIPPER_H
#define _CLIPPER_H

#include <stdint.h>
#include <stdbool.h>

#include "audio_elements_common.h"

#define CLIPPER_INTERP_TAPS         (33)

// Result enumerations
typedef enum {
	CLIPPER_OK, CLIPPER_INVALID_INSTANCE_POINTER, CLIPPER_INVALID_THRESHOLD
} RESULT_CLIPPER;

// Various polynomials used for clipping
typedef enum {
	POLY_SMOOTHSTEP, POLY_SMOOTHERSTEP
} POLY_CLIP_FUNC;

// Instance struct with parameters and state information
typedef struct {

	bool initialized;

	float fir_upsample_state[CLIPPER_INTERP_TAPS + 1];
	float fir_downsample_state[CLIPPER_INTERP_TAPS + 1];
	POLY_CLIP_FUNC poly_clip;
	float clip_threshold;
	bool upsample;
} CLIPPER;

#ifdef __cplusplus
extern "C" {
#endif

RESULT_CLIPPER clipper_setup(CLIPPER *c, float threshold,
		POLY_CLIP_FUNC poly_clip, bool upsample);

RESULT_CLIPPER modify_clipper_threshold(CLIPPER *c, float threshold);

void clipper_read(CLIPPER *c, float * audio_in, float * audio_out,
		uint32_t audio_block_size);

#ifdef __cplusplus
}
#endif

#endif  // _CLIPPER_H
