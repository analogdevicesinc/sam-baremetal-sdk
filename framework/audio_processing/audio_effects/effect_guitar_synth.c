/*
 * copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 *
 * A guitar synth creates additional synthesized voices / instruments at the
 * same frequency that is currently being played.  It does this by first
 * determining the frequency being played using a zero-crossing detector.
 * Based on the detected frequency, it synthesis additional waveforms.
 *
 * This audio effect also serves as an example of how to utilize the
 * zero_crossing_detector, simple synth and biquad filter audio elements.
 */

#include "effect_guitar_synth.h"
#include "../audio_elements/audio_utilities.h"

// Min/max limits and other constants
#define  GUITAR_SYNTH_CLEAN_MIX_MIN      (0.0)
#define  GUITAR_SYNTH_CLEAN_MIX_MAX      (1.0)
#define  GUITAR_SYNTH_SYNTH_MIX_MIN      (0.0)
#define  GUITAR_SYNTH_SYNTH_MIX_MAX      (1.0)

/**
 * @brief Initializes instance of a guitar synth
 *
 *
 * @param c Pointer to instance structure
 * @param clean_mix Orignal instrument mix (0.0 -> 1.0)
 * @param synth_mix Synth mix (0.0 -> 1.0)
 * @param audio_sample_rate System audio sample rate
 * @return Guitar synth result (enumeration)
 */
RESULT_GUITAR_SYNTH guitar_synth_setup(GUITAR_SYNTH * c, float clean_mix,
		float synth_mix, float audio_sample_rate) {

	if (c == NULL) {
		return GUITAR_SYNTH_INVALID_INSTANCE_POINTER;
	}

	c->initialized = false;

	c->clean_mix = clean_mix;
	c->synth_mix = synth_mix;

	c->synth_attack = 3000;
	c->synth_decay = 48000;
	c->synth_sustain = 48000 * 10;
	c->synth_release = 1000;

	c->synth_volume = 0.5;
	c->measured_ampitude = 0;

	// Set up zero-crossing detector
	zero_cross_setup(&c->zc_detect, ZC_DEFAULT_THRESHOLD, audio_sample_rate);

	// Set up synthesizers
	synth_setup(&c->synth, c->synth_attack, c->synth_decay, c->synth_sustain,
			c->synth_release, SYNTH_RAMP, audio_sample_rate);

	synth_setup(&c->synth_octave_low_1, c->synth_attack, c->synth_decay,
			c->synth_sustain, c->synth_release, SYNTH_TRIANGLE,
			audio_sample_rate);

	synth_setup(&c->synth_octave_low_2, c->synth_attack, c->synth_decay,
			c->synth_sustain, c->synth_release, SYNTH_SINE, audio_sample_rate);

	// Set up envelope filter
	filter_setup(&c->env_filter, BIQUAD_TYPE_BPF, BIQUAD_TRANS_VERY_SLOW,
			(pm float *) c->env_filter_coeffs, 400.0, 3.0, 1.0,
			audio_sample_rate);

	c->lock_cntr = 0;

	// Instance was successfully initialized
	c->initialized = true;
	return GUITAR_SYNTH_OK;
}

/**
 * @brief Modify the clean mix of the effect
 *
 * If the input parameter is out of bounds, it is clipped to the corresponding
 * min/max value.  This function will return a value indicating an
 * invalid input parameter was supplied but the effect will continue to operate.
 *
 * @param c Pointer to instance structure
 * @param clean_mix_new Mew clean / instrument mix (0.0 -> 1.0)
 * @return Guitar synth result (enumeration)
 */
RESULT_GUITAR_SYNTH guitar_synth_modify_clean_mix(GUITAR_SYNTH * c,
		float clean_mix_new) {

	RESULT_GUITAR_SYNTH res;

	float clean_mix;
	if (clean_mix_new > GUITAR_SYNTH_CLEAN_MIX_MAX) {
		clean_mix = GUITAR_SYNTH_CLEAN_MIX_MAX;
		res = GUITAR_SYNTH_INVALID_CLEAN_MIX;
	} else if (clean_mix_new < GUITAR_SYNTH_CLEAN_MIX_MIN) {
		clean_mix = GUITAR_SYNTH_CLEAN_MIX_MIN;
		res = GUITAR_SYNTH_INVALID_CLEAN_MIX;
	} else {
		clean_mix = clean_mix_new;
		res = GUITAR_SYNTH_OK;
	}

	c->clean_mix = clean_mix;

	return res;
}

/**
 * @brief Modify the synth mix of the effect
 *
 * If the input parameter is out of bounds, it is clipped to the corresponding
 * min/max value.  This function will return a value indicating an
 * invalid input parameter was supplied but the effect will continue to operate.
 *
 * @param c Pointer to instance structure
 * @param clean_mix_new New synth mix (0.0 -> 1.0)
 * @return Guitar synth result (enumeration)
 */
RESULT_GUITAR_SYNTH guitar_synth_modify_synth_mix(GUITAR_SYNTH * c,
		float synth_mix_new) {

	RESULT_GUITAR_SYNTH res;

	float synth_mix;
	if (synth_mix_new > GUITAR_SYNTH_SYNTH_MIX_MAX) {
		synth_mix = GUITAR_SYNTH_SYNTH_MIX_MAX;
		res = GUITAR_SYNTH_INVALID_SYNTH_MIX;
	} else if (synth_mix_new < GUITAR_SYNTH_SYNTH_MIX_MIN) {
		synth_mix = GUITAR_SYNTH_SYNTH_MIX_MIN;
		res = GUITAR_SYNTH_INVALID_SYNTH_MIX;
	} else {
		synth_mix = synth_mix_new;
		res = GUITAR_SYNTH_OK;
	}

	c->synth_mix = synth_mix;

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
void guitar_synth_read(GUITAR_SYNTH * c, float * audio_in, float * audio_out,
		uint32_t audio_block_size) {

	// If this instance hasn't been properly initialized, pass audio through
	if (c == NULL || !c->initialized) {
		for (int i = 0; i < audio_block_size; i++) {
			audio_out[i] = audio_in[i];
		}
		return;
	}

	float synth_out_1[MAX_AUDIO_BLOCK_SIZE], synth_out_2[MAX_AUDIO_BLOCK_SIZE],
			synth_out_3[MAX_AUDIO_BLOCK_SIZE];

	c->current_lock = zero_crossing_read(&c->zc_detect, audio_in,
			audio_block_size, &c->detected_frequency);

	if (c->current_lock) {
		c->lock_cntr++;
		if (c->lock_cntr > 100) {
			c->lock_cntr = 100;
		}
	} else if (c->lock_cntr) {
		c->lock_cntr--;
	}

	// Beginning of a new note event
	if (c->current_lock && !c->last_lock) {
		synth_play_note_freq(&c->synth, c->detected_frequency, c->synth_volume);
		synth_play_note_freq(&c->synth_octave_low_1,
				c->detected_frequency * 0.5, c->synth_volume);
		synth_play_note_freq(&c->synth_octave_low_2,
				c->detected_frequency * 0.25, c->synth_volume);
	}

	// End of note
	else if (!c->lock_cntr) {
		synth_stop_note(&c->synth);
		synth_stop_note(&c->synth_octave_low_1);
		synth_stop_note(&c->synth_octave_low_2);
	}

	// Update current note frequency in case note has been bent
	synth_update_note_freq(&c->synth, c->detected_frequency);
	synth_update_note_freq(&c->synth_octave_low_1, c->detected_frequency * 0.5);
	synth_update_note_freq(&c->synth_octave_low_2,
			c->detected_frequency * 0.25);

	// Read audio blocks from synth engine
	synth_read(&c->synth, synth_out_1, audio_block_size);
	synth_read(&c->synth_octave_low_1, synth_out_2, audio_block_size);
	synth_read(&c->synth_octave_low_2, synth_out_3, audio_block_size);

	// Mix it together
	float amplitude;
	for (int i = 0; i < audio_block_size; i++) {
		measure_amp_peak(audio_in[i], &c->measured_ampitude, 0.9999);
		audio_out[i] = (audio_in[i] * c->clean_mix * 2.0)
				+ (synth_out_1[i] * 0.5 + synth_out_2[i] * 0.95
						+ synth_out_3[i] * 0.5) * 4.0 * c->measured_ampitude
						* c->synth_mix;

	}

	// Apply envelope filter
	float env_freq = c->measured_ampitude * 15000.0;
	if (env_freq > 800.0)
		env_freq = 800.0;

	filter_modify_freq(&c->env_filter, 400.0 + env_freq);

	filter_read(&c->env_filter, audio_out, audio_out, audio_block_size);

	c->last_lock = c->current_lock;

}

