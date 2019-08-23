/*
 * copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 *
 * An autowah uses the amplitude of the input signal to position /
 * sweep a bandpass filter (similar to a wah) pedal. When the
 * amplitude of the input signal increase as a new note is played,
 * the frequency of the filter is swept to a higher frequency around
 * 800-1000Hz and as the note decays, the filter is swept towards
 * a lower frequnecy.
 *
 * This audio effect also serves as an example of how to utilize the
 * biquad filter audio element.
 *
 */
#include <stdlib.h>

#include "effect_autowah.h"
#include "../audio_elements/audio_utilities.h"

// Min/max limits and other constants
#define  AUTOWAH_DEPTH_MIN      (0.0)
#define  AUTOWAH_DEPTH_MAX      (1.0)
#define  AUTOWAH_DECAY_MIN      (0.0)
#define  AUTOWAH_DECAY_MAX      (1.0)
#define  AUTOWAH_Q_MIN          (0.0)
#define  AUTOWAH_Q_MAX          (1.0)
#define  AUTOWAH_MAX_BF_FREQ    (800.0)

/**
 * @brief Initializes instance of an autowah
 *
 * @param c Pointer to instance structure
 * @param depth How far the frequency sweep (0.0-1.0)
 * @param decay How quickly the ampitude decays following a note event
 * @param audio_sample_rate The system audio sample rate
 * @return Autowah result (enumeration)
 */
RESULT_AUTOWAH autowah_setup(AUTOWAH * c, float depth, float decay,
		float audio_sample_rate) {

	if (c == NULL) {
		return AUTOWAH_INVALID_INSTANCE_POINTER;
	}

	c->initialized = false;

	if (depth < AUTOWAH_DEPTH_MIN || depth > AUTOWAH_DEPTH_MAX) {
		return AUTOWAH_INVALID_DEPTH;
	}

	if (decay < AUTOWAH_DECAY_MIN || decay > AUTOWAH_DECAY_MAX) {
		return AUTOWAH_INVALID_DECAY;
	}

	filter_setup(&c->bpf1, BIQUAD_TYPE_BPF, BIQUAD_TRANS_MED,
			(pm float *) c->bpf_coeffs1, 400.0, 2.0, 1.0, audio_sample_rate);

	filter_setup(&c->bpf2, BIQUAD_TYPE_BPF, BIQUAD_TRANS_MED,
			(pm float *) c->bpf_coeffs2, 400.0, 2.0, 1.0, audio_sample_rate);

	filter_setup(&c->bpf3, BIQUAD_TYPE_BPF, BIQUAD_TRANS_MED,
			(pm float *) c->bpf_coeffs2, 400.0, 2.0, 1.0, audio_sample_rate);

	c->depth = 1000.0 * depth;
	c->decay = 0.999 + (0.001 * decay);

	c->initialized = true;
	return AUTOWAH_OK;
}

/**
 * @brief Modify decay parameter
 *
 * The decay parameter determines how quickly voltage envelop decays
 * towards zero.  A higher decay value results in a slower decay.
 *
 * If the input parameter is out of bounds, it is clipped to the corresponding
 * min/max value.  This function will return a value indicating an
 * invalid input parameter was supplied but the effect will continue to operate.
 *
 * @param c Pointer to instance structure
 * @param decay_new New decay value betweeen (0.0 and 1.0)
 *
 * @return Autowah result (enumeration)
 */
RESULT_AUTOWAH autowah_modify_decay(AUTOWAH * c, float decay_new) {

	RESULT_AUTOWAH res;

	float decay;
	if (decay_new > AUTOWAH_DECAY_MAX) {
		decay = AUTOWAH_DECAY_MAX;
		res = AUTOWAH_INVALID_DEPTH;
	} else if (decay_new < AUTOWAH_DECAY_MIN) {
		decay = AUTOWAH_DECAY_MIN;
		res = AUTOWAH_INVALID_DEPTH;
	} else {
		decay = decay_new;
		res = AUTOWAH_OK;
	}

	c->decay = 0.999 + (0.001 * decay);

	return res;

}

/**
 * @brief Modify depth parameter
 *
 * The depth parameter determines how far the bandpass filter is swept
 * for a given input amplitude value.  A higher value results in a greater
 * frequency range.
 *
 * If the input parameter is out of bounds, it is clipped to the corresponding
 * min/max value.  This function will return a value indicating an
 * invalid input parameter was supplied but the effect will continue to operate.
 *
 * @param c Pointer to instance structure
 * @param depth_new New depth value betweeen (0.0 and 1.0)
 *
 * @return Autowah result (enumeration)
 */
RESULT_AUTOWAH autowah_modify_depth(AUTOWAH * c, float depth_new) {

	RESULT_AUTOWAH res;

	float depth;
	if (depth_new > AUTOWAH_DEPTH_MAX) {
		depth = AUTOWAH_DEPTH_MAX;
		res = AUTOWAH_INVALID_DEPTH;
	} else if (depth_new < AUTOWAH_DEPTH_MIN) {
		depth = AUTOWAH_DEPTH_MIN;
		res = AUTOWAH_INVALID_DEPTH;
	} else {
		depth = depth_new;
		res = AUTOWAH_OK;
	}

	c->depth = 10000.0 * depth;

	return res;
}

/**
 * @brief Modify the width of the bandpass filter
 *
 * The q parameter determines how wide the bandpass filter is.  A higher value
 * results in a narrower filter.  A value of 0.0 results in a filter with a Q of 0.5.
 * A value of 1.0 results in a filter with a Q of 2.5.
 *
 * If the input parameter is out of bounds, it is clipped to the corresponding
 * min/max value.  This function will return a value indicating an
 * invalid input parameter was supplied but the effect will continue to operate.
 *
 * @param c Pointer to instance structure
 * @param q_new New depth value betweeen (0.0 and 1.0)
 *
 * @return Autowah result (enumeration)
 */
RESULT_AUTOWAH autowah_modify_q(AUTOWAH * c, float q_new) {

	RESULT_AUTOWAH res;

	float q;
	if (q_new > AUTOWAH_Q_MAX) {
		q = AUTOWAH_Q_MAX;
		res = AUTOWAH_INVALID_Q;
	} else if (q_new < AUTOWAH_Q_MIN) {
		q = AUTOWAH_Q_MIN;
		res = AUTOWAH_INVALID_Q;
	} else {
		q = q_new;
		res = AUTOWAH_OK;
	}

	c->q = 2.0 * q + 0.5;

	if (c->q == c->q_last) {
		return res;
	} else {
		c->q_last = q;
	}

	filter_modify_q(&c->bpf1, c->q);
	filter_modify_q(&c->bpf2, c->q);
	filter_modify_q(&c->bpf3, c->q);

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
void autowah_read(AUTOWAH * c, float * audio_in, float * audio_out,
		uint32_t audio_block_size) {

	// If this instance hasn't been properly initialized, pass audio through
	if (c == NULL || !c->initialized) {
		for (int i = 0; i < audio_block_size; i++) {
			audio_out[i] = audio_in[i];
		}
		return;
	}

	// Update amplitude
	for (int i = 0; i < audio_block_size; i++) {
		measure_amp_peak(audio_in[i], &c->measured_ampitude, c->decay);
	}

	float env_freq = c->measured_ampitude * c->depth;
	if (env_freq > AUTOWAH_MAX_BF_FREQ)
		env_freq = AUTOWAH_MAX_BF_FREQ;

	// Update filter center frequency based on amplitude
	filter_modify_freq(&c->bpf1, 300.0 + env_freq);
	filter_modify_freq(&c->bpf2, 300.0 + env_freq);
	filter_modify_freq(&c->bpf3, 300.0 + env_freq);

	// Apply band pass filters in series to create a 6th order filter
	filter_read(&c->bpf1, audio_in, audio_out, audio_block_size);
	filter_read(&c->bpf2, audio_out, audio_out, audio_block_size);
	filter_read(&c->bpf3, audio_out, audio_out, audio_block_size);
}
