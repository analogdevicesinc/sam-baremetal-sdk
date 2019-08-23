/*
 * copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 */

#include "effect_stereo_flanger.h"

// Min/max limits and other constants
#define FLANGER_DEPTH_MIN      (0.0)
#define FLANGER_DEPTH_MAX      (1.0)
#define FLANGER_FEEDBACK_MIN   (-1.0)
#define FLANGER_FEEDBACK_MAX   (1.0)
#define FLANGER_RATE_HZ_MIN    (0.01)
#define FLANGER_RATE_HZ_MAX    (10.0)

/**
 * @brief Initializes instance of a stereo flanger
 *
 * @param c Pointer to instance structure
 * @param depth Flanger depth (0.0->1.0)
 * @param rate_hz Flanger modulation rate (Hz) (0.01->10.0)
 * @param feedback Flanger feedback from output to input
 * @param audio_sample_rate The system audio sample rate
 * @return flanger result (enumeration)
 */
RESULT_FLANGER flanger_setup(STEREO_FLANGER * c, float depth, float rate_hz,
		float feedback, float audio_sample_rate) {

	if (c == NULL) {
		return FLANGER_INVALID_INSTANCE_POINTER;
	}

	c->initialized = false;

	if (depth < FLANGER_DEPTH_MIN || depth > FLANGER_DEPTH_MAX) {
		return FLANGER_INVALID_DEPTH;
	}

	if (rate_hz < FLANGER_RATE_HZ_MIN || rate_hz > FLANGER_RATE_HZ_MAX) {
		return FLANGER_INVALID_RATE;
	}

	if (feedback < FLANGER_FEEDBACK_MIN || feedback > FLANGER_FEEDBACK_MAX) {
		return FLANGER_INVALID_FEEDBACK;
	}

	variable_delay_setup(&c->var_del_left, depth, feedback, rate_hz,
			audio_sample_rate, VARIABLE_DELAY_EXT_LFO);

	variable_delay_setup(&c->var_del_right, depth, feedback, rate_hz,
			audio_sample_rate, VARIABLE_DELAY_EXT_LFO);

	// Set up oscillators to be 180 degrees out of phase
	c->lfo_t_left = 0.0;
	c->lfo_t_right = 0.5;
	c->inc = rate_hz / audio_sample_rate;

	c->audio_sample_rate = audio_sample_rate;

	// Instance was successfully initialized
	c->initialized = true;
	return FLANGER_OK;

}

/**
 * @brief Modify flanger rate (Hz)
 *
 * If the input parameter is out of bounds, it is clipped to the corresponding
 * min/max value.  This function will return a value indicating an
 * invalid input parameter was supplied but the effect will continue to operate.
 *
 * @param c Pointer to instance structure
 * @param new_rate_hz New flanger rate in Hz (0.01->10.0)
 * @return flanger result (enumeration)
 */
RESULT_FLANGER flanger_modify_rate(STEREO_FLANGER * c, float rate_hz_new) {

	RESULT_FLANGER res;

	float rate_hz;
	if (rate_hz_new < FLANGER_RATE_HZ_MIN) {
		rate_hz = FLANGER_RATE_HZ_MIN;
		res = FLANGER_INVALID_RATE;
	} else if (rate_hz_new > FLANGER_RATE_HZ_MAX) {
		rate_hz = FLANGER_RATE_HZ_MAX;
		res = FLANGER_INVALID_RATE;
	} else {
		rate_hz = rate_hz_new;
		res = FLANGER_OK;
	}

	// Update instance parameters
	c->rate_hz = rate_hz;
	c->inc = c->rate_hz / c->audio_sample_rate;

	return res;
}

/**
 * @brief Modify flanger depth value
 *
 * If the input parameter is out of bounds, it is clipped to the corresponding
 * min/max value.  This function will return a value indicating an
 * invalid input parameter was supplied but the effect will continue to operate.
 *
 * @param c Pointer to instance structure
 * @param new_depth New flanger depth (0.0->1.0)
 * @return flanger result (enumeration)
 */
RESULT_FLANGER flanger_modify_depth(STEREO_FLANGER * c, float depth_new) {

	RESULT_FLANGER res;

	float depth;
	if (depth_new < FLANGER_DEPTH_MIN) {
		depth = FLANGER_DEPTH_MIN;
		res = FLANGER_INVALID_DEPTH;
	} else if (depth_new > FLANGER_DEPTH_MAX) {
		depth = FLANGER_DEPTH_MAX;
		res = FLANGER_INVALID_DEPTH;
	} else {
		depth = depth_new;
		res = FLANGER_OK;
	}

	// Update instance parameters
	c->depth = depth;
	variable_delay_modify_depth(&c->var_del_left, depth);
	variable_delay_modify_depth(&c->var_del_right, depth);

	return res;
}

/**
 * @brief Modify flanger feedback value
 *
 * If the input parameter is out of bounds, it is clipped to the corresponding
 * min/max value.  This function will return a value indicating an
 * invalid input parameter was supplied but the effect will continue to operate.
 *
 * @param c Pointer to instance structure
 * @param new_feedback New flanger feedback value (-1.0->1.0)
 * @return flanger result (enumeration)
 */
RESULT_FLANGER flanger_modify_feedback(STEREO_FLANGER * c, float feedback_new) {

	RESULT_FLANGER res;

	float feedback;
	if (feedback_new < FLANGER_FEEDBACK_MIN) {
		feedback = FLANGER_FEEDBACK_MIN;
		res = FLANGER_INVALID_FEEDBACK;
	} else if (feedback_new > FLANGER_FEEDBACK_MAX) {
		feedback = FLANGER_FEEDBACK_MAX;
		res = FLANGER_INVALID_FEEDBACK;
	} else {
		feedback = feedback_new;
		res = FLANGER_OK;
	}

	// Update instance parameters
	c->feedback = feedback;
	variable_delay_modify_feedback(&c->var_del_left, feedback);
	variable_delay_modify_feedback(&c->var_del_right, feedback);

	return res;
}

/**
 * @brief Apply effect/process to a block of audio data
 *
 * @param c Pointer to instance structure
 * @param audio_in Pointer to floating point audio input buffer (mono)
 * @param audio_out_left Pointer to floating point output buffer (left mono)
 * @param audio_out_right Pointer to floating point output buffer (right mono)
 * @param audio_block_size The number of floating-point words to process
 */
void flanger_read(STEREO_FLANGER * c, float * audio_in, float * audio_out_left,
		float * audio_out_right, uint32_t audio_block_size) {

	// If this instance hasn't been properly initialized, pass audio through
	if (c == NULL || !c->initialized) {
		for (int i = 0; i < audio_block_size; i++) {
			audio_out_left[i] = audio_in[i];
			audio_out_right[i] = audio_in[i];
		}
		return;
	}

	float lfo_left[MAX_AUDIO_BLOCK_SIZE], lfo_right[MAX_AUDIO_BLOCK_SIZE];

	// Generate LFO signal
	float t_l = c->lfo_t_left;
	float t_r = c->lfo_t_right;
	float inc = c->inc;
	for (int i = 0; i < audio_block_size; i++) {
		lfo_left[i] = oscillator_sine(t_l += inc);
		lfo_right[i] = oscillator_sine(t_r += inc);
	}
	c->lfo_t_left = t_l - floor(t_l);
	c->lfo_t_right = t_r - floor(t_r);

	variable_delay_read(&c->var_del_left, audio_in, audio_out_left, lfo_left,
			audio_block_size);
	variable_delay_read(&c->var_del_right, audio_in, audio_out_right, lfo_right,
			audio_block_size);

}

