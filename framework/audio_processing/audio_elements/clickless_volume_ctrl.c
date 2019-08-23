/*
 * Copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 */
#include <math.h>
#include <stdlib.h>

#include "clickless_volume_ctrl.h"

// Min/max limits and other constants
#define     VOLUME_MAX_GAIN_LIN     (10.0)
#define     VOLUME_MIN_GAIN_LIN     (0.0)
#define     VOLUME_MAX_GAIN_DB      (100.0)
#define     VOLUME_MIN_GAIN_DB      (100.0)

/**
 * @brief Initializes instance of a clickless volume control
 *
 * @param c Pointer to instance structure
 * @param gain Initial gain
 *
 * @return Volume control result (enumeration)
 */
RESULT_VOLUME_CTRL volume_control_setup(VOLUME_CTRL * c, float gain) {

	// Ensure we don't have a null pointer
	if (c == NULL) {
		return VOLUME_INVALID_INSTANCE_POINTER;
	}

	c->initialized = false;

	// Ensure the initial gain isn't greater than the max allowed
	if (gain < VOLUME_MIN_GAIN_LIN || gain > VOLUME_MAX_GAIN_LIN) {
		return VOLUME_INVALID_GAIN;
	}

	// Initialize our state variables
	c->current_gain = gain;
	c->target_gain = gain;
	c->remaining_steps = 0;

	// Instance was successfully initialized
	c->initialized = true;
	return VOLUME_OK;

}

/**
 * @brief Modify the gain of the volume control
 *
 *
 * @param c Pointer to instance structure
 * @param gain_new Updated volume level
 * @param speed Speed of transition from current levels to new levels
 * @return Volume control result (enumeration)
 */
RESULT_VOLUME_CTRL volume_control_set_gain(VOLUME_CTRL * c, float gain_new,
		VOLUME_CTRL_TRANSITION_SPEED speed) {

	RESULT_VOLUME_CTRL res;

	/**
	 * If the input parameter is out of bounds, clip it to the corresponding min/max
	 * and apply that value.  This function will return a flag indicating an
	 * invalid input parameter was supplied but it won't disable the effect.
	 */
	float gain;
	if (gain_new > VOLUME_MAX_GAIN_LIN) {
		gain = VOLUME_MAX_GAIN_LIN;
		res = VOLUME_INVALID_GAIN;
	} else if (gain_new < VOLUME_MIN_GAIN_LIN) {
		gain = VOLUME_MIN_GAIN_LIN;
		res = VOLUME_INVALID_GAIN;
	} else {
		gain = gain_new;
		res = VOLUME_OK;
	}

	// If nothing has changed since last time we modified this parameter, return
	if (gain == c->gain_last) {
		return res;
	} else {
		c->gain_last = gain;
	}

	// Set the number of transition steps based on the transition speed
	c->remaining_steps = (uint32_t) speed;
	float difference = gain_new - c->current_gain;
	c->delta = difference / (float) c->remaining_steps;

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
void volume_control_read(VOLUME_CTRL * c, float * audio_in, float * audio_out,
		uint32_t audio_block_size) {

	// If this instance hasn't been properly initialized, pass audio through
	if (c == NULL || !c->initialized) {
		for (int i = 0; i < audio_block_size; i++) {
			audio_out[i] = audio_in[i];
		}
		return;
	}

	// Bring state variables into local variables
	int i;
	float current_gain = c->current_gain;
	float delta = c->delta;
	uint32_t remaining_steps = c->remaining_steps;

	for (i = 0; i < audio_block_size; i++) {
		audio_out[i] = audio_in[i] * current_gain;

		if (remaining_steps) {
			current_gain += delta;
			remaining_steps--;
		}
	}

	// store state variables back into struct
	c->current_gain = current_gain;
	c->remaining_steps = remaining_steps;

}
