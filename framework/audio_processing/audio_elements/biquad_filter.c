/*
 * Copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 *
 * This audio element implements a biquad filter (IIR).  A biquad filter is
 * essentially a second-order filter with a pair of poles and zeroes.  Biquads
 * filters are computationally efficient and cn be used to realize various
 * types of filters (low-pass, high-pass, etc.).
 *
 * More information on biquad filters can be found here:
 * https://en.wikipedia.org/wiki/Digital_biquad_filter
 * http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt
 */
#include "biquad_filter.h"

#include <filter.h>
#include <stdlib.h>

// Min/max limits and other constants
#define BIQUAD_MIN_Q        (0.01)
#define BIQUAD_MAX_Q        (100.0)
#define BIQUAD_MIN_FREQ     (10.0)
#define BIQUAD_MAX_FREQ     (20000.0)
#define BIQUAD_GAIN_MIN     (-100.0)
#define BIQUAD_GAIN_MAX     (100.0)

// Static function prototypes
static RESULT_BIQUAD filter_generate_coeffs(BIQUAD_FILTER_TYPE filter_type,
		float freq, float q, float gain_db, float audio_sample_rate,
		float * result);
static RESULT_BIQUAD convert_coeffs(float * coeffs_ab, float * sos_coeffs,
		float * scaling_factor);
static void filter_transition_coeffs(BIQUAD_FILTER * c);

/**
 * @brief Initializes instance of a biquad filter
 *
 * @param c Pointer to instance structure
 * @param type Type of filter (see enum in .h file)
 * @param transition_speed Speed to transition coefficents (see enum in .h file)
 * @param sos_coeffs Buffer for coefficents preferably in pm ram (e.g. float pm my_coeffs[4])
 * @param freq Cutoff/center frequency of filter
 * @param q Q factor of filter
 * @param gain_db Gain of the filter
 * @param audio_sample_rate Sampling frequency of system
 * @return Biquad result (enumeration)
 */
RESULT_BIQUAD filter_setup(BIQUAD_FILTER * c, BIQUAD_FILTER_TYPE type,
		BIQUAD_FILTER_TRANSITION_SPEED transition_speed, float pm * sos_coeffs,
		float freq, float q, float gain_db, float audio_sample_rate) {

	if (c == NULL) {
		return BIQUAD_INVALID_INSTANCE_POINTER;
	}

	c->initialized = false;

	if (q < BIQUAD_MIN_Q || q > BIQUAD_MAX_Q) {
		return BIQUAD_INVALID_Q;
	}

	if (freq < BIQUAD_MIN_FREQ || freq > BIQUAD_MAX_FREQ) {
		return BIQUAD_INVALID_FREQ;
	}

	if (gain_db < BIQUAD_GAIN_MIN || gain_db > BIQUAD_GAIN_MAX) {
		return BIQUAD_INVALID_GAIN;
	}

	// Save filter and system parameters
	c->q = q;
	c->freq = freq;
	c->gain_db = gain_db;
	c->audio_sample_rate = audio_sample_rate;

	// Set pointer to coefficients
	c->sos_coeffs = sos_coeffs;

	// Set how quickly we can transition coefficients
	c->transition_speed = transition_speed;

	float coeffs[6], sos[4];

	// Geneate A/B filter coefficients
	filter_generate_coeffs(type, freq, q, gain_db, audio_sample_rate, coeffs);

	// Convert them into SOS notation for ADI CCES iir() routine
	convert_coeffs(coeffs, c->sos_coeffs, &c->scaling_factor);

	// Zero out filter state line
	for (int i = 0; i < 3; i++) {
		c->sos_state[i] = 0;
	}

	// Clear filter transition counter
	c->sos_coeffs_steps = 0;

	// Instance was successfully initialized
	c->initialized = true;
	return BIQUAD_OK;

}

/**
 * @brief Modify Q of current frequency
 *
 * @param c Pointer to instance structure
 * @param q New Q value
 */
RESULT_BIQUAD filter_modify_q(BIQUAD_FILTER * c, float new_q) {

	RESULT_BIQUAD res = BIQUAD_OK;

	float q;

	/**
	 * If the input parameter is out of bounds, clip it to the corresponding min/max
	 * and apply that value.  This function will return a flag indicating an
	 * invalid input parameter was supplied but it won't disable the effect.
	 */
	if (new_q > BIQUAD_MAX_Q) {
		q = BIQUAD_MAX_Q;
		res = BIQUAD_INVALID_Q;
	} else if (new_q < BIQUAD_MIN_Q) {
		q = BIQUAD_MIN_Q;
		res = BIQUAD_INVALID_Q;
	} else {
		q = new_q;
		res = BIQUAD_OK;
	}

	// If nothing has changed since last time we modified this parameter, return
	if (q == c->q_last) {
		return res;
	} else {
		c->q_last = q;
	}

	// Calculate parameters
	c->q_steps = c->transition_speed;
	float factor = 1.0 / (float) c->transition_speed;
	c->q_dest = q;
	c->q_inc = (c->q_dest - c->q) * factor;

	return res;

}

/**
 * @brief Modify cutoff/center frequency of current filter
 *
 * @param c Pointer to instance structure
 * @param freq New cutoff/center frequency in Hz
 */
RESULT_BIQUAD filter_modify_freq(BIQUAD_FILTER * c, float freq_new) {

	RESULT_BIQUAD res = BIQUAD_OK;

	float freq;

	/**
	 * If the input parameter is out of bounds, clip it to the corresponding min/max
	 * and apply that value.  This function will return a flag indicating an
	 * invalid input parameter was supplied but it won't disable the effect.
	 */
	if (freq_new > BIQUAD_MAX_FREQ) {
		freq_new = BIQUAD_MAX_FREQ;
		res = BIQUAD_INVALID_Q;
	} else if (freq_new < BIQUAD_MIN_FREQ) {
		freq = BIQUAD_MIN_FREQ;
		res = BIQUAD_INVALID_Q;
	} else {
		freq = freq_new;
		res = BIQUAD_OK;
	}

	// If nothing has changed since last time we modified this parameter, return
	if (freq == c->freq_last) {
		return res;
	} else {
		c->freq_last = freq;
	}

	// Calculate parameters
	c->freq_steps = c->transition_speed;
	float factor = 1.0 / (float) c->transition_speed;
	c->freq_dest = freq;
	c->freq_inc = (c->freq_dest - c->freq) * factor;

	return res;

}

/**
 * @brief Apply effect/process to a block of audio data
 *
 * @param c Pointer to instance structure
 * @param input Pointer to floating point audio input buffer (mono)
 * @param output Pointer to floating point audio output buffer (mono)
 * @param block_size The number of floating-point words to process in buffers
 */
#pragma optimize_for_speed
void filter_read(BIQUAD_FILTER * c, float * audio_in, float * audio_out,
		uint32_t audio_block_size) {

	// If this instance hasn't been properly initialized, pass audio through
	if (c == NULL || !c->initialized) {
		for (int i = 0; i < audio_block_size; i++) {
			audio_out[i] = audio_in[i];
		}
		return;
	}

	// If we need to transition the coefficients do so now
	if (c->freq_steps) {
		filter_transition_coeffs(c);
	}

	// Process iir filter
	iir(audio_in, audio_out, c->sos_coeffs, c->sos_state, audio_block_size, 1);

	// the CCES iir() routine requires that the output be scaled by the b[0] coeff
	for (int i = 0; i < audio_block_size; i++) {
		audio_out[i] *= c->scaling_factor;
	}
}

#define COEFF_B0    (0)
#define COEFF_B1    (1)
#define COEFF_B2    (2)
#define COEFF_A0    (3)
#define COEFF_A1    (4)
#define COEFF_A2    (5)

/**
 * @brief Calculates coefficients for biquad filters
 *
 * @param filter_type Type of filter (see enum in .h file)
 * @param freq Cutoff/center frequency in Hz
 * @param q Filter Q
 * @param gain_db Filter gain in dB
 * @param audio_sample_rate Sampling frequency of system
 * @param result Pointer to floating-point buffer where coefficients will be stored
 * @return Result enum - see .h file for details
 */
static RESULT_BIQUAD filter_generate_coeffs(BIQUAD_FILTER_TYPE filter_type,
		float freq, float q, float gain_db, float audio_sample_rate,
		float * result) {

	float omega = PI2 * freq / audio_sample_rate;

	float s_omega = sinf(omega);
	float c_omega = cosf(omega);
	float ncos2 = -2.0 * c_omega;
	float alpha = s_omega / (2.0 * q);

	if (filter_type <= (int) BIQUAD_TYPE_NOTCH) {
		switch (filter_type) {

		case BIQUAD_TYPE_LPF:
			result[COEFF_B0] = (1.0 - c_omega) * 0.5;
			result[COEFF_B1] = (1.0 - c_omega);
			result[COEFF_B2] = result[COEFF_B0];
			result[COEFF_A0] = (1.0 + alpha);
			result[COEFF_A1] = ncos2;
			result[COEFF_A2] = (1.0 - alpha);
			break;

		case BIQUAD_TYPE_HPF:
			result[COEFF_B0] = (1.0 + c_omega) * 0.5;
			result[COEFF_B1] = -(1.0 + c_omega);
			result[COEFF_B2] = result[COEFF_B0];
			result[COEFF_A0] = (1.0 + alpha);
			result[COEFF_A1] = ncos2;
			result[COEFF_A2] = (1.0 - alpha);
			break;

		case BIQUAD_TYPE_BPF:
			result[COEFF_B0] = (alpha);
			result[COEFF_B1] = 0;
			result[COEFF_B2] = (-result[COEFF_B0]);
			result[COEFF_A0] = (1.0 + alpha);
			result[COEFF_A1] = ncos2;
			result[COEFF_A2] = (1.0 - alpha);
			break;

		case BIQUAD_TYPE_NOTCH:
			result[COEFF_B0] = (1.0);
			result[COEFF_B1] = -2.0 * c_omega;
			result[COEFF_B2] = (1.0);
			result[COEFF_A0] = (1.0 + alpha);
			result[COEFF_A1] = ncos2;
			result[COEFF_A2] = (1.0 - alpha);
			break;
		}
	} else {

		float A = powf(10.0, gain_db * 1.0 / 40.0);
		float sqrt_a_2 = 2.0 * sqrt(A);

		switch (filter_type) {
		case BIQUAD_TYPE_PEAKING:
			result[COEFF_B0] = 1.0 + alpha * A;
			result[COEFF_B1] = ncos2;
			result[COEFF_B2] = 1.0 - alpha * A;
			result[COEFF_A0] = 1.0 + alpha / A;
			result[COEFF_A1] = ncos2;
			result[COEFF_A2] = 1.0 - alpha / A;
			break;

		case BIQUAD_TYPE_L_SHELF:
			result[COEFF_B0] = A
					* ((A + 1) - (A - 1) * c_omega + sqrt_a_2 * alpha);
			result[COEFF_B1] = 2.0 * A * ((A - 1) - (A + 1) * c_omega);
			result[COEFF_B2] = A
					* ((A + 1) - (A - 1) * c_omega - sqrt_a_2 * alpha);
			result[COEFF_A0] = ((A + 1) + (A - 1) * c_omega + sqrt_a_2 * alpha);
			result[COEFF_A1] = -2.0 * ((A - 1) + (A + 1) * c_omega);
			result[COEFF_A2] = ((A + 1) + (A - 1) * c_omega - sqrt_a_2 * alpha);
			break;

		case BIQUAD_TYPE_H_SHELF:
			result[COEFF_B0] = A
					* ((A + 1) + (A - 1) * c_omega + sqrt_a_2 * alpha);
			result[COEFF_B1] = 2.0 * A * ((A - 1) + (A + 1) * c_omega);
			result[COEFF_B2] = A
					* ((A + 1) + (A - 1) * c_omega - sqrt_a_2 * alpha);
			result[COEFF_A0] = ((A + 1) - (A - 1) * c_omega + sqrt_a_2 * alpha);
			result[COEFF_A1] = -2.0 * ((A - 1) - (A + 1) * c_omega);
			result[COEFF_A2] = ((A + 1) - (A - 1) * c_omega - sqrt_a_2 * alpha);
			break;

		}
	}

	return BIQUAD_OK;

}

/**
 * @brief Convert our A and B coefficients into the format required by the
 * CCES iir() routine.
 *
 * @param coeffs_ab Coefficients in A/B format
 * @param sos_coeffs Coefficients scaled/negated/reversed for CCES routine
 * @param scaling_factor Scaling factor
 * @return Result enum - see .h file for details
 */
static RESULT_BIQUAD convert_coeffs(float * coeffs_ab, float * sos_coeffs,
		float * scaling_factor) {

	coeffs_ab[COEFF_B1] = coeffs_ab[COEFF_B1] / coeffs_ab[COEFF_B0];
	coeffs_ab[COEFF_B2] = coeffs_ab[COEFF_B2] / coeffs_ab[COEFF_B0];

	coeffs_ab[COEFF_A1] = -coeffs_ab[COEFF_A1] / coeffs_ab[COEFF_A0];
	coeffs_ab[COEFF_A2] = -coeffs_ab[COEFF_A2] / coeffs_ab[COEFF_A0];

	sos_coeffs[0] = coeffs_ab[COEFF_A2];
	sos_coeffs[1] = coeffs_ab[COEFF_A1];
	sos_coeffs[2] = coeffs_ab[COEFF_B2];
	sos_coeffs[3] = coeffs_ab[COEFF_B1];

	(*scaling_factor) = coeffs_ab[COEFF_B0];

	return BIQUAD_OK;

}

/**
 * @brief Transition coefficients when we're dynamically changing filter attributes
 *
 * When we set a new set of filter parameters (frequency, gain, Q), the coefficients
 * need to be changed gradually.  A sudden change in coefficients can cause an IIR
 * filter to become unstable.
 *
 * @param c Pointer to instance structure
 */
void filter_transition_coeffs(BIQUAD_FILTER * c) {

	// Check to see if we need to update coefficients
	bool update_coeffs = false;

	if (c->freq_steps) {
		c->freq_steps--;
		c->freq += c->freq_inc;
		update_coeffs = true;

	} else {
		c->freq_inc = 0.0;
	}

	if (c->q_steps) {
		c->q_steps--;
		c->q += c->q_inc;
		update_coeffs = true;

	} else {
		c->q_inc = 0.0;
	}

	// If so, generate transition coefficients and write them to our instance C struct
	if (update_coeffs) {

		float coeffs_ab[6];

		// Generate A/B filter coefficients
		filter_generate_coeffs(c->filter_type, c->freq, c->q, c->gain_db,
				c->audio_sample_rate, coeffs_ab);

		// Convert them into SOS notation for ADI CCES iir() routine
		convert_coeffs(coeffs_ab, c->sos_coeffs, &c->scaling_factor);
	}
}

