/*
 * Copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 *
 * See .c file for documentation.
 */
#ifndef _AUDIO_UTILITIES_H
#define _AUDIO_UTILITIES_H

#include <stdint.h>

// Wrapper allows C code to be called from C++ files
#if __cplusplus
extern "C" {
#endif

// Simple 1-pole filter functions
float filter_1pole(float last_y, float x, float coeff_fc);
float gen_1pole_coeff(float fc, float audio_sample_rate);
void filter_1pole_lpf_block(float * input, float * output, float * last_y,
		float coeff_fc, uint32_t audio_block_size);

// Basic audio buffer utilities
void clear_buffer(float * buffer, uint32_t audio_block_size);
void copy_buffer(float * input, float * output, uint32_t audio_block_size);
void gain_buffer(float * buffer, float gain, uint32_t audio_block_size);

// Mixing functions
void mix_2x1(float * input1, float * input2, float * output,
		uint32_t audio_block_size);

void mix_2x1_gain(float * input1, float gain1, float * input2, float gain2,
		float * output, uint32_t audio_block_size);

// Amplitude measurement functions
void measure_amp_peak(float input, float * amplitude, float decay);

// Wrapper allows C code to be called from C++ files
#if __cplusplus
}
#endif

#endif  // _AUDIO_UTILITIES_H
