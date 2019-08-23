/*
 * copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 *
 * This audio element implements an all-pass filter.  An all-pass filter
 * is a filter that has a gain of 1.0 at all frequencies but affects the
 * delay/phase at different frequencies.
 *
 * Allpass filters are an essential component of many reverb algorithms.
 *
 * For more information on allpass filters and this implementation, see:
 * https://ccrma.stanford.edu/~jos/pasp/Allpass_Two_Combs.html
 */

#include <stdlib.h>
#include "allpass_filter.h"

/**
 * @brief Initializes instance of an all-pass filter
 *
 * @param c Pointer to instance structure
 * @param delay_buffer Pointer to the delay buffer to use
 * @param delay_buffer_size The size of the delay buffer in float words
 * @param gain Gain parameter
 * @return Allpass result (enumeration)
 */
RESULT_ALLPASS_FILTER allpass_setup(ALLPASS_FILTER * c, float * delay_buffer,
		uint32_t delay_buffer_size, float gain) {

	float * ptr;

	if (c == NULL) {
		return ALLPASS_INVALID_INSTANCE_POINTER;
	}

	c->initialized = false;

	// If a buffer has been provided, use that
	if (delay_buffer == NULL) {
		return ALLPASS_INVALID_DELAY_POINTER;
	}

	// Set delay line
	c->delay_line = delay_buffer;
	c->delay_line_size = delay_buffer_size;
	c->index = 0;

	// Set gain parameter
	c->gain = gain;

	// Zero Delay Line
	for (int i = 0; i < delay_buffer_size; i++)
		c->delay_line[i] = 0.0;

	// Instance was successfully initialized
	c->initialized = true;
	return ALLPASS_OK;

}

/**
 * @brief Apply effect/process to a block of audio data
 *
 * @param c Pointer to instance structure
 * @param audio_in Pointer to floating point audio input buffer (mono)
 * @param audio_out Pointer to floating point output buffer (mono)
 * @param audio_block_size The number of floating-point words to process
 */
#pragma optimize_for_speed
void allpass_read(ALLPASS_FILTER * c, float * audio_in, float * audio_out,
		uint32_t audio_block_size) {

	// If this instance hasn't been properly initialized, pass audio through
	if (c == NULL || !c->initialized) {
		for (int i = 0; i < audio_block_size; i++) {
			audio_out[i] = audio_in[i];
		}
		return;
	}

	float * buffer = c->delay_line;
	int indx = c->index;
	int len = c->length;
	float gain = c->gain;
	float out, in;

	for (int i = 0; i < audio_block_size; i++) {
		out = -audio_in[i] * gain + buffer[indx];
		buffer[indx] = audio_in[i] + (buffer[indx] * gain);
		audio_out[i] = out;
		indx++;
		if (indx >= len)
			indx = 0;
	}

	c->index = indx;
}
