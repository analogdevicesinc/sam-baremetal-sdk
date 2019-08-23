/*
 * Copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 */

#include <stdlib.h>
#include <stddef.h>

#include "integer_delay_lpf.h"

// Min/max limits and other constants
#define DELAY_MIN_FEEDBACK      (-1.0)
#define DELAY_MAX_FEEDBACK      (1.0)
#define DELAY_MIN_FEEDTHROUGH   (-1.0)
#define DELAY_MAX_FEEDTHROUGH   (1.0)
#define DELAY_MIN_ACOEFF        (0.001)
#define DELAY_MAX_ACOEFF        (0.999)

#define DELAY_LPF_LENGTH_TRANS_STEPS    (16000)

/**
 * @brief Initializes instance of a digital delay effect
 *
 * @param c Pointer to instance structure
 * @param delay_buffer Pointer to delay line buffer
 * @param delay_buffer_size Size of delay line buffer in floating point words
 * @param delay_initial_length Initial length of delay (location of read pointer)
 * @param feedback Amount of feedback (-1.0->1.0)
 * @param feedthrough Amount of feedthrough (-1.0->1.0)
 * @param a_coeff Dampening coefficent - set to 0.0 for no dampening
 * @return Delay result (enumeration)
 */
RESULT_DELAY delay_setup(DELAY_LPF * c, float * delay_buffer,
		uint32_t delay_buffer_size, uint32_t delay_initial_length,
		float feedback, float feedthrough, float a_coeff) {

	if (c == NULL) {
		return DELAY_INVALID_INSTANCE_POINTER;
	}

	c->initialized = false;

	if (delay_initial_length > delay_buffer_size) {
		return DELAY_LENGTH_EXCEEDS_BUF_SIZE;
	}

	// Check if buffer pointer is valid
	if (delay_buffer == NULL) {
		return DELAY_INVALID_DELAY_LINE_POINTER;
	}

	// Set delay parameters
	c->delay_line = delay_buffer;
	c->delay_line_size = delay_buffer_size;

	if (feedback < DELAY_MIN_FEEDBACK || feedback > DELAY_MAX_FEEDBACK) {
		return DELAY_INVALID_FEEDBACK;
	}
	c->feedback = feedback;

	if (feedthrough < DELAY_MIN_FEEDTHROUGH
			|| feedthrough > DELAY_MAX_FEEDTHROUGH) {
		return DELAY_INVALID_FEEDTHROUGH;
	}
	c->feedthrough = feedthrough;

	// Zero delay line
	for (int i = 0; i < delay_buffer_size; i++) {
		delay_buffer[i] = 0.0;
	}

	c->read_tap = delay_initial_length;
	c->read_tap_f = (float) c->read_tap;
	c->target_read_tap = delay_initial_length;

	c->write_ptr = 0;

	if (a_coeff != 0.0
			&& (a_coeff > DELAY_MAX_ACOEFF || a_coeff < DELAY_MIN_ACOEFF)) {
		return DELAY_INVALID_DAMPENING_COEFF;
	}

	c->lpf_a = a_coeff;
	c->lpf_hist = 0.0;

	// Instance was successfully initialized
	c->initialized = true;
	return DELAY_OK;
}

/**
 * @brief Update dampening coefficent (0.01->0.99) (lower value = lower cutoff frequency)
 *
 * If the input parameter is out of bounds, clip it to the corresponding min/max
 * and apply that value.  This function will return a flag indicating an
 * invalid input parameter was supplied but it won't disable the effect.
 *
 * @param c Pointer to instance structure
 * @param coeff_new Updated dampening coefficent
 *
 * @return Delay result (enumeration)
 */
RESULT_DELAY delay_modify_dampening(DELAY_LPF * c, float coeff_new) {

	RESULT_DELAY res;

	if (coeff_new == 0.0) {
		c->lpf_a = coeff_new;
		return DELAY_OK;
	}

	/**
	 * If the input parameter is out of bounds, clip it to the corresponding min/max
	 * and apply that value.  This function will return a flag indicating an
	 * invalid input parameter was supplied but it won't disable the effect.
	 */
	float coeff;
	if (coeff_new > DELAY_MAX_ACOEFF) {
		coeff = DELAY_MAX_ACOEFF;
		res = DELAY_INVALID_DAMPENING_COEFF;
	} else if (coeff_new < DELAY_MIN_ACOEFF) {
		coeff = DELAY_MIN_ACOEFF;
		res = DELAY_INVALID_DAMPENING_COEFF;
	} else {
		coeff = coeff_new;
		res = DELAY_OK;
	}

	// Calculate / update parameters
	c->lpf_a = coeff;

	return res;

}

/**
 * @brief Modify delay length
 *
 * If the input parameter is out of bounds, clip it to the corresponding min/max
 * and apply that value.  This function will return a flag indicating an
 * invalid input parameter was supplied but it won't disable the effect.
 *
 * @param c Pointer to instance structure
 * @param delay_length_new New delay line length
 *
 * @return Delay result (enumeration)
 */
RESULT_DELAY delay_modify_length(DELAY_LPF * c, uint32_t delay_length_new) {

	RESULT_DELAY res;

	/**
	 * If the input parameter is out of bounds, clip it to the corresponding min/max
	 * and apply that value.  This function will return a flag indicating an
	 * invalid input parameter was supplied but it won't disable the effect.
	 */
	uint32_t delay_length;
	if (delay_length_new > c->delay_line_size) {
		delay_length = c->delay_line_size;
		res = DELAY_LENGTH_EXCEEDS_BUF_SIZE;
	} else {
		delay_length = delay_length_new;
		res = DELAY_OK;
	}

	if (delay_length_new == c->read_tap) {
		return res;
	}

	// Calculate / update parameters
	c->target_read_tap = delay_length;
	c->read_tap_inc = (float) ((int32_t) delay_length - c->read_tap)
			* (1.0 / DELAY_LPF_LENGTH_TRANS_STEPS);
	c->read_tap_steps = DELAY_LPF_LENGTH_TRANS_STEPS;

	return res;
}

/**
 * @brief Modify delay feedback
 *
 * If the input parameter is out of bounds, clip it to the corresponding min/max
 * and apply that value.  This function will return a flag indicating an
 * invalid input parameter was supplied but it won't disable the effect.
 *
 * @param c Pointer to instance structure
 * @param feedback_new Updated feedback value
 *
 * @return Delay result (enumeration)
 */
RESULT_DELAY delay_modify_feedback(DELAY_LPF * c, float feedback_new) {

	RESULT_DELAY res;

	float feedback;
	if (feedback_new > DELAY_MAX_FEEDBACK) {
		feedback = DELAY_MAX_FEEDBACK;
		res = DELAY_INVALID_FEEDBACK;
	} else if (feedback_new < DELAY_MIN_FEEDBACK) {
		feedback = DELAY_MIN_FEEDBACK;
		res = DELAY_INVALID_FEEDBACK;
	} else {
		feedback = feedback_new;
		res = DELAY_OK;
	}

	// Calculate / update parameters
	c->feedback = feedback;

	return res;
}

/**
 * @brief Modify feedthrough (dry) value
 *
 * If the input parameter is out of bounds, clip it to the corresponding min/max
 * and apply that value.  This function will return a flag indicating an
 * invalid input parameter was supplied but it won't disable the effect.
 *
 * @param c Pointer to instance structure
 * @param feedthrough_new Updated feedthrough (dry) value
 *
 * @return Delay result (enumeration)
 */
RESULT_DELAY delay_modify_feedthrough(DELAY_LPF * c, float feedthrough_new) {

	RESULT_DELAY res;

	float feedthrough;
	if (feedthrough_new > DELAY_MAX_FEEDTHROUGH) {
		feedthrough = DELAY_MAX_FEEDTHROUGH;
		res = DELAY_INVALID_FEEDTHROUGH;
	} else if (feedthrough_new < DELAY_MIN_FEEDTHROUGH) {
		feedthrough = DELAY_MIN_FEEDTHROUGH;
		res = DELAY_INVALID_FEEDTHROUGH;
	} else {
		feedthrough = feedthrough_new;
		res = DELAY_OK;
	}

	// Calculate / update parameters
	c->feedthrough = feedthrough;

	return res;
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
void delay_read(DELAY_LPF * c, float * audio_in, float * audio_out,
		uint32_t audio_block_size) {

	// If this instance hasn't been properly initialized, pass audio through
	if (c == NULL || !c->initialized) {
		for (int i = 0; i < audio_block_size; i++) {
			audio_out[i] = audio_in[i];
		}
	}

	int i;

	float * buffer = c->delay_line;
	int len = c->delay_line_size;
	float feedback_amt = c->feedback;
	float feedthrough_amt = c->feedthrough;

	// Intermediate values
	float out, in, fb;
	float lpf_hist = c->lpf_hist;
	float lpf_a = c->lpf_a;

	// Set initial taps
	int write_ptr = c->write_ptr;
	int read_tap = write_ptr - c->read_tap;
	if (read_tap < 0) {
		read_tap += len;
	}

	if (c->lpf_a != 0.0) {
		// Perform delay with LPF (LBCF)
		for (i = 0; i < audio_block_size; i++) {

			audio_out[i] = (audio_in[i] * c->feedthrough) + buffer[read_tap];
			out = audio_in[i] + buffer[read_tap];
			buffer[write_ptr] = lpf_hist;
			lpf_hist += lpf_a * (out * c->feedback - lpf_hist);

			write_ptr++;
			if (write_ptr >= len) {
				write_ptr = 0;
			}

			// If we are adjusting the delay length, do it here
			if (c->read_tap_steps) {
				c->read_tap_steps--;
				if (c->read_tap_steps == 0) {
					c->read_tap = c->target_read_tap;
					c->read_tap_f = (float) c->read_tap;
				} else {
					c->read_tap_f += c->read_tap_inc;
					c->read_tap = (uint32_t) c->read_tap_f;
				}
			}
			read_tap = write_ptr - c->read_tap;
			if (read_tap < 0) {
				read_tap += len;
			}
		}
		c->lpf_hist = lpf_hist;
	} else {
		// Perform standard delay
		for (i = 0; i < audio_block_size; i++) {

			audio_out[i] = (audio_in[i] * c->feedthrough) + buffer[read_tap];
			out = audio_in[i] + buffer[read_tap];

			fb = out * c->feedback;
			buffer[write_ptr] = fb;

			write_ptr++;
			if (write_ptr >= len) {
				write_ptr = 0;
			}

			// If we are adjusting the delay length, do it here
			if (c->read_tap_steps) {
				c->read_tap_steps--;
				if (c->read_tap_steps == 0) {
					c->read_tap = c->target_read_tap;
					c->read_tap_f = (float) c->read_tap;
				} else {
					c->read_tap_f += c->read_tap_inc;
					c->read_tap = (uint32_t) c->read_tap_f;
				}
			}
			read_tap = write_ptr - c->read_tap;
			if (read_tap < 0) {
				read_tap += len;
			}

		}
	}

	// Store index back into instance struct
	c->write_ptr = write_ptr;

}

