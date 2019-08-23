/*
 * copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 *
 * A ring modulator multiplies the incoming signal with a sine tone
 * which frequency modulates the original signal.  The effect is a bit
 * avant garde.
 *
 * Here's a nice write up of songs that feature a ring modulator
 * https://www.theguardian.com/music/2009/nov/09/ring-modulators
 *
 */
#include <stdlib.h>

#include "effect_ring_modulator.h"
#include "../audio_elements/oscillators.h"

// Min/max limits and other constants
#define RING_MOD_DEPTH_MIN      (0.0)
#define RING_MOD_DEPTH_MAX      (1.0)
#define RING_MOD_FREQ_HZ_MIN    (10.0)
#define RING_MOD_FREQ_HZ_MAX    (10000.0)

/**
 * @brief Initializes instance of a ring modulator
 *
 * @param c Pointer to instance structure
 * @param freq Ring modulator modulation frequency (10.0->10000.0)
 * @param depth Ring modulator depth (0.0->1.0)
 * @param audio_sample_rate The system audio sample rate
 * @return Ring modulator result (enumeration)
 */
RESULT_RING_MOD ring_modulator_setup(RING_MODULATOR * c, float freq,
		float depth, float audio_sample_rate) {

	if (c == NULL) {
		return RING_MOD_INVALID_INSTANCE_POINTER;
	}

	c->initialized = false;

	if (freq > RING_MOD_FREQ_HZ_MAX || freq < RING_MOD_FREQ_HZ_MIN) {
		return RING_MOD_INVALID_FREQ;
	}
	if (depth > RING_MOD_DEPTH_MAX || depth < RING_MOD_DEPTH_MIN) {
		return RING_MOD_INVALID_DEPTH;
	}

	c->t = 0;
	c->t_inc = 1.0 / (audio_sample_rate / freq);

	c->depth = depth;

	c->audio_sample_rate = audio_sample_rate;

	// Instance was successfully initialized
	c->initialized = true;
	return RING_MOD_OK;

}

/**
 * @brief Modify ring modulator modulation frequency parameter
 *
 * If the input parameter is out of bounds, it is clipped to the corresponding
 * min/max value.  This function will return a value indicating an
 * invalid input parameter was supplied but the effect will continue to operate.
 *
 * @param c Pointer to instance structure
 * @param freq_new New modulation frequency (10.0->10000.0)
 * @return Ring modulator result (enumeration)
 */
RESULT_RING_MOD ring_modulator_modify_freq(RING_MODULATOR * c, float freq_new) {

	RESULT_RING_MOD res;

	float freq;
	if (freq_new < RING_MOD_FREQ_HZ_MIN) {
		freq = RING_MOD_FREQ_HZ_MIN;
		res = RING_MOD_INVALID_FREQ;
	} else if (freq_new > RING_MOD_FREQ_HZ_MAX) {
		freq = RING_MOD_FREQ_HZ_MAX;
		res = RING_MOD_INVALID_FREQ;
	} else {
		freq = freq_new;
		res = RING_MOD_OK;
	}
	// Update instance parameters
	c->t_inc = 1.0 / (c->audio_sample_rate / freq_new);

	return res;

}

/**
 * @brief Modify ring modulator depth parameter
 *
 * If the input parameter is out of bounds, it is clipped to the corresponding
 * min/max value.  This function will return a value indicating an
 * invalid input parameter was supplied but the effect will continue to operate.
 *
 * @param c Pointer to instance structure
 * @param depth_new New depth parameter (0.0->1.0)
 * @return Ring modulator result (enumeration)
 */
RESULT_RING_MOD ring_modulator_modify_depth(RING_MODULATOR * c, float depth_new) {

	RESULT_RING_MOD res;

	float depth;
	if (depth_new < RING_MOD_DEPTH_MIN) {
		depth = RING_MOD_DEPTH_MIN;
		res = RING_MOD_INVALID_DEPTH;
	} else if (depth_new > RING_MOD_DEPTH_MAX) {
		depth = RING_MOD_DEPTH_MAX;
		res = RING_MOD_INVALID_DEPTH;
	} else {
		depth = depth_new;
		res = RING_MOD_OK;
	}

	// Update instance parameters
	c->depth = depth_new;

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
void ring_modulator_read(RING_MODULATOR * c, float * audio_in,
		float * audio_out, uint32_t audio_block_size) {

	// If this instance hasn't been properly initialized, pass audio through
	if (c == NULL || !c->initialized) {
		for (int i = 0; i < audio_block_size; i++) {
			audio_out[i] = audio_in[i];
		}
		return;
	}

	for (int i = 0; i < audio_block_size; i++) {
		audio_out[i] = (1.0 - c->depth) * audio_in[i]
				+ c->depth * audio_in[i] * oscillator_sine(c->t);
		c->t += c->t_inc;
	}
	c->t = c->t - floor(c->t);

}
