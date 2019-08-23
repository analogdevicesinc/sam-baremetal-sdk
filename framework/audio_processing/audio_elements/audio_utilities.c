/*
 * Copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 *
 * This file includes various misc audio processing functions used in audio
 * processing.
 */
#include <math.h>
#include "audio_elements_common.h"
#include "audio_utilities.h"

/**
 * @brief Implements a simple 1-pole low-pass filter
 *
 * A smaller coefficient value results in a longer time constant and a lower
 * cutoff frequency.  A higher coefficient value (e.g. 0.5) results in a
 * shorter time constant and a higher cutoff frequency.
 *
 * Here's a nice utility for calculating the coefficient: https://fiiir.com/
 * Not that this takes the 'decay' coefficient as an input.  The 'coeff'
 * input of this function is 1-decay.
 *
 * @param y Last output
 * @param x Current sample
 * @param coeff Filter coefficient
 * @return Current output
 */
float gen_1pole_coeff(float fc, float audio_sample_rate) {
	if (fc > audio_sample_rate * 0.5) {
		fc = audio_sample_rate;
	} else if (fc < 0) {
		fc = 0;
	}
	float val = 1.0 - expf(-PI2 * fc / audio_sample_rate);
	return val;
}

/**
 * @brief Implements a sample-based 1-pole LPF
 *
 * @param last_y Last output value
 * @param x Input value
 * @param coeff_fc Filter coefficient
 * @return Filtered output sample
 */
#pragma optimize_for_speed
float filter_1pole(float last_y, float x, float coeff_fc) {
	return coeff_fc * (x - last_y);
}

/**
 * @brief Implement block-based 1-pole LPF
 *
 * @param input Pointer to source floating point buffer to be filtered
 * @param output Pointer to output buffer to write filtered data to
 * @param last_y Pointer to the last output value (state value of filter)
 * @param coeff_fc Filter coefficient
 * @param audio_block_size The number of floating-point words to process in the buffer
 */
#pragma optimize_for_speed
void filter_1pole_lpf_block(float * input, float * output, float * last_y,
		float coeff_fc, uint32_t audio_block_size) {
	for (int i = 0; i < audio_block_size; i++) {
		*last_y += coeff_fc * (input[i] - *last_y);
		output[i] = *last_y;
	}
}

/**
 * @brief Converts a linear amplitude value to decibles
 *
 * @param linear_val Current value to be converted
 * @return Output value in dB
 */
inline float linear_to_db(float linear_val) {
	return 20.0 * log10f(linear_val);
}

/**
 * @brief Calculates the running RMS value for a stream of samples
 *
 * @param input Current sample
 * @param last_measurement Last RMS measurement
 * @param coeff_fc Low-pass filter coefficent
 * @return RMS value
 */
#pragma optimize_for_speed
float measure_amp_rms(float input, float last_measurement, float coeff_fc) {
	input = input * input;
	return sqrtf(filter_1pole(input, last_measurement, coeff_fc));
}

/**
 * @brief A basic peak follower
 *
 * @param input A single floating point input value (mono)
 * @param amplitude A pointer to the current amplitude value
 * @param decay Rate of decay.  A value closer to 1.0 will result in a slower decay
 */
#pragma optimize_for_speed
void measure_amp_peak(float input, float * amplitude, float decay) {
	input = fabs(input);
	if (input > *amplitude) {
		*amplitude = input;
	} else {
		*amplitude *= decay;
	}
}

/**
 * @brief Clears a floating-point audio buffer
 *
 * @param buffer Pointer to floating point buffer to clear (mono)
 * @param audio_block_size The number of floating-point words to process in the buffer
 */
#pragma optimize_for_speed
void clear_buffer(float * buffer, uint32_t audio_block_size) {
	for (int i = 0; i < audio_block_size; i++) {
		buffer[i] = 0.0;
	}
}

/**
 * @brief Copy a floating-point audio buffer
 *
 * @param input Pointer to source floating point buffer
 * @param output Pointer to destination floating point buffer
 * @param audio_block_size The number of floating-point words to process in the buffer
 */
#pragma optimize_for_speed
void copy_buffer(float * input, float * output, uint32_t audio_block_size) {
	for (int i = 0; i < audio_block_size; i++) {
		output[i] = input[i];
	}
}

/**
 * @brief Scale a floating-point buffer by a gain value
 *
 * @param buffer Pointer to floating point buffer to scale (mono)
 * @param gain Gain value
 * @param audio_block_size The number of floating-point words to process in the buffer
 */
#pragma optimize_for_speed
void gain_buffer(float * buffer, float gain, uint32_t audio_block_size) {
	for (int i = 0; i < audio_block_size; i++) {
		buffer[i] *= gain;
	}
}

/**
 * @brief Adds two channels together
 *  *
 * @param input1 Pointer to first floating point buffer to mix (mono)
 * @param input2 Pointer to second floating point buffer to mix (mono)
 * @param output Mixed output buffer
 * @param audio_block_size The number of floating-point words to process in the buffer
 */
#pragma optimize_for_speed
void mix_2x1(float * input1, float * input2, float * output,
		uint32_t audio_block_size) {
	for (int i = 0; i < audio_block_size; i++) {
		output[i] = input1[i] + input2[i];
	}
}

/**
 * @brief Mixes two channels together with individual gain values
 *
 * @param input1 Pointer to first floating point buffer to mix (mono)
 * @param gain1 Gain applied to first buffer
 * @param input2 Pointer to second floating point buffer to mix (mono)
 * @param gain2 Gain applied to second buffer
 * @param output Mixed output buffer
 * @param audio_block_size The number of floating-point words to process in the buffer
 */
#pragma optimize_for_speed
void mix_2x1_gain(float * input1, float gain1, float * input2, float gain2,
		float * output, uint32_t audio_block_size) {
	for (int i = 0; i < audio_block_size; i++) {
		output[i] = (input1[i] * gain1) + (input2[i] * gain2);
	}
}
