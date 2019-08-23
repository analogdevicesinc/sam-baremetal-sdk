/*
 * Copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 *
 * See .c file for documentation.
 */

#ifndef _COMPRESSOR_H
#define _COMPRESSOR_H

#include <stdint.h>
#include <stdbool.h>

// Result enumerations
typedef enum {
	COMPRESSOR_OK,
	COMPRESSOR_INVALID_INSTANCE_POINTER,
	COMPRESSOR_INVALID_THRESHOLD,
	COMPRESSOR_INVALID_RATIO,
	COMPRESSOR_INVALID_ATTACK,
	COMPRESSOR_INVALID_RELEASE,
	COMPRESSOR_INVALID_GAIN
} RESULT_COMPRESSOR;

// Struct for LP filter
typedef struct {
	float ff;
	float fb;
} LP_COEFF;

// C struct instance with parameters and state information
typedef struct {

	bool initialized;

	float threshold_db;
	float threshold_db_last;
	float threshold_coeff;

	float output_gain;

	float ratio;
	float ratio_last;
	float ratio_coeff;

	float attack_ms;
	float attack_ms_last;

	float release_ms;
	float release_ms_last;

	LP_COEFF rms_coeff;
	LP_COEFF attack_coeff;
	LP_COEFF release_coeff;

	float cur_rms;

	float x2_last, x_ar_last;
	float audio_sample_rate;

} COMPRESSOR;

// Wrapper allows C code to be called from C++ files
#ifdef __cplusplus
extern "C" {
#endif

RESULT_COMPRESSOR compressor_setup(COMPRESSOR * c, float threshold_db,
		float ratio, float attack_ms, float release_ms, float gain,
		float audio_sample_rate);

RESULT_COMPRESSOR compressor_modify_threshold(COMPRESSOR * c,
		float threshold_db_new);

RESULT_COMPRESSOR compressor_modify_ratio(COMPRESSOR * c, float ratio_new);

RESULT_COMPRESSOR compressor_modify_attack(COMPRESSOR * c, float attack_ms_new);

RESULT_COMPRESSOR compressor_modify_release(COMPRESSOR * c,
		float release_ms_new);

RESULT_COMPRESSOR compressor_modify_gain(COMPRESSOR * c, float gain_new);

void compressor_read(COMPRESSOR * c, float * audio_in, float * audio_out,
		uint32_t audio_block_size);

// Wrapper allows C code to be called from C++ files
#ifdef __cplusplus
}
#endif

#endif  // _COMPRESSOR_H
