/*
 * Copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 *
 * These routines contain a number of preset audio effects and a "selector" routine
 * to switch between them.
 *
 * There is a setup function and an audio processing function which should be included
 * in the setup and audio processing functions of the audio callback (callback_audio_processing.cpp).
 *
 */

#include "common/audio_system_config.h"
#include "common/multicore_shared_memory.h"

#include "audio_effects_selector.h"

// Audio buffers to pass audio to and from the effects
float audio_effects_left_in[AUDIO_BLOCK_SIZE];
float audio_effects_right_in[AUDIO_BLOCK_SIZE];

float audio_effects_left_out[AUDIO_BLOCK_SIZE];
float audio_effects_right_out[AUDIO_BLOCK_SIZE];

/**
 * @brief Audio bypass routine
 *
 * The effect bypass routine will simply pass audio from the input buffers to
 * the output buffers.
 */
static void effect_bypass(void) {

	// Copy input buffers to output buffers, thereby bypassing effects
	copy_buffer(audio_effects_left_in, audio_effects_left_out,
	AUDIO_BLOCK_SIZE);
	copy_buffer(audio_effects_right_in, audio_effects_right_out,
	AUDIO_BLOCK_SIZE);

}

/******************************************************************************
 * Effects running on SHARC core 1
 *****************************************************************************/

/**
 * 1 - ECHO EFFECT
 *
 * This effect uses the integer delay audio element to create a basic echo effect.
 * This effect is build using a single audio element, the integer_delay_lpf element.
 * This element implements an echo effect but also includes a low-pass filter in
 * the feedback path which is a useful function when building reverbs out of delay
 * lines.
 *
 * POT/HADC0 : Modifies the amount of dampening in the delay feedback loop
 * POT/HADC1 : Modifies the lenght of the delay
 * POT/HADC2 : Modifies the amount of feedback in the delay (duration of the echoes)
 *
 * Some fun things to try:
 *  - Add a second delay line and feed the output of the first delay line into the second
 *  - Feed the output of the second delay back into the first
 *  - Try very different delay values for left and right side
 *
 */

// Declare instances and buffers
DELAY_LPF integer_delay_l, integer_delay_r;

// declare delay buffers in SDRAM with a max length of 32000 (2/3 of a second each
#define INT_DELAY_LEN	(32000)
float section("seg_sdram") integer_delay_line_l[INT_DELAY_LEN];
float section("seg_sdram") integer_delay_line_r[INT_DELAY_LEN];

/**
 * @brief Setup routine to initialize instances of the delay line
 */
static void effect_echo_setup() {

	// Initialize effect instances
	delay_setup(&integer_delay_l, integer_delay_line_l,
	INT_DELAY_LEN,
	INT_DELAY_LEN - 1000, 0.5, 0.8, 0.2);
	delay_setup(&integer_delay_r, integer_delay_line_r,
	INT_DELAY_LEN,
	INT_DELAY_LEN - 3000, 0.5, 0.8, 0.2);
}

/**
 * @brief  Process audio and update some modifiable parameters via the pots
 */
static void effect_echo_process() {

	// Apply effect
	delay_read(&integer_delay_l, audio_effects_left_in, audio_effects_left_out,
	AUDIO_BLOCK_SIZE);
	delay_read(&integer_delay_r, audio_effects_left_in, audio_effects_right_out,
	AUDIO_BLOCK_SIZE);

	// Use pot (HADC0) to modify the dampening factor in feeedback path of delay
	delay_modify_dampening(&integer_delay_l,
			multicore_data->audioproj_fin_pot_hadc0 * 0.3 + 0.1);
	delay_modify_dampening(&integer_delay_r,
			multicore_data->audioproj_fin_pot_hadc0 * 0.3 + 0.1);

	// Use pot (HADC1) to modify the lenght of the delay
	delay_modify_length(&integer_delay_l,
			INT_DELAY_LEN / 2
					+ multicore_data->audioproj_fin_pot_hadc1 * INT_DELAY_LEN
							/ 2);
	delay_modify_length(&integer_delay_r,
			INT_DELAY_LEN / 2
					+ multicore_data->audioproj_fin_pot_hadc1 * INT_DELAY_LEN
							/ 2);

	// Use pot (HADC2) to modify the feedback value
	delay_modify_feedback(&integer_delay_l,
			multicore_data->audioproj_fin_pot_hadc2);
	delay_modify_feedback(&integer_delay_r,
			multicore_data->audioproj_fin_pot_hadc2);

}

/**
 * 2 - MULTITAP ECHO EFFECT
 *
 * A multi-tap echo effect reads values of out of the delay line at multiple points
 * and mixes the outputs together.  Rather than a uniform set of echoes, multi-tap
 * delays can be used to create some very interesting echo rhythms.
 *
 * This implementation uses the integer_delay_multitap audio element and is configured
 * to utilize three taps.
 *
 * POT/HADC0 : nothing
 * POT/HADC1 : nothing
 * POT/HADC2 : nothing
 *
 * Some fun things to try:
 *  - Add more taps
 *  - Set the taps close to each other (e.g. 28000, 29000, 30000)
 *
 */

// Declare instances and buffers
MULTITAP_DELAY integer_mt_delay_l, integer_mt_delay_r;
#define INT_DELAY_LEN	(32000)
float section("seg_sdram") integer_mt_delay_line_l[INT_DELAY_LEN]; // Delay line in SDRAM
float section("seg_sdram") integer_mt_delay_line_r[INT_DELAY_LEN]; // Delay line in SDRAM

uint32_t tap_offsets_l[3] = { 10000, 20000, 28000 };
uint32_t tap_offsets_r[3] = { 8000, 22000, 29000 };
float tap_gains_l[3] = { 0.3, 0.4, 0.2 };
float tap_gains_r[3] = { 0.4, 0.3, 0.2 };

/**
 * @brief Setup routine to initialize instances of the multi-tap delay line
 */
static void effect_multitap_delay_setup() {

	// Initialize effect instance
	multitap_delay_setup(&integer_mt_delay_l, integer_mt_delay_line_l,
	INT_DELAY_LEN, 3, tap_offsets_l, tap_gains_l, 0.8);

	multitap_delay_setup(&integer_mt_delay_r, integer_mt_delay_line_r,
	INT_DELAY_LEN, 3, tap_offsets_r, tap_gains_r, 0.8);
}

/**
 * @brief  Process audio and update some modifiable parameters via the pots
 */
static void effect_multitap_delay_process() {

	// Apply effect
	multitap_delay_read(&integer_mt_delay_l, audio_effects_left_in,
			audio_effects_left_out,
			AUDIO_BLOCK_SIZE);

	multitap_delay_read(&integer_mt_delay_r, audio_effects_left_in,
			audio_effects_right_out,
			AUDIO_BLOCK_SIZE);
}

/**
 * 3 - TUBE DISTORTION SIMULATION
 *
 * This effect provides a basic simulation of a tube amplifier.  More information can
 * be found at the top of the audio_effects/effect_tube_distortion.c file.
 *
 * This implementation uses the integer_delay_multitap audio element and is configured
 * to utilize three taps.
 *
 * POT/HADC0 : distortion output gain
 * POT/HADC1 : distortion drive (prior to clipping)
 * POT/HADC2 : tone of output
 *
 * Some fun things to try:
 *  - Modify the original effect to include more filters or clipping stages
 *  - Add an effect like the echo effect after the distortion.  All of these
 *    effects can operate on data in place so you don't need separate input
 *    and output buffers.  In other words, the input and output buffer can
 *    be the same buffer.  You can run the delay effect on
 *    audio_effects_left_out and store the results in audio_effects_left_out.
 *
 */
TUBE_DISTORTION tube_dist;

/**
 * @brief Setup routine to initialize instance of the tube distortion simulator
 */
static void effect_tube_distortion_setup(void) {

	// Initialize effect instance
	tube_distortion_setup(&tube_dist,
			multicore_data->audioproj_fin_pot_hadc1 * 64.0,
			multicore_data->audioproj_fin_pot_hadc0 * 1.0,
			multicore_data->audioproj_fin_pot_hadc2,
			AUDIO_SAMPLE_RATE);
}

/**
 * @briend Process audio and update some modifiable parameters via the pots
 */
static void effect_tube_distortion_process(void) {
	tube_distortion_read(&tube_dist, audio_effects_left_in,
			audio_effects_left_out,
			AUDIO_BLOCK_SIZE);

	// Make stereo
	for (int i = 0; i < AUDIO_BLOCK_SIZE; i++) {
		audio_effects_right_out[i] = audio_effects_left_out[i];
	}

	// Use pot (HADC0) to modify the output gain of the distortion
	tube_distortion_modify_gain(&tube_dist,
			multicore_data->audioproj_fin_pot_hadc2 * 0.5);

	// Use pot (HADC1) to modify the input drive into the clipping function of the distortion
	tube_distortion_modify_drive(&tube_dist,
			multicore_data->audioproj_fin_pot_hadc1 * 64.0);

	// Use pot (HADC2) to modify the bandpass filter after the clipper to change the tone
	tube_distortion_modify_contour(&tube_dist,
			multicore_data->audioproj_fin_pot_hadc0);

}

/**
 * 4 - MULTIBAND COMPRESSOR
 *
 * A multiband compressor applies compression (dynamics processing) to
 * different frequency bands of the original signal.  This enables different
 * compression parameters to be used on different bands of the signal.  This
 * implmentation uses just two bands.  The bands are split using a low-pass
 * filter and a high-pass filter with the same cutoff frequency.  The cutoff
 * frequency is modifiable and one of the parameters.
 *
 * In general, compressors are used to increase the perceived sustain of an
 * instrument and work very well in particular with acoustic guitars.
 *
 * POT/HADC0 : the cross-over frequency (Hz) ranging from 100-600Hz
 * POT/HADC1 : the compressor threshold
 * POT/HADC2 : the output gain of the compressor
 *
 * Some fun things to try:
 *  - There are several additional parameters that can be modified in the
 *    setup routine in effect_multiband_compressor.c.  Try playing around
 *    with different settings.
 *
 */
MULTIBAND_COMPRESSOR multiband_comp_l, multiband_comp_r;

/**
 * @brief Setup routine to initialize instances of the multiband compressor
 */
static void effect_multiband_compressor_setup(void) {

	// Initialize effect instances for left and right channels
	multiband_comp_setup(&multiband_comp_l, 200.0, -40.0,
	AUDIO_SAMPLE_RATE);

	multiband_comp_setup(&multiband_comp_r, 200.0, -40.0,
	AUDIO_SAMPLE_RATE);

}

/**
 * @brief Process audio and update some modifiable parameters via the pots
 */
static void effect_multiband_compressor_process(void) {

	multiband_comp_read(&multiband_comp_l, audio_effects_left_in,
			audio_effects_left_out,
			AUDIO_BLOCK_SIZE);

	multiband_comp_read(&multiband_comp_r, audio_effects_right_in,
			audio_effects_right_out,
			AUDIO_BLOCK_SIZE);

	// Use pot (HADC0) set the cross-over frequency in Hz
	multiband_comp_change_xover(&multiband_comp_l,
			100.0 + 600.0 * multicore_data->audioproj_fin_pot_hadc0);
	multiband_comp_change_xover(&multiband_comp_r,
			100.0 + 600.0 * multicore_data->audioproj_fin_pot_hadc0);

	// Use pot (HADC1) to set compressor threshold (dB)
	multiband_comp_change_thresh(&multiband_comp_l,
			-50.0 * multicore_data->audioproj_fin_pot_hadc1);
	multiband_comp_change_thresh(&multiband_comp_r,
			-50.0 * multicore_data->audioproj_fin_pot_hadc1);

	// Use pot (HADC2) to modify the output gain of the compressors
	multiband_comp_change_gain(&multiband_comp_l,
			4.0 * multicore_data->audioproj_fin_pot_hadc2);
	multiband_comp_change_gain(&multiband_comp_r,
			4.0 * multicore_data->audioproj_fin_pot_hadc2);

}

/**
 * 5 - STEREO FLANGER
 *
 * A stereo flanger is a variable delay effect where the original signal
 * is delayed by a varying amount and mixed back into the original signal.
 * A variable delay is the basis for a flanger effect, a chorus effect, a
 * vibrato effect and a phaser effect.  In this case, it is configured
 * as a flanger but could be easily modified to realize these other effects.
 *
 * POT/HADC0 : the cross-over frequency (Hz) ranging from 100-600Hz
 * POT/HADC1 : the compressor threshold
 * POT/HADC2 : the output gain of the compressor
 *
 * Some fun things to try:
 *  - Try reducing the delay length to create more of a phaser effect
 *
 */

STEREO_FLANGER flanger;

/**
 * @brief Setup routine to initialize instance of the stereo flanger
 */
static void effect_flanger_setup(void) {

	// Initialize effect instance
	flanger_setup(&flanger, 0.5, 0.5, 0.5, AUDIO_SAMPLE_RATE);

}

/**
 * Process audio and update some modifiable parameters via the pots
 */
static void effect_flanger_process(void) {

	// Apply effect
	flanger_read(&flanger, audio_effects_left_in, audio_effects_left_out,
			audio_effects_right_out,
			AUDIO_BLOCK_SIZE);

	// Use pot (HADC0) to set the flanger rate in Hz
	flanger_modify_rate(&flanger,
			2.0 * multicore_data->audioproj_fin_pot_hadc0);

	// Use pot (HADC1) to set the flanger depth (0 -> 1.0)
	flanger_modify_depth(&flanger, multicore_data->audioproj_fin_pot_hadc1);

	// Use pot (HADC2) to set the flanger feedback (-1.0 -> 0 -> 1.0)
	flanger_modify_feedback(&flanger,
			2.0 * multicore_data->audioproj_fin_pot_hadc2 - 1.0);

}

/**
 * 6 - GUITAR SYNTH
 *
 * The guitar synth effect attempts to determine which note has been played
 * by examining the periodicity of the waveform using the zero_crossing_detector
 * audio element.  Based on the detected frequency, it then generates tones using
 * the simple_synth audio element.
 *
 * POT/HADC0 : clean guitar mix
 * POT/HADC1 : synthesizer mix
 * POT/HADC2 : nothing
 *
 * Some fun things to try:
 *  - There are several additional parameters that can be tuned in the
 *    effect_guitar_synth.c file.
 *  - the pot connected to HADC2 is presently unused.  Use this to modify
 *    some other parameter of the guitar synth.
 *
 */
GUITAR_SYNTH guitar_synth;

/**
 * @brief Setup routine to initialize instance of the guitar synth
 */
static void effect_guitar_synth_setup(void) {

	// Initialize effect instance
	guitar_synth_setup(&guitar_synth, 0.5, 0.5,
	AUDIO_SAMPLE_RATE);

}

/**
 * Process audio and update some modifiable parameters via the pots
 */
static void effect_guitar_synth_process(void) {

	// Apply effect
	guitar_synth_read(&guitar_synth, audio_effects_left_in,
			audio_effects_left_out,
			AUDIO_BLOCK_SIZE);

	copy_buffer(audio_effects_left_out, audio_effects_right_out,
	AUDIO_BLOCK_SIZE);

	// Use pot (HADC0) to set the clean mix
	guitar_synth_modify_clean_mix(&guitar_synth,
			multicore_data->audioproj_fin_pot_hadc0);

	// Use pot (HADC1) to set the synth mix
	guitar_synth_modify_synth_mix(&guitar_synth,
			multicore_data->audioproj_fin_pot_hadc1);

}

/**
 * 7 - AUTO-WAH
 *
 * An autowah is a type of envelope filter that sweeps a bandpass filter over
 * the incoming audio based on the current amplitude of the input signal.  When
 * the amplitude increases, the filter sweeps towards higher frequnecies.
 * When the amplitude decreases, the filter sweeps towards lower frequencies.
 * The filter characteristics and range are similar to a traditional Wah pedal.
 *
 * POT/HADC0 : depth of frequency sweep
 * POT/HADC1 : decay time
 * POT/HADC2 : width of filter (Q)
 *
 * Some fun things to try:
 *  - Try changing the effect so the filter moves in the opposite direction than amplitude
 *
 */
AUTOWAH autowah;

/**
 * @brief Setup routine to initialize instance of the autowah
 */
static void effect_autowah_setup(void) {

	// Initialize effect instance
	autowah_setup(&autowah, multicore_data->audioproj_fin_pot_hadc0,
			multicore_data->audioproj_fin_pot_hadc1,
			AUDIO_SAMPLE_RATE);

}

/**
 * Process audio and update some modifiable parameters via the pots
 */
static void effect_autowah_process(void) {

	// Apply effect
	autowah_read(&autowah, audio_effects_left_in, audio_effects_left_out,
	AUDIO_BLOCK_SIZE);

	copy_buffer(audio_effects_left_out, audio_effects_right_out,
	AUDIO_BLOCK_SIZE);

	// Use pot (HADC0) to set the depth (i.e. frequency range of sweep)
	autowah_modify_depth(&autowah, multicore_data->audioproj_fin_pot_hadc0);

	// Use pot (HADC0) to set the decay time
	autowah_modify_decay(&autowah, multicore_data->audioproj_fin_pot_hadc1);

	// Use pot (HADC2) to set the width of the filter
	autowah_modify_q(&autowah, multicore_data->audioproj_fin_pot_hadc2);

}

/**
 * 9 -  MULTI-FX CHAINING
 *
 * This effect demonstrates how to chain multiple effects together.
 * In this case, we are chaining the tube distortion into the stereo flanger
 * and then into the echo effect.
 *
 * POT/HADC0 : flanger depth
 * POT/HADC1 : distortion drive
 * POT/HADC2 : echo delay
 *
 * Some fun things to try:
 *  - Try to create pleasing music with a ring modulator
 *
 */
STEREO_FLANGER flanger_fx1;
TUBE_DISTORTION tube_dist_fx1;
DELAY_LPF delay_l_fx1, delay_r_fx1;
#define FX_DELAY_LEN	(32000)
float section("seg_sdram") delay_line_l_fx1[INT_DELAY_LEN];	// Delay line in SDRAM
float section("seg_sdram") delay_line_r_fx1[INT_DELAY_LEN];	// Delay line in SDRAM

/**
 * @brief Setup routine to initialize instances for the multli-effects example
 */
static void multifx_1_test_setup(void) {

	// Initialize effect instances
	// Initialize effect instance
	flanger_setup(&flanger_fx1, 0.3, 0.2, -0.35, AUDIO_SAMPLE_RATE);

	tube_distortion_setup(&tube_dist_fx1,
			multicore_data->audioproj_fin_pot_hadc1 * 128.0, 0.20, 0.9,
			AUDIO_SAMPLE_RATE);

	delay_setup(&delay_l_fx1, delay_line_l_fx1,
	FX_DELAY_LEN,
	FX_DELAY_LEN - 1000, 0.3, 0.6, 0.2);
	delay_setup(&delay_r_fx1, delay_line_r_fx1,
	FX_DELAY_LEN,
	FX_DELAY_LEN, 0.3, 0.6, 0.2);

}

/**
 * @brief: Process audio and update some modifiable parameters via the pots
 */
static void multifx_1_test_process(void) {

	// Apply effects
	float temp_1[AUDIO_BLOCK_SIZE], temp_2[AUDIO_BLOCK_SIZE];

	// Apply distortion
	tube_distortion_read(&tube_dist_fx1, audio_effects_left_in, temp_1,
	AUDIO_BLOCK_SIZE);

	// Apply tremelo
	flanger_read(&flanger_fx1, temp_1, audio_effects_left_out,
			audio_effects_right_out,
			AUDIO_BLOCK_SIZE);

	// Apply delay / echo
	delay_read(&delay_l_fx1, audio_effects_left_out, audio_effects_left_out,
	AUDIO_BLOCK_SIZE);

	delay_read(&delay_r_fx1, audio_effects_right_out, audio_effects_right_out,
	AUDIO_BLOCK_SIZE);

	// Use pot (HADC0) to modify the flanger depth
	flanger_modify_depth(&flanger_fx1, multicore_data->audioproj_fin_pot_hadc0);

	// Use pot (HADC1) to modify the distortion drive
	tube_distortion_modify_drive(&tube_dist_fx1,
			multicore_data->audioproj_fin_pot_hadc1 * 64.0);

	// Use pot (HADC2) to modify the length of the delay
	delay_modify_length(&delay_l_fx1,
			FX_DELAY_LEN / 2
					+ multicore_data->audioproj_fin_pot_hadc2 * FX_DELAY_LEN
							/ 2);
	delay_modify_length(&delay_r_fx1,
			FX_DELAY_LEN / 2
					+ multicore_data->audioproj_fin_pot_hadc2 * FX_DELAY_LEN / 2
					- 1000);

}

/**
 * 9 - RING MODULATOR
 *
 * A ring modulator is an interesting beast.  It essentially modulates the
 * input signal with a second tone, often in the range of 50-500Hz.  This
 * modulation shifts the notes played around in the frequency spectrum to
 * create some wild effects.
 *
 * Here's a nice write up of songs that feature a ring modulator
 * https://www.theguardian.com/music/2009/nov/09/ring-modulators
 *
 * POT/HADC0 : modulation frequency (50->350.0 Hz)
 * POT/HADC1 : depth / mix
 * POT/HADC2 : nothing
 *
 * Some fun things to try:
 *  - Try to create pleasing music with a ring modulator
 *
 */
RING_MODULATOR ring_mod;

/**
 * @brief Setup routine to initialize instance of the ring modulator
 */
static void effect_ringmod_setup(void) {

	// Initialize effect instance
	ring_modulator_setup(&ring_mod, 200.0, 0.5,
	AUDIO_SAMPLE_RATE);

}

/**
 * @brief Process audio and update some modifiable parameters via the pots
 */
static void effect_ringmod_process(void) {

	// Apply effect
	ring_modulator_read(&ring_mod, audio_effects_left_in,
			audio_effects_left_out,
			AUDIO_BLOCK_SIZE);

	copy_buffer(audio_effects_left_out, audio_effects_right_out,
	AUDIO_BLOCK_SIZE);

	// Use pot (HADC0) to set the modulation frequency
	ring_modulator_modify_freq(&ring_mod,
			50.0 + 300.0 * multicore_data->audioproj_fin_pot_hadc0);

	// Use pot (HADC1) to set the depth / mix of the effect
	ring_modulator_modify_depth(&ring_mod,
			multicore_data->audioproj_fin_pot_hadc1);

}

/**
 * @brief Set up routines for all effects running on core 1
 */
void audio_effects_setup_core1(void) {

	effect_echo_setup();
	effect_multitap_delay_setup();
	effect_multiband_compressor_setup();
	effect_tube_distortion_setup();
	effect_flanger_setup();
	effect_guitar_synth_setup();
	effect_autowah_setup();
	multifx_1_test_setup();
	effect_ringmod_setup();

}

/**
 * This routine should be called every time a new block of audio arrives (in the callback
 * function) in SHARC core 1.
 */
void audio_effects_process_audio_core1(void) {

	/**
	 * On core 1, we'll apply various audio effects and on core 2, we'll do just reverb
	 */

	static int32_t core_1_effect_preset = 0;
	uint32_t core_1_total_presets = 10;

	switch (multicore_data->effects_preset) {
	case 1:
		effect_echo_process();
		break;
	case 2:
		effect_multitap_delay_process();
		break;
	case 3:
		effect_tube_distortion_process();
		break;
	case 4:
		effect_multiband_compressor_process();
		break;
	case 5:
		effect_flanger_process();
		break;
	case 6:
		effect_guitar_synth_process();
		break;
	case 7:
		effect_autowah_process();
		break;
	case 8:
		multifx_1_test_process();
		break;
	case 9:
		effect_ringmod_process();
		break;

	default:
		effect_bypass();
		break;
	}

}

/******************************************************************************
 * Effects running on SHARC core 2
 *
 * Audio from core 1 is passed to core 2.  Core 2 uses the same buffer naming
 * conventions so the input and output buffers use the same names.  This makes
 * it easy to move effects from core 1 to core 2 and visa versa.
 *
 *****************************************************************************/

// Instances
STEREO_REVERB reverb_stereo;
COMPRESSOR limiter_l, limiter_r;

/**
 * @brief  Set up routines for any effects running on core 2
 */
void audio_effects_setup_core2(void) {

	// Fast limiter on output
	compressor_setup(&limiter_l, -6.0, 1000.0, 5, 5, 1.0, AUDIO_SAMPLE_RATE);
	compressor_setup(&limiter_r, -6.0, 1000.0, 5, 5, 1.0, AUDIO_SAMPLE_RATE);

	// Stereo reverb
	reverb_setup(&reverb_stereo, 0.3, 1.0, 0.92, 0.2);

}

/**
 * @brief  Called every time a new block of audio arrives (in the callback
 * function) in SHARC core 2.
 */
void audio_effects_process_audio_core2(void) {

	float reverb_feedback[10] = { 0.0, 0.9, 0.8, 0.95, 0.8, 0.9, 0.95, 0.7, 0.9,
			0.97 };
	float reverb_dampening[10] = { 0.0, 0.1, 0.2, 0.2, 0.3, 0.3, 0.3, 0.4, 0.4,
			0.4 };

	reverb_change_feedback(&reverb_stereo,
			reverb_feedback[multicore_data->reverb_preset]);
	reverb_change_lp_damp_coeff(&reverb_stereo,
			reverb_dampening[multicore_data->reverb_preset]);

	if (multicore_data->reverb_preset == 0) {
		effect_bypass();
	} else {

		// Apply limiter at -6dB to avoid clipping from earlier stage effects
		compressor_read(&limiter_l, audio_effects_left_out,
				audio_effects_left_out, AUDIO_BLOCK_SIZE);
		compressor_read(&limiter_r, audio_effects_left_out,
				audio_effects_left_out, AUDIO_BLOCK_SIZE);

		// Apply stereo reverb effect
		reverb_read(&reverb_stereo, audio_effects_left_in,
				audio_effects_left_out, audio_effects_right_out,
				AUDIO_BLOCK_SIZE);

	}

}
