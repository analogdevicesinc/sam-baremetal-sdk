/*
 * Copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 *
 * See .c file for documentation.
 */

#ifndef _AUDIO_EFFECT_AUTOWAH_H
#define _AUDIO_EFFECT_AUTOWAH_H

#include  <stdint.h>

#include "../audio_elements/biquad_filter.h"
#include "../audio_elements/audio_elements_common.h"

// Result enumerations
typedef enum {
	AUTOWAH_OK,
	AUTOWAH_INVALID_INSTANCE_POINTER,
	AUTOWAH_INVALID_DEPTH,
	AUTOWAH_INVALID_DECAY,
	AUTOWAH_INVALID_Q
} RESULT_AUTOWAH;

// Instance struct with parameters and state information
typedef struct {

	bool initialized;
	BIQUAD_FILTER bpf1, bpf2, bpf3;
	float bpf_coeffs1[6], bpf_coeffs2[6], bpf_coeffs3[6];
	float measured_ampitude;
	float freq_start;
	float depth;
	float decay;
	float q;
	float q_last;

} AUTOWAH;

// Wrapper allows C code to be called from C++ files
#if __cplusplus
extern "C" {
#endif

RESULT_AUTOWAH autowah_setup(AUTOWAH * c, float depth, float decay,
		float audio_sample_rate);

RESULT_AUTOWAH autowah_modify_decay(AUTOWAH * c, float decay_new);

RESULT_AUTOWAH autowah_modify_depth(AUTOWAH * c, float depth_new);

RESULT_AUTOWAH autowah_modify_q(AUTOWAH * c, float q_new);

void autowah_read(AUTOWAH * c, float * audio_in, float * audio_out,
		uint32_t audio_block_size);

// Wrapper allows C code to be called from C++ files
#ifdef __cplusplus
}
#endif

#endif  // _AUDIO_EFFECT_AUTOWAH_H
