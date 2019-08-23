/*
 * copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 *
 * A multiband compressor divides the incoming audio into different
 * freqency bands using a series of filters and applies compression to
 * each band independently.  Thus each band of audio can be compressed
 * using unique compression parameters.
 *
 * This implementaton is comprised of a low-pass and high-pass with the
 * same cutoff frequency this the input signal is split into two bands.
 * The output of each filter feeds into an independent compressor with
 * its own state and parameters.
 *
 * This audio effect also serves as an example of how to utilize the
 * biquad filter and compressor audio elements.
 *
 */

#include <stdlib.h>

#include "effect_multiband_compressor.h"

// Min/max limits and other constants
#define MULTIBAND_COMP_CROSSOVER_MIN    (100.0)
#define MULTIBAND_COMP_CROSSOVER_MAX    (800.0)
#define MULTIBAND_COMP_GAIN_MIN         (0.1)
#define MULTIBAND_COMP_GAIN_MAX         (5.0)
#define MULTIBAND_COMP_THRESHOLD_MIN    (-100.0)
#define MULTIBAND_COMP_THRESHOLD_MAX    (30.0)

/**
 * @brief Initializes instance of a multiband compressor
 *
 * @param c Pointer to instance structure
 * @param cross_over Crossover frequency between LPF and HPF (100.0->800.0)
 * @param threshold Compressor threshold
 * @param audio_sample_rate The system audio sample rate
 * @return multiband compressor result (enumeration)
 */
RESULT_MULTIBAND_COMP multiband_comp_setup(MULTIBAND_COMPRESSOR * c,
		float cross_over, float threshold, float audio_sample_rate) {

	if (c == NULL) {
		return MULTIBAND_COMP_INVALID_INSTANCE_POINTER;
	}

	c->initialized = false;

	if (cross_over < MULTIBAND_COMP_CROSSOVER_MIN
			|| cross_over > MULTIBAND_COMP_CROSSOVER_MAX) {
		return MULTIBAND_COMP_INVALID_CROSSOVER_FREQ;
	}

	if (threshold < MULTIBAND_COMP_THRESHOLD_MIN
			|| threshold > MULTIBAND_COMP_THRESHOLD_MAX) {
		return MULTIBAND_COMP_INVALID_THRESHOLD;
	}

	// Change these gain settings to change the mix of low and high end
	c->gain_low = 1.4;
	c->gain_high = 1.0;

	// Initialize filters
	filter_setup(&c->lpf, BIQUAD_TYPE_LPF, BIQUAD_TRANS_MED,
			(pm float *) c->lpf_filter_coeffs, cross_over, 1.0, 1.0,
			audio_sample_rate);

	filter_setup(&c->hpf, BIQUAD_TYPE_HPF, BIQUAD_TRANS_MED,
			(pm float *) c->hpf_filter_coeffs, cross_over, 1.0, 1.0,
			audio_sample_rate);

	compressor_setup(&c->compressor_low, -30.0, 100.0, 100, 100, 2.0,
			audio_sample_rate);
	compressor_setup(&c->compressor_high, -35.0, 100.0, 50, 50, 2.2,
			audio_sample_rate);

	c->initialized = true;
	return MULTIBAND_COMP_OK;

}

/**
 * @brief Modify the cross over frequency between the LPF and HPF
 *
 * If the input parameter is out of bounds, it is clipped to the corresponding
 * min/max value.  This function will return a value indicating an
 * invalid input parameter was supplied but the effect will continue to operate.
 *
 * @param c Pointer to instance structure
 * @param crossover_freq_new New crossover frequency (100.0->800.0)
 *
 * @return multiband compressor result (enumeration)
 */
RESULT_MULTIBAND_COMP multiband_comp_change_xover(MULTIBAND_COMPRESSOR * c,
		float crossover_freq_new) {

	RESULT_MULTIBAND_COMP res;

	float crossover_freq;
	if (crossover_freq_new < MULTIBAND_COMP_CROSSOVER_MIN) {
		crossover_freq = MULTIBAND_COMP_CROSSOVER_MIN;
		res = MULTIBAND_COMP_INVALID_CROSSOVER_FREQ;
	} else if (crossover_freq_new > MULTIBAND_COMP_CROSSOVER_MAX) {
		crossover_freq = MULTIBAND_COMP_CROSSOVER_MAX;
		res = MULTIBAND_COMP_INVALID_CROSSOVER_FREQ;
	} else {
		crossover_freq = crossover_freq_new;
		res = MULTIBAND_COMP_OK;
	}

	// Update instance parameters
	filter_modify_freq(&c->lpf, crossover_freq);
	filter_modify_freq(&c->hpf, crossover_freq);

	return MULTIBAND_COMP_OK;
}

/**
 * @brief Modify multiband compressor threshold
 *
 * If the input parameter is out of bounds, it is clipped to the corresponding
 * min/max value.  This function will return a value indicating an
 * invalid input parameter was supplied but the effect will continue to operate.
 *
 * @param c Pointer to instance structure
 * @param threshold_db_new Updated threshold vale
 *
 * @return multiband compressor result (enumeration)
 */
RESULT_MULTIBAND_COMP multiband_comp_change_thresh(MULTIBAND_COMPRESSOR * c,
		float threshold_db_new) {

	RESULT_MULTIBAND_COMP res;

	float threshold_db;
	if (threshold_db_new < MULTIBAND_COMP_THRESHOLD_MIN) {
		threshold_db = MULTIBAND_COMP_THRESHOLD_MIN;
		res = MULTIBAND_COMP_INVALID_THRESHOLD;
	} else if (threshold_db_new > MULTIBAND_COMP_THRESHOLD_MAX) {
		threshold_db = MULTIBAND_COMP_THRESHOLD_MAX;
		res = MULTIBAND_COMP_INVALID_THRESHOLD;
	} else {
		threshold_db = threshold_db_new;
		res = MULTIBAND_COMP_OK;
	}

	// Update instance parameters
	compressor_modify_threshold(&c->compressor_low, threshold_db - 5.0);
	compressor_modify_threshold(&c->compressor_high, threshold_db);

	return res;
}

/**
 * @brief Modify multiband compressor output gain
 *
 * If the input parameter is out of bounds, it is clipped to the corresponding
 * min/max value.  This function will return a value indicating an
 * invalid input parameter was supplied but the effect will continue to operate.
 *
 * @param c Pointer to instance structure
 * @param gain_new Updated gain value
 *
 * @return multiband compressor result (enumeration)
 */
RESULT_MULTIBAND_COMP multiband_comp_change_gain(MULTIBAND_COMPRESSOR * c,
		float gain_new) {

	RESULT_MULTIBAND_COMP res;

	float gain;
	if (gain_new < MULTIBAND_COMP_GAIN_MIN) {
		gain = MULTIBAND_COMP_GAIN_MIN;
		res = MULTIBAND_COMP_INVALID_GAIN;
	} else if (gain_new > MULTIBAND_COMP_GAIN_MAX) {
		gain = MULTIBAND_COMP_GAIN_MAX;
		res = MULTIBAND_COMP_INVALID_GAIN;
	} else {
		gain = gain_new;
		res = MULTIBAND_COMP_OK;
	}

	// Update instance parameters
	compressor_modify_gain(&c->compressor_low, gain);
	compressor_modify_gain(&c->compressor_high, gain);

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
void multiband_comp_read(MULTIBAND_COMPRESSOR * c, float * audio_in,
		float * audio_out, uint32_t audio_block_size) {

	// If this instance hasn't been properly initialized, pass audio through
	if (c == NULL || !c->initialized) {
		for (int i = 0; i < audio_block_size; i++) {
			audio_out[i] = audio_in[i];
		}
		return;
	}

	float temp_audio_low[MAX_AUDIO_BLOCK_SIZE];
	float temp_audio_high[MAX_AUDIO_BLOCK_SIZE];

	// Apply lpf / hpf
	filter_read(&c->lpf, audio_in, temp_audio_low, audio_block_size);

	filter_read(&c->hpf, audio_in, temp_audio_high, audio_block_size);

	gain_buffer(temp_audio_low, c->gain_low, audio_block_size);
	gain_buffer(temp_audio_high, c->gain_high, audio_block_size);

	compressor_read(&c->compressor_low, temp_audio_low, temp_audio_low,
			audio_block_size);

	compressor_read(&c->compressor_high, temp_audio_high, temp_audio_high,
			audio_block_size);

	mix_2x1(temp_audio_low, temp_audio_high, audio_out, audio_block_size);

}
