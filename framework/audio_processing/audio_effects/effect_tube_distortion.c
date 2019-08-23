/*
 * copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 *
 * This effect simulates a basic tube distortion which is essentially a
 * filter followed by a clipping function followed then by a second filter.
 *
 *  IN o-->[BPF]-->[drive]-->[clipper]-->[BPF]-->[gain]--->o OUT
 *
 * More advanced tube distortion modeling may rely on several clipping
 * and filtering stages. The SHARC processor certainly has the processing
 * power to realize much more complex models.
 *
 * This audio effect also serves as an example of how to utilize the
 * clipper and biquad filter audio elements.
 */

#include "effect_tube_distortion.h"

// Min/max limits and other constants
#define  TUBE_DISTORTION_CONTOUR_MIN      (0.0)
#define  TUBE_DISTORTION_CONTOUR_MAX      (1.0)
#define  TUBE_DISTORTION_THRESHOLD_MIN    (0.05)
#define  TUBE_DISTORTION_THRESHOLD_MAX    (1.0)
#define  TUBE_DISTORTION_DRIVE_MIN        (1.0)
#define  TUBE_DISTORTION_DRIVE_MAX        (128.0)
#define  TUBE_DISTORTION_GAIN_MIN         (0.0)
#define  TUBE_DISTORTION_GAIN_MAX         (4.0)

/**
 * @brief Initializes instance of a tube distortion
 *
 * @param c Pointer to instance structure
 * @param drive Signal gain going into the clipper (> 0)
 * @param gain Signal gain after the clipper (> 0)
 * @param contour Filter shape of output filter
 * @param audio_sample_rate The system audio sample rate
 * @return Tube distortion result (enumeration)
 */
RESULT_TUBE_DISTORTION tube_distortion_setup(TUBE_DISTORTION * c, float drive,
		float gain, float contour, float audio_sample_rate) {

	if (c == NULL) {
		return TUBE_DISTORTION_INVALID_INSTANCE_POINTER;
	}

	c->initialized = false;

	// Where the clipping occurs
	c->threshold = 0.2;
	clipper_setup(&c->clipper, c->threshold, POLY_SMOOTHERSTEP, true);

	// Check input parameters
	if (contour > TUBE_DISTORTION_CONTOUR_MAX
			|| contour < TUBE_DISTORTION_CONTOUR_MIN) {
		return TUBE_DISTORTION_INVALID_CONTOUR;
	}

	if (gain < TUBE_DISTORTION_GAIN_MIN || gain > TUBE_DISTORTION_GAIN_MAX) {
		return TUBE_DISTORTION_INVALID_GAIN;
	}

	if (drive < TUBE_DISTORTION_DRIVE_MIN || drive > TUBE_DISTORTION_DRIVE_MAX) {
		return TUBE_DISTORTION_INVALID_DRIVE;
	}

	filter_setup(&c->input_filter, BIQUAD_TYPE_BPF, BIQUAD_TRANS_MED,
			(pm float *) c->input_filter_coeffs, 600.0, 1.0, 1.0,
			audio_sample_rate);

	filter_setup(&c->output_filter, BIQUAD_TYPE_BPF, BIQUAD_TRANS_MED,
			(pm float *) c->output_filter_coeffs, 600.0 + 600.0 * contour, 1.5,
			1.0, audio_sample_rate);

	c->gain = gain;

	filter_modify_freq(&c->output_filter, 600.0 + 600.0 * contour);

	// Instance was successfully initialized
	c->initialized = true;
	return TUBE_DISTORTION_OK;

}

/**
 * @brief Modify output gain parameter
 *
 * @param c Pointer to instance structure
 * @param gain New gain value (0.1->4.0)
 * @return Tube distortion result (enumeration)
 */
RESULT_TUBE_DISTORTION tube_distortion_modify_gain(TUBE_DISTORTION * c,
		float gain_new) {

	RESULT_TUBE_DISTORTION res;

	float gain;
	if (gain_new < TUBE_DISTORTION_GAIN_MIN) {
		gain = TUBE_DISTORTION_GAIN_MIN;
		res = TUBE_DISTORTION_INVALID_GAIN;
	} else if (gain_new > TUBE_DISTORTION_GAIN_MAX) {
		gain = TUBE_DISTORTION_GAIN_MAX;
		res = TUBE_DISTORTION_INVALID_GAIN;
	} else {
		gain = gain_new;
		res = TUBE_DISTORTION_OK;
	}

	// Update parameter in instance
	c->gain = gain;

	return res;

}

/**
 * @brief Modify drive parameter
 *
 * If the input parameter is out of bounds, it is clipped to the corresponding
 * min/max value.  This function will return a value indicating an
 * invalid input parameter was supplied but the effect will continue to operate.
 *
 * @param c Pointer to instance structure
 * @param drive New drive value (0.1->128.0)
 * @return Tube distortion result (enumeration)
 */
RESULT_TUBE_DISTORTION tube_distortion_modify_drive(TUBE_DISTORTION * c,
		float drive_new) {

	RESULT_TUBE_DISTORTION res;

	float drive;
	if (drive_new < TUBE_DISTORTION_DRIVE_MIN) {
		drive = TUBE_DISTORTION_DRIVE_MIN;
		res = TUBE_DISTORTION_INVALID_DRIVE;
	} else if (drive_new > TUBE_DISTORTION_DRIVE_MAX) {
		drive = TUBE_DISTORTION_DRIVE_MAX;
		res = TUBE_DISTORTION_INVALID_DRIVE;
	} else {
		drive = drive_new;
		res = TUBE_DISTORTION_OK;
	}

	// Update parameter in instance
	c->drive = drive;

	return res;
}

/**
 * @brief Modify threshold parameter
 *
 * If the input parameter is out of bounds, it is clipped to the corresponding
 * min/max value.  This function will return a value indicating an
 * invalid input parameter was supplied but the effect will continue to operate.
 *
 * @param c Pointer to instance structure
 * @param threshold New threshold value (0.0->1.0)
 * @return Tube distortion result (enumeration)
 */
RESULT_TUBE_DISTORTION tube_distortion_modify_threshold(TUBE_DISTORTION * c,
		float threshold_new) {

	RESULT_TUBE_DISTORTION res;

	float threshold;
	if (threshold_new < TUBE_DISTORTION_THRESHOLD_MIN) {
		threshold = TUBE_DISTORTION_THRESHOLD_MIN;
		res = TUBE_DISTORTION_INVALID_THRESHOLD;
	} else if (threshold_new > TUBE_DISTORTION_THRESHOLD_MAX) {
		threshold = TUBE_DISTORTION_THRESHOLD_MAX;
		res = TUBE_DISTORTION_INVALID_THRESHOLD;
	} else {
		threshold = threshold_new;
		res = TUBE_DISTORTION_OK;
	}

	// Update parameter in instance
	c->threshold = threshold;
	c->clipper.clip_threshold = threshold;

	return res;
}

/**
 * @brief Modify contour parameter
 *
 * If the input parameter is out of bounds, it is clipped to the corresponding
 * min/max value.  This function will return a value indicating an
 * invalid input parameter was supplied but the effect will continue to operate.
 *
 * @param c Pointer to instance structure
 * @param contour New contour value (0.0 -> 1.0)
 * @return Tube distortion result (enumeration)
 */
RESULT_TUBE_DISTORTION tube_distortion_modify_contour(TUBE_DISTORTION * c,
		float contour_new) {

	RESULT_TUBE_DISTORTION res;

	float contour;
	if (contour_new < TUBE_DISTORTION_CONTOUR_MIN) {
		contour = TUBE_DISTORTION_CONTOUR_MIN;
		res = TUBE_DISTORTION_INVALID_CONTOUR;
	} else if (contour_new > TUBE_DISTORTION_CONTOUR_MAX) {
		contour = TUBE_DISTORTION_CONTOUR_MAX;
		res = TUBE_DISTORTION_INVALID_CONTOUR;
	} else {
		contour = contour_new;
		res = TUBE_DISTORTION_OK;
	}

	// Update parameter in instance
	filter_modify_freq(&c->output_filter, 600.0 + 1200.0 * contour);

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
void tube_distortion_read(TUBE_DISTORTION * c, float * audio_in,
		float * audio_out, uint32_t audio_block_size) {

	// If this instance hasn't been properly initialized, pass audio through
	if (c == NULL || !c->initialized) {
		for (int i = 0; i < audio_block_size; i++) {
			audio_out[i] = audio_in[i];
		}
		return;
	}

	float temp_audio_1[MAX_AUDIO_BLOCK_SIZE],
			temp_audio_2[MAX_AUDIO_BLOCK_SIZE];

	// Apply input filter
	filter_read(&c->input_filter, audio_in, temp_audio_1, audio_block_size);

	// Apply output gain
	for (int i = 0; i < audio_block_size; i++) {
		audio_in[i] *= c->drive;
	}

	// Apply clipping
	clipper_read(&c->clipper, audio_in, audio_out, audio_block_size);

	// Apply output gain
	for (int i = 0; i < audio_block_size; i++) {
		audio_out[i] *= c->gain;
	}

	// Apply output filter
	filter_read(&c->output_filter, audio_out, audio_out, audio_block_size);

}
