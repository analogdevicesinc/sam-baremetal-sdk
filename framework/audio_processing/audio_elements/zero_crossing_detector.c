/*
 * Copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 *
 * A zero-crossing detector measures the periodicity of an incoming
 * waveform to determine its fundamental frequency.
 */

#include <stats.h>
#include <filter.h>
#include <stdlib.h>

#include "common/multicore_shared_memory.h"

#include "../audio_elements/audio_utilities.h"
#include "zero_crossing_detector.h"

#include "audio_utilities.h"

#define ZERO_CROSS_THRESHOLD_MIN    (0.0)
#define ZERO_CROSS_THRESHOLD_MAX    (1.0)

// Min/max limits and other constants

/**
 * @brief Initializes instance of a zero-crossing detector
 *
 * @param c Pointer to instance structure
 * @param threshold Amplitude to detect crossing event (usually around 0.1)
 * @param audio_sample_rate The system audio sample rate
 * @return zero crossing result (enumeration)
 */
RESULT_ZERO_CROSSING zero_cross_setup(ZERO_CROSSING_DETECTOR * c,
		float threshold, float audio_sample_rate) {

	if (c == NULL) {
		return ZERO_CROSS_INVALID_INSTANCE_POINTER;
	}
	c->initialized = false;

	if (threshold > ZERO_CROSS_THRESHOLD_MAX
			|| threshold < ZERO_CROSS_THRESHOLD_MIN) {
		return ZERO_CROSS_INVALID_THRESHOLD;
	}

	c->peak_amplitude_pos = 0;
	c->peak_amplitude_neg = 0;

	filter_setup(&c->lpf, BIQUAD_TYPE_LPF, BIQUAD_TRANS_VERY_SLOW,
			(pm float *) c->lpf_coeffs, 600.0, 1.0, 1.0, audio_sample_rate);

	filter_setup(&c->zero_block, BIQUAD_TYPE_HPF, BIQUAD_TRANS_VERY_SLOW,
			(pm float *) c->zb_coeffs, 50.0, 1.0, 1.0, audio_sample_rate);

	// Initialize C struct parameters
	c->audio_sample_rate = audio_sample_rate;
	c->freq_lock = false;
	c->mean_freq = 0.0;

	c->wave_state = 0;
	c->period_counter = 0;

	// Instance was successfully initialized
	c->initialized = true;
	return ZERO_CROSS_OK;

}

/**
 * @brief Processes a block of audio data
 *
 * @param c Pointer to instance structure
 * @param audio_in Pointer to floating point audio input buffer (mono)
 * @param audio_block_size The number of floating-point words to process
 * @param detected_frequency A pointer to return the detected frequency
 * @return True indicates frequnecy lock, false indicates no signal or no lock
 */
#pragma optimize_for_speed
bool zero_crossing_read(ZERO_CROSSING_DETECTOR * c, float * audio_in,
		uint32_t audio_block_size, float * detected_frequency) {

	// If this instance hasn't been properly initialized, pass audio through
	if (c == NULL || !c->initialized) {
		return false;
	}

	int i;

	float zc_buff1[MAX_AUDIO_BLOCK_SIZE];
	float zc_buff2[MAX_AUDIO_BLOCK_SIZE];
	float zc_buff3[MAX_AUDIO_BLOCK_SIZE];

	float filtered_audio_in[MAX_AUDIO_BLOCK_SIZE];
	float dc_blocked_audio_in[MAX_AUDIO_BLOCK_SIZE];

	copy_buffer(audio_in, zc_buff1, audio_block_size);

	// Remove DC offset
	filter_read(&c->zero_block, audio_in, dc_blocked_audio_in,
			audio_block_size);

	// Run a low-pass filter on the audio
	filter_read(&c->lpf, dc_blocked_audio_in, filtered_audio_in,
			audio_block_size);

	// Optionally gain up the input if needed
	// gain_buffer(filtered_audio_in, 2.0, audio_block_size);

	// Measure current peak amplitude of waveform and pick a threshold value that's
	// a fraction of this
	for (int i = 0; i < audio_block_size; i++) {
		measure_amp_peak(filtered_audio_in[i], &c->peak_amplitude_pos, 0.9999);
		measure_amp_peak(-filtered_audio_in[i], &c->peak_amplitude_neg, 0.9999);
	}
	float vol_threshold_pos = c->peak_amplitude_pos * 0.5;
	float vol_threshold_neg = c->peak_amplitude_neg * 0.5;

	if (c->peak_amplitude_pos < 0.001) {
		return false;
	}

	for (i = 0; i < audio_block_size - 1; i++) {

		if (filtered_audio_in[i] < vol_threshold_pos
				&& filtered_audio_in[i + 1] >= vol_threshold_pos
				&& c->wave_state == 1) {

			// Calculate current frequency
			float freq = c->audio_sample_rate / (float) c->period_counter;

			// If the measured frequency is in a valid range, operate!
			if (freq > 70.0 && freq < 700.0) {

				float freq_half = freq * 0.5;
				uint32_t freq_half_cnt = 0;
				for (int i = 0; i < FREQ_HIST_LEN; i++) {
					if (fabs(c->freq_history[i] - freq_half) < 10.0) {
						freq_half_cnt++;
					}
				}
				if (freq_half_cnt > 4) {
					freq = freq_half;
				}

				// save measured frequency in our frequency buffer
				c->freq_history[c->freq_ptr++] = freq;
				if (c->freq_ptr >= FREQ_HIST_LEN) {
					c->freq_ptr = 0;
				}

				// calculate the variance of our last several frequency readings
				c->varf_val = varf(c->freq_history, FREQ_HIST_LEN);

				// if the variance is below a threshold we have a lock!
				if (c->varf_val < 5.0) {
					c->freq_lock = true;
					c->mean_freq = meanf(c->freq_history, FREQ_HIST_LEN);
					c->freq_lock_cntr = ZC_FREQ_LOCK_CNTR;
				} else {
					c->freq_lock = false;
				}

			}
			// toggle wave state (positive or negative portion of waveform)
			c->wave_state = 0;
			c->period_counter = 0;

		}

		// otherwise, see if we need to toggle our wave state
		else if (filtered_audio_in[i] < -vol_threshold_neg
				&& filtered_audio_in[i + 1] >= -vol_threshold_neg
				&& c->wave_state == 0) {
			c->wave_state = 1;
		}

		c->period_counter += 1;

	}

	// One more since we're only iterating audio_block_size-1 times
	c->period_counter += 1;

	// If the period is more than 2000 samples, it's likely not a valid waveform anymore
	if (c->period_counter > 2000) {
		c->freq_lock = false;
	}

	// If it's been a while since we've seen any action, start over
	if (c->period_counter > 10000) {
		for (int i = 0; i < FREQ_HIST_LEN; i++) {
			c->freq_history[i] = (float) rand();
		}
	}

	// Set frequency to current mean
	*detected_frequency = c->mean_freq;

	// Return the frequency lock status

	if (!c->freq_lock && c->freq_lock_cntr) {
		c->freq_lock_cntr--;
		return true;
	} else {
		return c->freq_lock;
	}
}
