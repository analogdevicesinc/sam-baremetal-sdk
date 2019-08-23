/*
 * Copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 *
 * This is an implementation of a tremelo which is essentially an
 * effect that modulates the amplitude of the incoming signal using
 * a low frequency oscillator.
 *
 * This audio effect also serves as an example of how to utilize the
 * amplitude_modulation audio elements.
 */
#include <stdlib.h>

#include "effect_tremelo.h"

// Min/max limits and other constants
#define  TREMELO_RATE_MIN       (0.01)
#define  TREMELO_RATE_MAX       (100.0)
#define  TREMELO_DEPTH_MIN      (0.0)
#define  TREMELO_DEPTH_MAX      (1.0)

/**
 * @brief Initializes instance of a tremelo
 *
 * @param c Pointer to instance structure
 * @param depth Depth of modulation (0.0->1.0)
 * @param rate_hz Rate of modulation (0.01->100.0)
 * @param audio_sample_rate The system audio sample rate
 * @return Tremelo result (enumeration)
 */
RESULT_TREMELO tremelo_setup(TREMELO * c, float depth, float rate_hz,
		float audio_sample_rate) {

	if (c == NULL) {
		return TREMELO_INVALID_INSTANCE_POINTER;
	}

	c->initialized = false;

	if (rate_hz > TREMELO_RATE_MAX || rate_hz < TREMELO_RATE_MIN) {
		return TREMELO_INVALID_RATE;
	}
	if (depth > TREMELO_DEPTH_MAX || depth < TREMELO_DEPTH_MIN) {
		return TREMELO_INVALID_DEPTH;
	}

	amplitude_modulation_setup(&c->modulator, depth, rate_hz, AMP_MOD_SIN,
			audio_sample_rate);

	// Set sample rate for Hz rate calculations
	c->audio_sample_rate = audio_sample_rate;

	// Set t value for oscillator
	c->lfo_t = 0.0;
	c->lfo_t_inc = rate_hz / audio_sample_rate;

	// Instance was successfully initialized
	c->initialized = true;
	return TREMELO_OK;

}

/**
 * @brief Modify tremelo rate (Hz) parameter
 *
 * If the input parameter is out of bounds, it is clipped to the corresponding
 * min/max value.  This function will return a value indicating an
 * invalid input parameter was supplied but the effect will continue to operate.
 *
 * @param c Pointer to instance structure
 * @param new_rate_hz New tremelo rate in Hz (0.001 -> 100.0)
 *
 * @return Tremelo result (enumeration)
 */
RESULT_TREMELO tremelo_modify_rate(TREMELO * c, float rate_hz_new) {

	RESULT_TREMELO res;

	float rate_hz;
	if (rate_hz_new < TREMELO_RATE_MIN) {
		rate_hz = TREMELO_RATE_MIN;
		res = TREMELO_INVALID_RATE;
	} else if (rate_hz_new > TREMELO_RATE_MAX) {
		rate_hz = TREMELO_RATE_MAX;
		res = TREMELO_INVALID_RATE;
	} else {
		rate_hz = rate_hz_new;
		res = TREMELO_OK;
	}

	// Update instance parameters
	c->rate_hz = rate_hz;
	c->lfo_t_inc = rate_hz / c->audio_sample_rate;
	amplitude_modulation_modify_rate(&c->modulator, rate_hz);

	return res;

}

/**
 * @brief Modify tremelo depth parameter
 *
 * If the input parameter is out of bounds, it is clipped to the corresponding
 * min/max value.  This function will return a value indicating an
 * invalid input parameter was supplied but the effect will continue to operate.
 *
 * @param c Pointer to instance structure
 * @param new_depth New tremelo depth (0.0->1.0)
 *
 * @return Tremelo result (enumeration)
 */
RESULT_TREMELO tremelo_modify_depth(TREMELO * c, float depth_new) {

	RESULT_TREMELO res;

	float depth;
	if (depth_new < TREMELO_DEPTH_MIN) {
		depth = TREMELO_DEPTH_MIN;
		res = TREMELO_INVALID_DEPTH;
	} else if (depth_new > TREMELO_DEPTH_MAX) {
		depth = TREMELO_DEPTH_MAX;
		res = TREMELO_INVALID_DEPTH;
	} else {
		depth = depth_new;
		res = TREMELO_OK;
	}

	// Update instance parameters
	c->depth = depth;
	amplitude_modulation_modify_depth(&c->modulator, depth);

	return res;
}

/**
 * @brief Apply effect/process to a block of audio data
 *
 * @param c Pointer to instance structure
 * @param audio_in Pointer to floating point audio input buffer (mono)
 * @param audio_out Pointer to floating point output buffer (mono)
 * @param audio_block_size The number of floating-point words to process
 */
void tremelo_read(TREMELO * c, float * audio_in, float * audio_out,
		uint32_t audio_block_size) {

	// If this instance hasn't been properly initialized, pass audio through
	if (c == NULL || !c->initialized) {
		for (int i = 0; i < audio_block_size; i++) {
			audio_out[i] = audio_in[i];
		}
		return;
	}

	amplitude_modulation_read(&c->modulator, audio_in, audio_out,
	NULL, audio_block_size);

}
