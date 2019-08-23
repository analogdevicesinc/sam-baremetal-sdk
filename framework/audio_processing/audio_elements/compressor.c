/*
 * Copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 *
 * A compressor attenuates signals that are louder than a certain volume
 * threshold.  They can also be used for amplifying signals that are below
 * a certain threshold.
 *
 * The following provides a nice summary of compressors, what they're
 * used for, and their parameters:
 * https://www.uaudio.com/blog/audio-compression-basics/
 *
 */
#include "compressor.h"
#include "audio_elements_common.h"

#include <math.h>
#include <stdlib.h>

// Min/max limits and other constants
#define     COMPRESSOR_MIN_THRESHOLD    (-100.0)
#define     COMPRESSOR_MAX_THRESHOLD    (30.0)
#define     COMPRESSOR_MIN_RATIO        (1.0)
#define     COMPRESSOR_MAX_RATIO        (100000.0)
#define     COMPRESSOR_MIN_ATTACK_MS    (0)
#define     COMPRESSOR_MAX_ATTACK_MS    (1000.0)
#define     COMPRESSOR_MIN_RELEASE_MS   (0)
#define     COMPRESSOR_MAX_RELEASE_MS   (1000.0)
#define     COMPRESSOR_MIN_GAIN         (0)
#define     COMPRESSOR_MAX_GAIN         (10.0)

// Static function prototypes
static float log2f(float x);
static float calculate_threshold_coeff(float threshold_db);
static float calculate_ratio_coeff(float ratio);
static LP_COEFF calculate_rms_coeffs(float rms_fc, float fs);
static LP_COEFF calculate_lp_coeffs(float timeconstant_ms, float fs);

/**
 * @brief Initializes instance of a compressor
 *
 * Here's a nice guide to what the various parameters of a compressor do:
 * https://www.uaudio.com/blog/audio-compression-basics/
 *
 * @param c Pointer to instance structure
 * @param threshold_db The threshold at which the audio compression is applied
 * @param ratio The ratio of compression to loudness (>=1.0)
 * @param attack_ms The amount of time after the signal crosses the threshold to when compression is applied
 * @param release_ms The amount of time the compression is held after the signal returns below threshold
 * @param output_gain The output gain of the compressor
 * @param audio_sample_rate The system audio sample rate
 * @return Compressor result (enumeration)
 */
RESULT_COMPRESSOR compressor_setup(COMPRESSOR * c, float threshold_db,
		float ratio, float attack_ms, float release_ms, float output_gain,
		float audio_sample_rate) {

	if (c == NULL) {
		return COMPRESSOR_INVALID_INSTANCE_POINTER;
	}

	c->initialized = false;

	// Set compressor threshold
	if (threshold_db > COMPRESSOR_MAX_THRESHOLD
			|| threshold_db < COMPRESSOR_MIN_THRESHOLD) {
		return COMPRESSOR_INVALID_THRESHOLD;
	}
	c->threshold_db = threshold_db;
	c->threshold_coeff = calculate_threshold_coeff(threshold_db);

	// Set compressor ratio
	if (ratio > COMPRESSOR_MAX_RATIO || ratio < COMPRESSOR_MIN_RATIO) {
		return COMPRESSOR_INVALID_RATIO;
	}
	c->ratio = ratio;
	c->ratio_coeff = calculate_ratio_coeff(ratio);

	// Set compressor attack time
	if (attack_ms > COMPRESSOR_MAX_ATTACK_MS
			|| attack_ms < COMPRESSOR_MIN_ATTACK_MS) {
		return COMPRESSOR_INVALID_ATTACK;
	}
	c->attack_ms = attack_ms;
	c->attack_coeff = calculate_lp_coeffs(attack_ms, audio_sample_rate);

	// Set compressor release time
	if (release_ms > COMPRESSOR_MAX_RELEASE_MS
			|| release_ms < COMPRESSOR_MIN_RELEASE_MS) {
		return COMPRESSOR_INVALID_RELEASE;
	}
	c->release_ms = release_ms;
	c->release_coeff = calculate_lp_coeffs(release_ms, audio_sample_rate);

	// Set RMS coefficient for 100ms
	c->rms_coeff = calculate_rms_coeffs(100.0, audio_sample_rate);

	// Set output gain
	if (output_gain > COMPRESSOR_MAX_GAIN || output_gain < COMPRESSOR_MIN_GAIN) {
		return COMPRESSOR_INVALID_GAIN;
	}
	c->output_gain = output_gain;

	// Set sample rate
	c->audio_sample_rate = audio_sample_rate;

	// Initialize state variables
	c->x2_last = 0.0;
	c->x_ar_last = 0.0;

	// Instance was successfully initialized
	c->initialized = true;
	return COMPRESSOR_OK;

}

/**
 * @brief Modify the compression threshold
 *
 * If the input parameter is out of bounds, clip it to the corresponding min/max
 * and apply that value.  This function will return a flag indicating an
 * invalid input parameter was supplied but it won't disable the effect.
 *
 * @param c Pointer to instance structure
 * @param threshold_db_new Updated threshold value
 *
 * @return Compressor result (enumeration)
 */
RESULT_COMPRESSOR compressor_modify_threshold(COMPRESSOR * c,
		float threshold_db_new) {

	RESULT_COMPRESSOR res;

	float threshold_db;
	if (threshold_db_new > COMPRESSOR_MAX_THRESHOLD) {
		threshold_db = COMPRESSOR_MAX_THRESHOLD;
		res = COMPRESSOR_INVALID_THRESHOLD;
	} else if (threshold_db_new < COMPRESSOR_MIN_THRESHOLD) {
		threshold_db = COMPRESSOR_MIN_THRESHOLD;
		res = COMPRESSOR_INVALID_THRESHOLD;
	} else {
		threshold_db = threshold_db_new;
		res = COMPRESSOR_OK;
	}

	// If nothing has changed since last time we modified this parameter, return
	if (threshold_db == c->threshold_db_last) {
		return res;
	} else {
		c->threshold_db_last = threshold_db;
	}

	// Update parameters
	c->threshold_db = threshold_db;
	c->threshold_coeff = calculate_threshold_coeff(threshold_db);

	return res;

}

/**
 * @brief  Modify compression ratio
 *
 * The compression ratio should be greater than 1.0 and will be translated
 * to a fraction 1/compression ratio within the compression algorithm.
 *
 * If the input parameter is out of bounds, clip it to the corresponding min/max
 * and apply that value.  This function will return a flag indicating an
 * invalid input parameter was supplied but it won't disable the effect.
 *
 * @param c Pointer to instance structure
 * @param ratio_new Updated compression ratio.
 *
 * @return  Compressor result (enumeration)
 */
RESULT_COMPRESSOR compressor_modify_ratio(COMPRESSOR * c, float ratio_new) {

	RESULT_COMPRESSOR res;

	float ratio;
	if (ratio_new > COMPRESSOR_MAX_RATIO) {
		ratio = COMPRESSOR_MAX_RATIO;
		res = COMPRESSOR_INVALID_RATIO;
	} else if (ratio_new < COMPRESSOR_MIN_RATIO) {
		ratio = COMPRESSOR_MIN_RATIO;
		res = COMPRESSOR_INVALID_RATIO;
	} else {
		ratio = ratio_new;
		res = COMPRESSOR_OK;
	}

	// If nothing has changed since last time we modified this parameter, return
	if (ratio == c->ratio_last) {
		return res;
	} else {
		c->ratio_last = ratio;
	}

	// Update parameters
	c->ratio = ratio;
	c->ratio_coeff = calculate_ratio_coeff(ratio);

	return res;

}

/**
 * @brief Modify the attack time in ms
 *
 * If the input parameter is out of bounds, clip it to the corresponding min/max
 * and apply that value.  This function will return a flag indicating an
 * invalid input parameter was supplied but it won't disable the effect.
 *
 * @param c Pointer to instance structure
 * @param attack_ms_new Updated attack time in milliseconds
 *
 * @return Compressor result (enumeration)
 */
RESULT_COMPRESSOR compressor_modify_attack(COMPRESSOR * c, float attack_ms_new) {

	RESULT_COMPRESSOR res;

	float attack_ms;
	if (attack_ms_new > COMPRESSOR_MAX_ATTACK_MS) {
		attack_ms = COMPRESSOR_MAX_ATTACK_MS;
		res = COMPRESSOR_INVALID_ATTACK;
	} else if (attack_ms_new < COMPRESSOR_MIN_ATTACK_MS) {
		attack_ms = COMPRESSOR_MIN_ATTACK_MS;
		res = COMPRESSOR_INVALID_ATTACK;
	} else {
		attack_ms = attack_ms_new;
		res = COMPRESSOR_OK;
	}

	// If nothing has changed since last time we modified this parameter, return
	if (attack_ms == c->attack_ms_last) {
		return res;
	} else {
		c->attack_ms_last = attack_ms;
	}

	// Update parameters
	c->attack_ms = attack_ms;
	c->attack_coeff = calculate_lp_coeffs(attack_ms, c->audio_sample_rate);

	return res;

}

/**
 * @brief Modfiy the release time in ms
 *
 * If the input parameter is out of bounds, clip it to the corresponding min/max
 * and apply that value.  This function will return a flag indicating an
 * invalid input parameter was supplied but it won't disable the effect.
 *
 * @param c Pointer to instance structure
 * @param release_ms_new Updated release time in milliseconds
 *
 * @return Compressor result (enumeration)
 */
RESULT_COMPRESSOR compressor_modify_release(COMPRESSOR * c,
		float release_ms_new) {

	RESULT_COMPRESSOR res;

	float release_ms;
	if (release_ms_new > COMPRESSOR_MAX_RELEASE_MS) {
		release_ms = COMPRESSOR_MAX_RELEASE_MS;
		res = COMPRESSOR_INVALID_RELEASE;
	} else if (release_ms_new < COMPRESSOR_MIN_RELEASE_MS) {
		release_ms = COMPRESSOR_MIN_RELEASE_MS;
		res = COMPRESSOR_INVALID_RELEASE;
	} else {
		release_ms = release_ms_new;
		res = COMPRESSOR_OK;
	}

	// If nothing has changed since last time we modified this parameter, return
	if (release_ms == c->release_ms_last) {
		return res;
	} else {
		c->release_ms_last = release_ms;
	}

	// Update parameters
	c->release_ms = release_ms;
	c->release_coeff = calculate_lp_coeffs(release_ms, c->audio_sample_rate);

	return res;

}

/**
 * @brief Modify compressor output gain
 *
 * If the input parameter is out of bounds, clip it to the corresponding min/max
 * and apply that value.  This function will return a flag indicating an
 * invalid input parameter was supplied but it won't disable the effect.
 *
 * @param c Pointer to instance structure
 * @param gain_new Updated output gain value
 *
 * @return Compressor result (enumeration)
 */
RESULT_COMPRESSOR compressor_modify_gain(COMPRESSOR * c, float gain_new) {

	RESULT_COMPRESSOR res;

	float gain;
	if (gain_new > COMPRESSOR_MAX_GAIN) {
		gain = COMPRESSOR_MAX_GAIN;
		res = COMPRESSOR_INVALID_RELEASE;
	} else if (gain_new < COMPRESSOR_MIN_GAIN) {
		gain = COMPRESSOR_MIN_GAIN;
		res = COMPRESSOR_INVALID_RELEASE;
	} else {
		gain = gain_new;
		res = COMPRESSOR_OK;
	}

	// Update parameters
	c->output_gain = gain;

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
void compressor_read(COMPRESSOR * c, float * audio_in, float * audio_out,
		uint32_t audio_block_size) {

	// If this instance hasn't been properly initialized, pass audio through
	if (c == NULL || !c->initialized) {
		for (int i = 0; i < audio_block_size; i++) {
			audio_out[i] = audio_in[i];
		}
		return;
	}

	float x2_last = c->x2_last;
	float x_ar_last = c->x_ar_last;

	float rms_ff = c->rms_coeff.ff;
	float rms_fb = c->rms_coeff.fb;

	for (int i = 0; i < audio_block_size; i++) {
		float x = audio_in[i];

		// Calculate current signal RMS
		float x2 = x * x;
		float x2_lpf = rms_ff * x2 + rms_fb * x2_last;
		x2_last = x2;
		float x_rms = 0.5 * log2f(x2_lpf);

		// Calculate and apply vca
		float x_thresh = c->threshold_coeff - x_rms;
		if (x_thresh > 0.0) {
			x_thresh = 0.0;
		}
		float x_ratio = c->ratio_coeff * x_thresh;

		float ff, fb;
		if (x_ar_last < x_ratio) {
			ff = c->release_coeff.ff;
			fb = c->release_coeff.fb;
		} else {
			ff = c->attack_coeff.ff;
			fb = c->attack_coeff.fb;
		}
		float x_ar = ff * x_ratio + fb * x_ar_last;
		x_ar_last = x_ar;

		float vca_coeff = powf(2.0, x_ar);

		audio_out[i] = x * vca_coeff * c->output_gain;

	}

	// Save state variables for next time through
	c->x2_last = x2_last;
	c->x_ar_last = x_ar_last;

}

/**
 * @brief Calculates log2(x)
 *
 * @param x input value
 * @return log2(input)
 */
static float log2f(float x) {
	float log10_2_recip = 1.0 / 0.301029995663981;
	return log10f(x) * log10_2_recip;
}

/**
 * @brief Calculates LP coefficent for threshold
 *
 * @param threshold_db Target threshold
 * @return Coefficent
 */
static float calculate_threshold_coeff(float threshold_db) {
	return log2f(powf(10.0, threshold_db / 20.0));
}

/**
 * @brief Calculates the ratio coefficient
 *
 * @param ratio Ratio value
 * @return Ratio coefficient
 */
static float calculate_ratio_coeff(float ratio) {
	return 1.0 - 1.0 / ratio;
}

/**
 * @brief Calculates the RMS LPF coefficient
 *
 * @param rms_fc RMS constant
 * @param fs Audio sample rate
 *
 * @return Coefficient
 */
static LP_COEFF calculate_rms_coeffs(float rms_fc, float fs) {
	LP_COEFF coeffs;

	coeffs.fb = expf(-PI2 * rms_fc / fs);
	coeffs.ff = 1.0 - coeffs.fb;

	return coeffs;
}

/**
 * @brief Calculates attack / release coefficent
 *
 * @param timeconstant_ms   Time constant in milliseconds
 * @param fs Audio sample rate
 *
 * @return Coefficient
 */
static LP_COEFF calculate_lp_coeffs(float timeconstant_ms, float fs) {
	LP_COEFF coeffs;

	coeffs.fb = expf(-3.0 / (1e-3 * timeconstant_ms * fs));
	coeffs.ff = 1.0 - coeffs.fb;

	return coeffs;
}
