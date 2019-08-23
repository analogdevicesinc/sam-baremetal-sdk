/*
 * Copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 *
 * See .c file for documentation.
 */

#ifndef _SIMPLE_SYNTH_H
#define _SIMPLE_SYNTH_H

#include <stdint.h>
#include <stdbool.h>
#include "../audio_elements/audio_elements_common.h"

// Various types of synth oscillators to choose from
typedef enum {
	SYNTH_SINE, SYNTH_TRIANGLE, SYNTH_SQUARE, SYNTH_PULSE, SYNTH_RAMP
} SYNTH_OPERATOR;

// Result enumerations
typedef enum {
	SIMPLE_SYNTH_OK, SIMPLE_SYNTH_INVALID_INSTANCE_POINTER
} RESULT_SIMPLE_SYNTH;

// C struct with parameters and state information
typedef struct {

	bool initialized;

	bool playing;    // Whether this synth is playing or not

	// Shape of the ADSR envelope
	uint32_t env_attack;
	uint32_t env_decay;
	uint32_t env_sustain;
	uint32_t env_release;

	// Current note and volume
	float volume;
	uint32_t note;

	// time variable for tone generators
	float t;
	float t_inc;

	// Position in ADSR envelope
	uint32_t position;

	SYNTH_OPERATOR synth_operator;

	// Optional additional parameters for the tone generators
	float operator_param1;
	float operator_param2;

	// System parameters
	uint32_t audio_block_size;
	float sample_rate;

} SIMPLE_SYNTH;

#if __cplusplus
extern "C" {
#endif

RESULT_SIMPLE_SYNTH synth_setup(SIMPLE_SYNTH * c, uint32_t attack,
		uint32_t decay, uint32_t sustain, uint32_t release,
		SYNTH_OPERATOR synth_operator, float audio_sample_rate);

void synth_play_note(SIMPLE_SYNTH * C, uint32_t note, float volume);

void synth_play_note_freq(SIMPLE_SYNTH * C, float freq, float volume);

void synth_update_note_freq(SIMPLE_SYNTH * C, float freq);

void synth_stop_note(SIMPLE_SYNTH * C);

void synth_set_operator_param1(SIMPLE_SYNTH * C, float val);
void synth_set_operator_param2(SIMPLE_SYNTH * C, float val);

void synth_read(SIMPLE_SYNTH * C, float * audio_out, uint32_t audio_block_size);

#if __cplusplus
}
#endif

#endif
