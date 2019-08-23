/*
 * Copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 */

#include <stdlib.h>

#include "variable_delay.h"
#include "oscillators.h"

// Min/max limits and other constants
#define VAR_DELAY_FEEDBACK_MIN      (-0.99)
#define VAR_DELAY_FEEDBACK_MAX      (0.99)
#define VAR_DELAY_DEPTH_MIN         (0.0)
#define VAR_DELAY_DEPTH_MAX         (1.0)
#define VAR_DELAY_RATE_HZ_MIN       (0.0)
#define VAR_DELAY_RATE_HZ_MAX       (10.0)

/**
 * @brief Initializes instance of a variable delay
 *
 * @param c Pointer to instance structure
 * @param depth Depth of modulation
 * @param feedback Feedback from output to input
 * @param rate_hz Rate of modulation in Hz
 * @param audio_sample_rate The system audio sample rate
 * @param type Type of modulator (see enumeration)
 * @return variable delay result (enumeration)
 */
RESULT_VARIABLE_DELAY variable_delay_setup(VARIABLE_DELAY * c, float depth,
		float feedback, float rate_hz, float audio_sample_rate,
		VARIABLE_DELAY_TYPE type) {
	int d, i;

	float inv_d, x;

	if (c == NULL) {
		return VARIABLE_DELAY_INVALID_INSTANCE_POINTER;
	}
	c->initialized = false;

	if (depth > VAR_DELAY_DEPTH_MAX || depth < VAR_DELAY_DEPTH_MIN) {
		return VARIABLE_DELAY_INVALID_DEPTH;
	}
	if (feedback > VAR_DELAY_FEEDBACK_MAX || feedback < VAR_DELAY_FEEDBACK_MIN) {
		return VARIABLE_DELAY_INVALID_FEEDBACK;
	}
	if (rate_hz > VAR_DELAY_RATE_HZ_MAX || rate_hz < VAR_DELAY_RATE_HZ_MIN) {
		return VARIABLE_DELAY_INVALID_RATE;
	}

	// Save parameters
	c->feedback = feedback;
	c->mod_depth = depth;
	c->mod_rate_hz = rate_hz;

	c->audio_sample_rate = audio_sample_rate;
	c->inc = rate_hz / c->audio_sample_rate;

	c->delay_index = 0;

	// clear delay line
	for (i = 0; i < VARIABLE_DELAY_MAX_DEPTH + VARIABLE_DELAY_PRE_DELAY; i++)
		c->delay_buffer[i] = 0.0;

	c->initialized = true;
	return VARIABLE_DELAY_OK;
}

/**
 * @brief Modify variable delay feedback parameter
 *
 * If the input parameter is out of bounds, clip it to the corresponding min/max
 * and apply that value.  This function will return a flag indicating an
 * invalid input parameter was supplied but it won't disable the effect.
 *
 * @param c Pointer to instance structure
 * @param new_feedback Update feedback value
 * @return variable delay result (enumeration)
 */
RESULT_VARIABLE_DELAY variable_delay_modify_feedback(VARIABLE_DELAY * c,
		float feedback_new) {

	RESULT_VARIABLE_DELAY res;

	float feedback;
	if (feedback_new > VAR_DELAY_FEEDBACK_MAX) {
		feedback = VAR_DELAY_FEEDBACK_MAX;
		res = VARIABLE_DELAY_INVALID_FEEDBACK;
	} else if (feedback_new < VAR_DELAY_FEEDBACK_MIN) {
		feedback = VAR_DELAY_FEEDBACK_MIN;
		res = VARIABLE_DELAY_INVALID_FEEDBACK;
	} else {
		feedback = feedback_new;
		res = VARIABLE_DELAY_OK;
	}

	c->feedback = feedback;

	return res;
}

/**
 * @brief Modify variable delay modulation depth
 *
 * If the input parameter is out of bounds, clip it to the corresponding min/max
 * and apply that value.  This function will return a flag indicating an
 * invalid input parameter was supplied but it won't disable the effect.
 *
 * @param c Pointer to instance structure
 * @param new_depth Updated depth parameter
 * @return variable delay result (enumeration)
 */
RESULT_VARIABLE_DELAY variable_delay_modify_depth(VARIABLE_DELAY * c,
		float depth_new) {

	RESULT_VARIABLE_DELAY res;

	float depth;
	if (depth_new > VAR_DELAY_DEPTH_MAX) {
		depth = VAR_DELAY_DEPTH_MAX;
		res = VARIABLE_DELAY_INVALID_DEPTH;
	} else if (depth_new < VAR_DELAY_DEPTH_MIN) {
		depth = VAR_DELAY_DEPTH_MIN;
		res = VARIABLE_DELAY_INVALID_DEPTH;
	} else {
		depth = depth_new;
		res = VARIABLE_DELAY_OK;
	}

	c->mod_depth = depth;

	return res;
}

/**
 * @brief Modify variable delay modulation rate
 *
 * If the input parameter is out of bounds, clip it to the corresponding min/max
 * and apply that value.  This function will return a flag indicating an
 * invalid input parameter was supplied but it won't disable the effect.
 *
 * @param c Pointer to instance structure
 * @param new_rate_hz Updated rate parameter in Hz
 * @return variable delay result (enumeration)
 */
RESULT_VARIABLE_DELAY variable_delay_modify_rate(VARIABLE_DELAY * c,
		float rate_hz_new) {

	RESULT_VARIABLE_DELAY res;

	float rate_hz;
	if (rate_hz_new > VAR_DELAY_RATE_HZ_MAX) {
		rate_hz = VAR_DELAY_RATE_HZ_MAX;
		res = VARIABLE_DELAY_INVALID_RATE;
	} else if (rate_hz_new < VAR_DELAY_RATE_HZ_MIN) {
		rate_hz = VAR_DELAY_RATE_HZ_MIN;
		res = VARIABLE_DELAY_INVALID_RATE;
	} else {
		rate_hz = rate_hz_new;
		res = VARIABLE_DELAY_OK;
	}

	c->mod_rate_hz = rate_hz;
	c->inc = c->mod_rate_hz / c->audio_sample_rate;

	return res;
}

/**
 * @brief Apply effect/process to a block of audio data
 *
 * @param c Pointer to instance structure
 * @param audio_in Pointer to floating point audio input buffer (mono)
 * @param audio_out Pointer to floating point output buffer (mono)
 * @param ext_mod An external waveform used to modulate input
 * @param audio_block_size The number of floating-point words to process
 */
#pragma optimize_for_speed
void variable_delay_read(VARIABLE_DELAY * c, float * audio_in,
		float * audio_out, float * ext_mod, uint32_t audio_block_size) {

	// If this instance hasn't been properly initialized, pass audio through
	if (c == NULL || !c->initialized) {
		for (int i = 0; i < audio_block_size; i++) {
			audio_out[i] = audio_in[i];
		}
		return;
	}

	float * delay_buf = c->delay_buffer;
	int delay_indx = c->delay_index;
	int delay_len = VARIABLE_DELAY_MAX_DEPTH;

	float delayed, mod_depth;
	float mod_pointer, original, last_sample;

	uint32_t indx, indx2;
	float delta;

	float t = c->t;
	float inc = c->inc;

	last_sample = c->feedback_lastsamp;

	for (int i = 0; i < audio_block_size; i++) {
		if (c->mod_type == VARIABLE_DELAY_EXT_LFO) {
			mod_pointer = VARIABLE_DELAY_PRE_DELAY
					+ (0.5 * ext_mod[i] * c->mod_depth
							* VARIABLE_DELAY_MAX_DEPTH * 0.9);
		} else {
			switch (c->mod_type) {
			case VARIABLE_DELAY_SIN:
				mod_pointer = VARIABLE_DELAY_PRE_DELAY
						+ (0.5 * oscillator_sine(t += inc) + 0.5) * c->mod_depth
								* VARIABLE_DELAY_MAX_DEPTH * 0.9;
				break;
			case VARIABLE_DELAY_TRI:
				mod_pointer = VARIABLE_DELAY_PRE_DELAY
						+ (0.5 * oscillator_triangle(t += inc) + 0.5)
								* c->mod_depth * VARIABLE_DELAY_MAX_DEPTH * 0.9;
				break;
			case VARIABLE_DELAY_SQR:
				mod_pointer = VARIABLE_DELAY_PRE_DELAY
						+ (0.5 * oscillator_square(t += inc) + 0.5)
								* c->mod_depth * VARIABLE_DELAY_MAX_DEPTH * 0.9;
				break;
			default:
				break;
			}
		}

		original = audio_in[i];

		mod_pointer = delay_indx - mod_pointer;
		if (mod_pointer < 0)
			mod_pointer += delay_len;

		// Interpolate delayed signal
		indx = (uint32_t) mod_pointer;
		indx2 = (indx + 1) % VARIABLE_DELAY_MAX_DEPTH;
		delta = mod_pointer - (float) indx;
		delayed = delay_buf[indx] * (1.0 - delta) + delay_buf[indx2] * delta;

		last_sample = delayed;
		delay_buf[delay_indx] = original + last_sample * c->feedback;

		audio_out[i] = delayed + original;

		delay_indx++;
		if (delay_indx >= VARIABLE_DELAY_MAX_DEPTH) {
			delay_indx = 0;
		}

	}

	// Save state back to C struct
	c->feedback_lastsamp = last_sample;
	c->delay_index = delay_indx;

	// Wrap t and save it
	t = t - floor(t);
	c->t = t;

}

