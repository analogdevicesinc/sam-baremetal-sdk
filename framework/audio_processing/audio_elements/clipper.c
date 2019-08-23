/*
 * Copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 *
 * A clipper is used to apply a polynomial clipping function to an incoming signal
 * and is the core of many distortion and tube amplifier simulation algorithms.
 *
 * This implementation includes an optional upsampling / downsampling component that
 * can be used to eliminate the audio artifacts that can occur with clipping using
 * polynomial expansion.
 *
 */

#include <math.h>
#include <filter.h>
#include <stdlib.h>

#include "audio_utilities.h"
#include "clipper.h"

// Min/max limits and other constants
#define CLIPPER_INTERP_FACTOR       (8)
#define CLIPPER_MAX_THRESHOLD       (1.0)
#define CLIPPER_MIN_THRESHOLD       (0.001)

// Static function prototypes
static void upsample_signal(CLIPPER * c, float * audio_in, float * audio_out,
		uint32_t audio_block_size);

static void downsample_signal(CLIPPER * c, float * audio_in, float * audio_out,
		uint32_t audio_block_size);

static void polynomial_smoothstep(float clip_value, float * input,
		float * output, uint32_t audio_block_size);

static void polynomial_smootherstep(float clip_value, float * input,
		float * output, uint32_t audio_block_size);

/**
 * @brief Initializes instance of a clipper
 *
 * @param c Pointer to instance structure
 * @param threshold  Threshold where clipping will begin
 * @param poly_clip Which clipping function to use
 * @param upsample Whether to upsample / downsample on either side of clipping
 * @return Clipper result (enumeration)
 */
RESULT_CLIPPER clipper_setup(CLIPPER *c, float threshold,
		POLY_CLIP_FUNC poly_clip, bool upsample) {

	if (c == NULL) {
		return CLIPPER_INVALID_INSTANCE_POINTER;
	}

	c->initialized = false;

	if (threshold < CLIPPER_MIN_THRESHOLD || threshold > CLIPPER_MAX_THRESHOLD) {
		return CLIPPER_INVALID_THRESHOLD;
	}

	for (int i = 0; i < CLIPPER_INTERP_TAPS + 1; i++) {
		c->fir_upsample_state[i] = 0.0;
		c->fir_downsample_state[i] = 0.0;
	}

	// Set parameters
	c->clip_threshold = threshold;
	c->upsample = upsample;

	// Instance was successfully initialized
	c->initialized = true;
	return CLIPPER_OK;

}

/**
 * @brief Modify the threshold value of the clipper
 *
 * If the input parameter is out of bounds, clip it to the corresponding min/max
 * and apply that value.  This function will return a flag indicating an
 * invalid input parameter was supplied but it won't disable the effect.
 *
 * @param c Pointer to instance structure
 * @param threshold_new Updated threshold value
 *
 * @returnClipper result (enumeration)
 */
RESULT_CLIPPER modify_clipper_threshold(CLIPPER *c, float threshold_new) {

	RESULT_CLIPPER res;

	float threshold;

	if (threshold_new > CLIPPER_MAX_THRESHOLD) {
		threshold = CLIPPER_MAX_THRESHOLD;
		res = CLIPPER_INVALID_THRESHOLD;
	} else if (threshold_new < CLIPPER_MIN_THRESHOLD) {
		threshold = CLIPPER_MIN_THRESHOLD;
		res = CLIPPER_INVALID_THRESHOLD;
	} else {
		threshold = threshold_new;
		res = CLIPPER_OK;
	}

	// Update parameter
	c->clip_threshold = threshold;

	return res;
}

/**
 * @brief Apply effect/process to a block of audio data
 *
 * If the input parameter is out of bounds, clip it to the corresponding min/max
 * and apply that value.  This function will return a flag indicating an
 * invalid input parameter was supplied but it won't disable the effect.
 *
 * @param c Pointer to instance structure
 * @param audio_in Pointer to floating point audio input buffer (mono)
 * @param audio_out Pointer to floating point audio output buffer (mono)
 * @param audio_block_size The number of floating-point words to process
 */
void clipper_read(CLIPPER *c, float * audio_in, float * audio_out,
		uint32_t audio_block_size) {

	// If this instance hasn't been properly initialized, pass audio through
	if (c == NULL || !c->initialized) {
		for (int i = 0; i < audio_block_size; i++) {
			audio_out[i] = audio_in[i];
		}
		return;
	}

	float clipper_read_temp[MAX_AUDIO_BLOCK_SIZE * CLIPPER_INTERP_FACTOR];
	int buffer_size_multipler = 1;

	if (c->upsample) {
		upsample_signal(c, audio_in, clipper_read_temp, audio_block_size);
		buffer_size_multipler = CLIPPER_INTERP_FACTOR;
	} else {
		copy_buffer(audio_in, clipper_read_temp, audio_block_size);
	}

	// Apply polynomial
	switch (c->poly_clip) {
	case POLY_SMOOTHSTEP:
		polynomial_smoothstep(c->clip_threshold, clipper_read_temp,
				clipper_read_temp, audio_block_size * buffer_size_multipler);
		break;

	case POLY_SMOOTHERSTEP:
		polynomial_smootherstep(c->clip_threshold, clipper_read_temp,
				clipper_read_temp, audio_block_size * buffer_size_multipler);
		break;
	default:
		break;
	}

	if (c->upsample) {
		downsample_signal(c, clipper_read_temp, audio_out, audio_block_size);
	} else {
		copy_buffer(clipper_read_temp, audio_out, audio_block_size);
	}

}

// Coefficients used for upsampling/downsampling
float pm fir_resample_x_8[CLIPPER_INTERP_TAPS] = { -3.88257917522e-19,
		-0.000718555656558, -0.00184171525988, -0.0035491808885,
		-0.00567312990492, -0.00757389494893, -0.00815933632458,
		-0.00606971771843, 2.62074094328e-18, 0.0109147410627, 0.026753283728,
		0.0466046240811, 0.0685805055399, 0.0900655377644, 0.108164525988,
		0.120252312056, 0.124500000963, 0.120252312056, 0.108164525988,
		0.0900655377644, 0.0685805055399, 0.0466046240811, 0.026753283728,
		0.0109147410627, 2.62074094328e-18, -0.00606971771843,
		-0.00815933632458, -0.00757389494893, -0.00567312990492,
		-0.0035491808885, -0.00184171525988, -0.000718555656558,
		-3.88257917522e-19 };

/**
 * @brief Simple upsampling function
 *
 * @param c Pointer to instance structure
 * @param audio_in Pointer to floating point audio input buffer (mono)
 * @param audio_out Pointer to floating point audio output buffer (mono)
 * @param audio_block_size The number of floating-point words to process
 */
#pragma optimize_for_speed
static void upsample_signal(CLIPPER * c, float * audio_in, float * audio_out,
		uint32_t audio_block_size) {

	int i;
	int indx = 0;

	// Upsample
	for (i = 0; i < audio_block_size; i++) {
		for (int j = 0; j < CLIPPER_INTERP_FACTOR; j++) {
			audio_out[indx++] = audio_in[i];
		}
	}

	// Filter
	fir(audio_out, audio_out, fir_resample_x_8, c->fir_upsample_state,
			audio_block_size * CLIPPER_INTERP_FACTOR,
			CLIPPER_INTERP_TAPS);
}

/**
 * @brief Simple downsampling function
 *
 * @param c Pointer to instance structure
 * @param audio_in Pointer to floating point audio input buffer (mono)
 * @param audio_out Pointer to floating point audio output buffer (mono)
 * @param audio_block_size The number of floating-point words to process
 */
#pragma optimize_for_speed
static void downsample_signal(CLIPPER * c, float * audio_in, float * audio_out,
		uint32_t audio_block_size) {

	// Filter
	fir(audio_in, audio_in, fir_resample_x_8, c->fir_downsample_state,
			audio_block_size * CLIPPER_INTERP_FACTOR,
			CLIPPER_INTERP_TAPS);

	int indx = 0;
	for (int i = 0; i < audio_block_size * CLIPPER_INTERP_FACTOR; i +=
			CLIPPER_INTERP_FACTOR) {
		audio_out[indx++] = audio_in[i];
	}
}

/**
 * @brief Smoothstep polynomial
 *
 * https://en.wikipedia.org/wiki/Smoothstep
 *
 * @param clip_value Threshold where clipping will occur (0.0->1.0)
 * @param input Input signal
 * @param output Clipped output signal
 * @param audio_block_size The number of floating-point words to process
 */
#pragma optimize_for_speed
static void polynomial_smoothstep(float clip_value, float * input,
		float * output, uint32_t audio_block_size) {

	for (int i = 0; i < audio_block_size; i++) {

		// Scale input so 1.0 = our clip value
		float x = input[i] / clip_value;

		// Shift input from 0.0 -> 1.0
		x = x * 0.5 + 0.5;

		if (x > 1.0)
			x = 1.0;
		else if (x < 0)
			x = 0.0;
		else {

			// Apply smoothstep polynomial
			x = x * x * (3.0 - 2.0 * x);
		}

		// unscale input
		x = 2.0 * x - 1.0;
		x = x * clip_value;

		output[i] = x;
	}
}

/**
 * @brief Smootherstep polynomial
 *
 * Ken Perlin's smoother step
 * https://en.wikipedia.org/wiki/Smoothstep
 *
 * @param clip_value Threshold where clipping will occur (0.0->1.0)
 * @param input Input signal
 * @param output lipped output signals
 * @param audio_block_size The number of floating-point words to process
 */
#pragma optimize_for_speed
static void polynomial_smootherstep(float clip_value, float * input,
		float * output, uint32_t audio_block_size) {

	for (int i = 0; i < audio_block_size; i++) {

		// Scale input so 1.0 = our clip value
		float x = input[i] / clip_value;

		// Shift input from 0.0 -> 1.0
		x = x * 0.5 + 0.5;

		if (x > 1.0)
			x = 1.0;
		else if (x < -1.0)
			x = -1.0;
		else {
			// Apply smoothstep polynomial
			x = x * x * x * (x * (x * 6.0 - 15.0) + 10.0);
		}

		// unscale input
		x = 2.0 * x - 1.0;
		x = x * clip_value;

		output[i] = x;
	}
}

