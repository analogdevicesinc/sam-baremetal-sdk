/*
 * Copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 *
 * A multitap delay line is a delay line that has multiple read
 * "taps" that pull audio off the delay line at different delay
 * values.  Multitap delays are used in reverb algorithms but can also
 * be used to create interesting echo and delay effects.
 *
 */

#include <stdlib.h>
#include <stddef.h>

#include "integer_delay_multitap.h"

/**
 * @brief Initializes instance of a multi-tap delay
 *
 * @param c Pointer to instance structure
 * @param delay_line Pointer to delay line
 * @param delay_line_size Length of delay line in samples / floating point words
 * @param num_taps Number of delay line taps
 * @param tap_offsets A pointer to an array of offsets for each tap
 * @param tap_gains A pointer to an array of gains for each tap
 * @param feedthrough The clean mix of audio passed through
 * @return Multitap delay result (enumeration)
 */
RESULT_MT_DELAY multitap_delay_setup(MULTITAP_DELAY * c, float * delay_line,
		uint32_t delay_line_size, uint32_t num_taps, uint32_t * tap_offsets,
		float * tap_gains, float feedthrough) {

	if (c == NULL) {
		return MT_DELAY_INVALID_INSTANCE_POINTER;
	}
	c->initialized = false;

	// Allocate our buffer
	if (delay_line == NULL) {
		return MT_DELAY_INVALID_DELAY_LINE_POINTER;
	}

	if (num_taps > MULTITAP_DELAY_MAX_TAPS) {
		return MT_DELAY_TOO_MANY_TAPS;
	}

	if (tap_offsets == NULL || tap_gains == NULL) {
		return MT_DELAY_INVALID_TAPS_POINTER;
	}

	// Set delay parameters
	c->delay_line = delay_line;
	c->delay_line_size = delay_line_size;
	c->feedthrough = feedthrough;

	c->num_taps = num_taps;
	for (int tap = 0; tap < c->num_taps; tap++) {
		if (tap_offsets[tap] > delay_line_size) {
			return MT_DELAY_TAP_EXCEEDS_DELAY_LINE_LEN;
		}
		c->tap_offsets[tap] = tap_offsets[tap];
		c->tap_gains[tap] = tap_gains[tap];
	}

	// Zero delay line
	int i;
	float * ptr;
	ptr = c->delay_line;
	for (i = 0; i < delay_line_size; i++)
		delay_line[i] = 0.0;
	c->index = 0;

	c->initialized = true;
	return MT_DELAY_OK;
}

/**
 * @brief Modify the tap sizes
 *
 * @param c Pointer to instance structure
 * @param new_tap_offsets Pointer to array with new offsets
 *
 * @return Multitap delay result (enumeration)
 */
#pragma optimize_for_speed
RESULT_MT_DELAY multitap_delay_modify_taps(MULTITAP_DELAY * c,
		uint32_t * new_tap_offsets) {

	// Copy new taps into instance struct
	for (int tap = 0; tap < c->num_taps; tap++) {
		if (new_tap_offsets[tap] > c->delay_line_size) {
			return MT_DELAY_TAP_EXCEEDS_DELAY_LINE_LEN;
		}
		c->tap_offsets[tap] = new_tap_offsets[tap];
	}

	return MT_DELAY_OK;

}

/**
 * @brief Apply effect/process to a block of audio data
 *
 * @param c Pointer to instance structure
 * @param audio_in Pointer to floating point audio input buffer (mono)
 * @param audio_out Pointer to floating point audio output buffer (mono)
 * @param audio_block_size The number of floating-point words to process
 */
#pragma optimize_for_speed
void multitap_delay_read(MULTITAP_DELAY * c, float * audio_in,
		float * audio_out, uint32_t audio_block_size) {

	// If this instance hasn't been properly initialized, pass audio through
	if (c == NULL || !c->initialized) {
		for (int i = 0; i < audio_block_size; i++) {
			audio_out[i] = audio_in[i];
		}
		return;
	}

	float * delay_buffer = c->delay_line;
	uint32_t indx = c->index;

	for (int i = 0; i < audio_block_size; i++) {
		delay_buffer[indx] = audio_in[i];
		audio_out[i] = audio_in[i] * c->feedthrough;

		for (int tap = 0; tap < c->num_taps; tap++) {
			int32_t tap_pos = c->tap_offsets[tap];
			tap_pos = indx - tap_pos;
			if (tap_pos < 0) {
				tap_pos += c->delay_line_size;
			}
			audio_out[i] += delay_buffer[tap_pos] * c->tap_gains[tap];
		}
		indx++;
		if (indx >= c->delay_line_size) {
			indx = 0;
		}
	}

	// Store index back into instance struct
	c->index = indx;
}

