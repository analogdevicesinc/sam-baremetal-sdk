/*
 * Copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 *
 * Simple synth is a basic synthesizer engine which supports a handful of common
 * oscillators (sine, triangle, square, ramp) and ADSR (attack, decay, sustain,
 * release) envelop funtionality.  Each instance of the simple synth can generate
 * one voice so to generate multiple voices, multiple instances need to be used.
 * Each instance has a 'playing' flag so you can set up an array of SIMPLE_SYNTH
 * instances and poll each element of the array to find one that is not presently
 * playing to initiate a new voice.
 *
 */
#include <stdlib.h>
#include <math.h>
#include "simple_synth.h"
#include "../audio_elements/oscillators.h"

// Prototypes for static functions
static float note_to_increment(uint32_t note, float sampling_rate);
static float get_envelope(SIMPLE_SYNTH * c);

/**
 * @brief Initializes instance of the synthesizer (single voice)
 * @param attack Waveform attack in number of samples (i.e. 48000=1 second with 48KHz sampling rate)
 * @param decay Waveform decay measured in number of samples
 * @param sustain Waveform sustain measured in number of samples
 * @param release Waveform release measured in number of samples
 * @param synth_operator Type of waveform used in synth
 * @param audio_sample_rate The system audio sample rate
 * @return simple synth result (enumeration)
 */
RESULT_SIMPLE_SYNTH synth_setup(SIMPLE_SYNTH * c, uint32_t attack,
		uint32_t decay, uint32_t sustain, uint32_t release,
		SYNTH_OPERATOR synth_operator, float audio_sample_rate) {

	if (c == NULL) {
		return SIMPLE_SYNTH_INVALID_INSTANCE_POINTER;
	}
	c->initialized = false;

	// reset state variables
	c->playing = false;
	c->position = 0;

	// Set ADSR parameters
	c->env_attack = attack;
	c->env_decay = decay;
	c->env_sustain = sustain;
	c->env_release = release;

	// Set operator
	c->synth_operator = synth_operator;

	// Set system audio parameters
	c->sample_rate = audio_sample_rate;

	// Instance was successfully initialized
	c->initialized = true;
	return SIMPLE_SYNTH_OK;

}

/**
 * @brief Reads the next frame of audio from the synth engine
 *
 * @param c Pointer to instance structure
 * @param audio_out Pointer to floating point output buffer (mono)
 * @param audio_block_size The number of floating-point words to process
 */
#pragma optimize_for_speed
void synth_read(SIMPLE_SYNTH * c, float * audio_out, uint32_t audio_block_size) {

	int i;

	if (c == NULL || !c->initialized || !c->playing) {
		for (i = 0; i < audio_block_size; i++) {
			audio_out[i] = 0.0;
		}
	}

	float vol = c->volume;
	float t = c->t;
	float t_inc = c->t_inc;
	switch (c->synth_operator) {
	case SYNTH_SINE:
		for (i = 0; i < audio_block_size; i++) {
			audio_out[i] = vol * get_envelope(c) * oscillator_sine(t);
			t += t_inc;
			if (t >= 1.0)
				t -= 1.0;
			c->position++;
		}
		break;
	case SYNTH_TRIANGLE:
		for (i = 0; i < audio_block_size; i++) {
			audio_out[i] = vol * get_envelope(c) * oscillator_triangle(t);
			t += t_inc;
			if (t >= 1.0)
				t -= 1.0;
			c->position++;
		}
		break;
	case SYNTH_SQUARE:
		for (i = 0; i < audio_block_size; i++) {
			audio_out[i] = vol * get_envelope(c) * oscillator_square(t);
			t += t_inc;
			if (t >= 1.0)
				t -= 1.0;
			c->position++;
		}
		break;
	case SYNTH_PULSE:
		for (i = 0; i < audio_block_size; i++) {
			audio_out[i] = vol * get_envelope(c)
					* oscillator_pulse(t, c->operator_param1);
			t += t_inc;
			if (t >= 1.0)
				t -= 1.0;
			c->position++;
		}
		break;
	case SYNTH_RAMP:
		for (i = 0; i < audio_block_size; i++) {
			audio_out[i] = vol * get_envelope(c) * oscillator_ramp(t);
			t += t_inc;
			if (t >= 1.0)
				t -= 1.0;
			c->position++;
		}
		break;
	}
	c->t = t;

}

/**
 * @brief Plays a note using MIDI note number
 *
 * More info on MIDI note numbers here:
 * http://www.inspiredacoustics.com/en/MIDI_note_numbers_and_center_frequencies
 *
 * @param c Pointer to instance structure
 * @param note The MIDI note value
 * @param volume Volume of the note (0.0->1.0 typically)
 */
void synth_play_note(SIMPLE_SYNTH * c, uint32_t note, float volume) {

	c->playing = true;
	c->position = 0;
	c->t = 0.0;
	c->volume = volume;
	c->note = note;
	c->t_inc = note_to_increment(note, c->sample_rate);

}

/**
 * @brief Plays a note using note frequency
 *
 * @param c Pointer to instance structure
 * @param freq Frequency of note to be played in Hz
 * @param volume Volume of the note (0.0->1.0 typically)
 */
void synth_play_note_freq(SIMPLE_SYNTH * c, float freq, float volume) {

	c->playing = true;
	c->position = 0;
	c->t = 0.0;
	c->volume = volume;

	c->t_inc = 1.0 / (c->sample_rate / freq);
}

/**
 * @brief Updates the frequency of the note being played
 *
 * This is useful for supporting note bending, for example.
 * See the guitar synth audio effecf for an example of how
 * to use this.
 *
 * @param c Pointer to instance structure
 * @param freq New note frequency
 */
void synth_update_note_freq(SIMPLE_SYNTH * c, float freq) {

	c->t_inc = 1.0 / (c->sample_rate / freq);
}

/**
 * @brief Stops the note if one is playing
 *
 * @param c Pointer to instance structure
 */
void synth_stop_note(SIMPLE_SYNTH * c) {

	// If we're not already playing a note, do nothing
	if (!c->playing) {
		return;
	}

	// If we're already in the 'release' portion of the envelope, let it play out
	if (c->position > c->env_attack + c->env_decay + c->env_sustain) {
		return;
	}
	// Otherwise, jump to the beginning of the 'release' portion of the envelope
	else {
		c->position = c->env_attack + c->env_decay + c->env_sustain;
	}
}

/**
 * @brief Sets a synth parameter (if one exists)
 *
 * Certain synthesis oscillators may have additional parameters
 *
 * @param c Pointer to instance structure
 * @param val Value for this parameter
 */
void synth_set_operator_param1(SIMPLE_SYNTH * c, float val) {
	c->operator_param1 = val;
}

/**
 * @brief Sets a synth parameter (if one exists)
 *
 * Certain synthesis oscillators may have additional parameters
 *
 * @param c Pointer to instance structure
 * @param val Value for this parameter
 */
void synth_set_operator_param2(SIMPLE_SYNTH * c, float val) {
	c->operator_param2 = val;
}

/**
 * @brief Converts a frequency to a time increment
 *
 * @param note MIDI note value
 * @param sampling_rate Current system sampling rate
 *
 * @return Time increment value
 */
static float note_to_increment(uint32_t note, float sampling_rate) {

	if (note < 21)
		note = 21;
	if (note > 108)
		note = 108;
	float note_f = (float) note;

	float freq = powf(2.0, (note_f - 69.0) * (1.0 / 12.0)) * 440.0;

	float t_inc = freq / sampling_rate;

	return t_inc;

}

/*
 *
 */

/**
 * @brief  Gets the current volume modifier from our ADSR envelope
 *
 * @param c Pointer to instance structure
 * @return Current instrument volume level
 */
static float get_envelope(SIMPLE_SYNTH * c) {

	if (!c->playing) {
		return 0.0;
	}

	// Attack
	if (c->position < c->env_attack) {
		return (float) c->position / (float) c->env_attack;
	}

	// Decay
	if (c->position < c->env_attack + c->env_decay) {
		uint32_t pos = c->position - c->env_attack;

		return 0.8 + 0.2 * (1.0 - ((float) pos / (float) c->env_decay));
	}

	// Sustain
	if (c->position < c->env_attack + c->env_decay + c->env_sustain) {
		return 0.8;
	}

	// Release
	if (c->position
			< c->env_attack + c->env_decay + c->env_sustain + c->env_release) {
		uint32_t pos = c->position
				- (c->env_attack + c->env_decay + c->env_sustain);
		return 0.8 * (1.0 - ((float) pos / (float) c->env_release));
	}

	else {
		c->playing = false;
		return 0.0;
	}
}

