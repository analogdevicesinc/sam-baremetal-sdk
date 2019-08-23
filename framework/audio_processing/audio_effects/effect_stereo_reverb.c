/*
 * copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 */

#include "effect_stereo_reverb.h"

// Min/max limits and other constants
#define     REVERB_WET_MIX_MIN   (0.0)
#define     REVERB_WET_MIX_MAX   (1.0)
#define     REVERB_DRY_MIX_MIN   (0.0)
#define     REVERB_DRY_MIX_MAX   (1.0)
#define     REVERB_FEEDBACK_MIN  (0.0)
#define     REVERB_FEEDBACK_MAX  (1.0)
#define     REVERB_LP_DAMP_MIN   (0.0)
#define     REVERB_LP_DAMP_MAX   (1.0)

/**
 * @brief Initializes instance of a stereo reverb
 *
 * @param c Pointer to instance structure
 * @param wet_mix Mix of processed (reverb) audio (0.0->1.0)
 * @param dry_mix Mix of unprocessed audio (0.0->1.0)
 * @param feedback Feedback in delay lines (0.0->1.0)
 * @param lp_damp Lowpass dampening coefficent (0.0->1.0)
 * @return Reverb result (enumerated)
 */
RESULT_STEREO_REVERB reverb_setup(STEREO_REVERB * c, float wet_mix,
		float dry_mix, float feedback, float lp_damp) {

	if (c == NULL) {
		return REVERB_INVALID_INSTANCE_POINTER;
	}

	c->initialized = false;

	// Modify these delay lenghts to change the characteristics of the reverb
	uint32_t delay_lens_left[8] = { 1557, 1617, 1491, 1422, 1277, 1356, 1118,
			1116 };
	uint32_t allpass_left[4] = { 225, 556, 441, 341 };
	uint32_t delay_lens_right[8] = { 1551, 1593, 1463, 1433, 1252, 1372, 1101,
			1105 };
	uint32_t allpass_right[4] = { 228, 546, 431, 321 };

	for (int i = 0; i < REVERB_ALLPASS_ELEMENTS; i++) {
		allpass_setup(&c->allpass_outputs_left[i], c->allpass_buffers_left[i],
				allpass_left[i], 0.5);
		allpass_setup(&c->allpass_outputs_right[i], c->allpass_buffers_right[i],
				allpass_right[i], 0.5);
	}

	for (int i = 0; i < REVERB_DELAY_ELEMENTS; i++) {
		delay_setup(&c->lpcf_left[i], &c->delay_buffers_left[i][0],
				REVERB_MAX_DELAY_SIZE, delay_lens_left[i], feedback, 0.0,
				lp_damp);
		delay_setup(&c->lpcf_right[i], &c->delay_buffers_right[i][0],
				REVERB_MAX_DELAY_SIZE, delay_lens_right[i], feedback, 0.0,
				lp_damp);
	}

	c->dry_mix = dry_mix;
	c->wet_mix = wet_mix;
	c->lp_damp = lp_damp;
	c->feedback = feedback;

	// Instance was successfully initialized
	c->initialized = true;
	return REVERB_OK;

}

/**
 * @brief Modify reverb wet (processed) mix
 *
 * If the input parameter is out of bounds, it is clipped to the corresponding
 * min/max value.  This function will return a value indicating an
 * invalid input parameter was supplied but the effect will continue to operate.
 *
 * @param c Pointer to instance structure
 * @param new_wet_mix New wet mix value (0.0->1.0)
 * @return Reverb result (enumerated)
 */
RESULT_STEREO_REVERB reverb_change_wet_mix(STEREO_REVERB * c, float wet_mix_new) {

	RESULT_STEREO_REVERB res;

	float wet_mix;
	if (wet_mix_new < REVERB_WET_MIX_MIN) {
		wet_mix = REVERB_WET_MIX_MIN;
		res = REVERB_INVALID_WET_MIX;
	} else if (wet_mix_new > REVERB_WET_MIX_MAX) {
		wet_mix = REVERB_WET_MIX_MAX;
		res = REVERB_INVALID_WET_MIX;
	} else {
		wet_mix = wet_mix_new;
		res = REVERB_OK;
	}

	// Update instance parameters
	c->wet_mix = wet_mix;

	return res;
}

/**
 * @brief Modify reverb dry (unprocessed) mix
 *
 * If the input parameter is out of bounds, it is clipped to the corresponding
 * min/max value.  This function will return a value indicating an
 * invalid input parameter was supplied but the effect will continue to operate.
 *
 * @param c Pointer to instance structure
 * @param new_dry_mix New dry mix value (0.0->1.0)
 * @return Reverb result (enumerated)
 */
RESULT_STEREO_REVERB reverb_change_dry_mix(STEREO_REVERB * c, float dry_mix_new) {

	RESULT_STEREO_REVERB res;

	float dry_mix;
	if (dry_mix_new < REVERB_DRY_MIX_MIN) {
		dry_mix = REVERB_DRY_MIX_MIN;
		res = REVERB_INVALID_DRY_MIX;
	} else if (dry_mix_new > REVERB_DRY_MIX_MAX) {
		dry_mix = REVERB_DRY_MIX_MAX;
		res = REVERB_INVALID_DRY_MIX;
	} else {
		dry_mix = dry_mix_new;
		res = REVERB_OK;
	}

	// Update instance parameters
	c->dry_mix = dry_mix;

	return res;
}

/**
 * @brief Modify reverb feedback
 *
 * If the input parameter is out of bounds, it is clipped to the corresponding
 * min/max value.  This function will return a value indicating an
 * invalid input parameter was supplied but the effect will continue to operate.
 *
 * @param c Pointer to instance structure
 * @param new_feedback New feedback value (0.0->1.0)
 * @return Reverb result (enumerated)
 */
RESULT_STEREO_REVERB reverb_change_feedback(STEREO_REVERB * c,
		float feedback_new) {

	RESULT_STEREO_REVERB res;

	float feedback;
	if (feedback_new < REVERB_FEEDBACK_MIN) {
		feedback = REVERB_FEEDBACK_MIN;
		res = REVERB_INVALID_FEEDBACK;
	} else if (feedback_new > REVERB_FEEDBACK_MAX) {
		feedback = REVERB_FEEDBACK_MAX;
		res = REVERB_INVALID_FEEDBACK;
	} else {
		feedback = feedback_new;
		res = REVERB_OK;
	}

	// Update instance parameters
	for (int i = 0; i < REVERB_DELAY_ELEMENTS; i++) {
		delay_modify_feedback(&c->lpcf_left[i], feedback);
		delay_modify_feedback(&c->lpcf_right[i], feedback);
	}
	c->feedback = feedback;

	return res;

}

/**
 * @brief Modify reverb dampening
 *
 * If the input parameter is out of bounds, it is clipped to the corresponding
 * min/max value.  This function will return a value indicating an
 * invalid input parameter was supplied but the effect will continue to operate.
 *
 * @param c Pointer to instance structure
 * @param new_lp_damp New low-pass dampening value (0.0->1.0); higher is more dampening
 * @return Reverb result (enumerated)
 */
RESULT_STEREO_REVERB reverb_change_lp_damp_coeff(STEREO_REVERB * c,
		float lp_damp_new) {

	RESULT_STEREO_REVERB res;

	float lp_damp;
	if (lp_damp_new < REVERB_LP_DAMP_MIN) {
		lp_damp = REVERB_LP_DAMP_MIN;
		res = REVERB_INVALID_LP_DAMP;
	} else if (lp_damp_new > REVERB_LP_DAMP_MAX) {
		lp_damp = REVERB_LP_DAMP_MAX;
		res = REVERB_INVALID_LP_DAMP;
	} else {
		lp_damp = lp_damp_new;
		res = REVERB_OK;
	}

	// Update instance parameters
	lp_damp = 1.0 - lp_damp;
	lp_damp = (lp_damp * 0.4) + 0.1;

	for (int i = 0; i < REVERB_DELAY_ELEMENTS; i++) {
		delay_modify_dampening(&c->lpcf_left[i], lp_damp);
		delay_modify_dampening(&c->lpcf_right[i], lp_damp);
	}
	c->lp_damp = lp_damp;

	return res;

}

/**
 * @brief Apply effect/process to a block of audio data
 *
 * @param c Pointer to instance structure
 * @param audio_in Pointer to floating point audio input buffer (mono)
 * @param audio_out_left Pointer to floating point output buffer (mono left)
 * @param audio_out_right Pointer to floating point output buffer (mono right)
 * @param audio_block_size The number of floating-point words to process
 */
void reverb_read(STEREO_REVERB * c, float * audio_in, float * audio_out_left,
		float * audio_out_right, uint32_t audio_block_size) {

	// If this instance hasn't been properly initialized, pass audio through
	if (c == NULL || !c->initialized) {
		for (int i = 0; i < audio_block_size; i++) {
			audio_out_left[i] = audio_in[i];
			audio_out_right[i] = audio_in[i];
		}
		return;
	}

	int i;
	float temp_audio1[MAX_AUDIO_BLOCK_SIZE], temp_audio2[MAX_AUDIO_BLOCK_SIZE];

	clear_buffer(temp_audio1, audio_block_size);
	for (int i = 0; i < REVERB_DELAY_ELEMENTS; i++) {
		delay_read(&c->lpcf_left[i], audio_in, temp_audio2, audio_block_size);
		mix_2x1(temp_audio1, temp_audio2, temp_audio1, audio_block_size);
	}

	// run through all-pass filters
	for (int i = 0; i < REVERB_ALLPASS_ELEMENTS; i++) {
		allpass_read(&c->allpass_outputs_left[i], temp_audio1, temp_audio1,
				audio_block_size);
	}
	mix_2x1_gain(temp_audio1, c->wet_mix * (1.0 / (2 * REVERB_DELAY_ELEMENTS)),
			audio_in, c->dry_mix, audio_out_left, audio_block_size);

	clear_buffer(temp_audio1, audio_block_size);
	for (int i = 0; i < REVERB_DELAY_ELEMENTS; i++) {
		delay_read(&c->lpcf_right[i], audio_in, temp_audio2, audio_block_size);
		mix_2x1(temp_audio1, temp_audio2, temp_audio1, audio_block_size);
	}

	// run through all-pass filters
	for (int i = 0; i < REVERB_ALLPASS_ELEMENTS; i++) {
		allpass_read(&c->allpass_outputs_right[i], temp_audio1, temp_audio1,
				audio_block_size);
	}
	mix_2x1_gain(temp_audio1, c->wet_mix * (1.0 / (2 * REVERB_DELAY_ELEMENTS)),
			audio_in, c->dry_mix, audio_out_right, audio_block_size);

}
