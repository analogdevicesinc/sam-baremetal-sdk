/*
 * Copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 *
 * This audio element implements an amplitude modulator.  An amplitude modulator
 * changes the amplitude of an input signal over time typically using a low-
 * frequency oscillator (LFO).  This implementation can be configured to use an
 * internal LFO (using a number of common waveform types) or you can supply
 * your own modulation signal.
 *
 * Amplitude modulation is used most commonly when creating tremelos.  However,
 * using higher frequencies for the rate_hz input can also yield some very
 * interesting results.
 *
 */

#include <math.h>
#include <filters.h>
#include <stdlib.h>

#include "amplitude_modulation.h"
#include "oscillators.h"

// Min/max limits and other constants
#define     AMPLITUDE_MOD_MIN_RATE  (0.0)
#define     AMPLITUDE_MOD_MAX_RATE  (10000.0)
#define     AMPLITUDE_MOD_MIN_DEPTH (0.0)
#define     AMPLITUDE_MOD_MAX_DEPTH (1.0)

/**
 * @brief Initializes instance of an amplitude modulator
 *
 * @param c Pointer to instance structure
 * @param depth Depth of modulation
 * @param rate_hz The frequency of the modulation
 * @param type The type of LFO to be used
 * @param audio_sample_rate The audio sample rate (to compute rate_hz)
 * @return Amplitude modulation result (enumeration)
 */
RESULT_AMPLITUDE_MOD amplitude_modulation_setup(AMPLITUDE_MODULATION * c,
		float depth, float rate_hz, AMPLITUDE_MOD_TYPE type,
		float audio_sample_rate) {

	if (c == NULL) {
		return AMPLITUDE_MOD_INVALID_INSTANCE_POINTER;
	}

	c->initialized = false;

	if (rate_hz < AMPLITUDE_MOD_MIN_RATE || rate_hz > AMPLITUDE_MOD_MAX_RATE) {
		return AMPLITUDE_MOD_INVALID_RATE;
	}

	if (depth < AMPLITUDE_MOD_MIN_DEPTH || depth > AMPLITUDE_MOD_MAX_DEPTH) {
		return AMPLITUDE_MOD_INVALID_DEPTH;
	}

	// Set parameters
	c->type = type;
	c->mod_depth = depth;
	c->mod_rate_hz = rate_hz;

	c->audio_sample_rate = audio_sample_rate;

	// Initialize variables used as inputs to the LFO
	c->inc = c->mod_rate_hz / c->audio_sample_rate;
	c->t = 0;

	// Instance was successfully initialized
	c->initialized = true;
	return AMPLITUDE_MOD_OK;
}

/**
 * @brief Modify modulation depth parameter
 *
 * @param c Pointer to instance structure
 * @param new_depth Updated depth
 */
RESULT_AMPLITUDE_MOD amplitude_modulation_modify_depth(AMPLITUDE_MODULATION * c,
		float new_depth) {

	float depth;

	if (new_depth < AMPLITUDE_MOD_MIN_DEPTH) {
		depth = AMPLITUDE_MOD_MIN_DEPTH;
	} else if (new_depth > AMPLITUDE_MOD_MAX_DEPTH) {
		depth = AMPLITUDE_MOD_MAX_DEPTH;
	} else {
		depth = new_depth;
	}

	// Update parameter in instance
	c->mod_depth = depth;

	// Return result
	if (depth != new_depth) {
		return AMPLITUDE_MOD_INVALID_DEPTH;
	} else {
		return AMPLITUDE_MOD_OK;
	}
}

/**
 * @brief Modify modulation rate (Hz) parameter
 *
 * @param c Pointer to instance structure
 * @param new_rate_hz Updated modulation rate (>0 Hz)
 */
RESULT_AMPLITUDE_MOD amplitude_modulation_modify_rate(AMPLITUDE_MODULATION * c,
		float new_rate_hz) {

	float rate_hz;

	/**
	 * If the input parameter is out of bounds, clip it to the corresponding min/max
	 * and apply that value.  This function will return a flag indicating an
	 * invalid input parameter was supplied but it won't disable the effect.
	 */
	if (new_rate_hz < AMPLITUDE_MOD_MIN_RATE) {
		rate_hz = AMPLITUDE_MOD_MIN_RATE;
	} else if (new_rate_hz > AMPLITUDE_MOD_MAX_RATE) {
		rate_hz = AMPLITUDE_MOD_MAX_RATE;
	} else {
		rate_hz = new_rate_hz;
	}

	// Update parameter in instance
	c->mod_rate_hz = rate_hz;
	c->inc = c->mod_rate_hz / c->audio_sample_rate;

	// Return result
	if (rate_hz != new_rate_hz) {
		return AMPLITUDE_MOD_INVALID_RATE;
	} else {
		return AMPLITUDE_MOD_OK;
	}

}

/**
 * @brief Apply effect/process to a block of audio data
 *
 * @param c Pointer to instance structure
 * @param input Pointer to floating point audio input buffer (mono)
 * @param output Pointer to floating point output buffer (mono)
 * @param ext_mod A buffer of values from external LFO (same size as audio bufs)
 * @param audio_block_size The number of floating-point words to process in buffers
 */
#pragma optimize_for_speed
void amplitude_modulation_read(AMPLITUDE_MODULATION * c, float * audio_in,
		float * audio_out, float * ext_mod, uint32_t audio_block_size) {

	// If this instance hasn't been properly initialized, pass audio through
	if (c == NULL || !c->initialized) {
		for (int i = 0; i < audio_block_size; i++) {
			audio_out[i] = audio_in[i];
		}
		return;
	}

	float t = c->t;
	float inc = c->inc;
	float depth = c->mod_depth;
	float trem_factor;

	switch (c->type) {
	case AMP_MOD_SIN:
		for (int i = 0; i < audio_block_size; i++) {
			trem_factor = 1.0
					- (depth * (0.5 * oscillator_sine(t += inc) + 0.5));
			audio_out[i] = audio_in[i] * trem_factor;
		}
		break;

	case AMP_MOD_TRI:
		for (int i = 0; i < audio_block_size; i++) {
			trem_factor = 1.0
					- (depth * (0.5 * oscillator_triangle(t += inc) + 0.5));
			audio_out[i] = audio_in[i] * trem_factor;
		}
		break;

	case AMP_MOD_SQR:
		for (int i = 0; i < audio_block_size; i++) {
			trem_factor = 1.0
					- (depth * (0.5 * oscillator_square(t += inc) + 0.5));
			audio_out[i] = audio_in[i] * trem_factor;
		}
		break;

	case AMP_MOD_RAMP:
		for (int i = 0; i < audio_block_size; i++) {
			trem_factor = 1.0
					- (depth * (0.5 * oscillator_ramp(t += inc) + 0.5));
			audio_out[i] = audio_in[i] * trem_factor;
		}
		break;

	case AMP_MOD_EXT_LFO:
		for (int i = 0; i < audio_block_size; i++) {
			trem_factor = 1.0 - (depth * (0.5 * ext_mod[i] + 0.5));
			audio_out[i] = audio_in[i] * trem_factor;
		}
		break;

	}

	// Store updated and wrapped t value
	c->t = t - floor(t);

}
