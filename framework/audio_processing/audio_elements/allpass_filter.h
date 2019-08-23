/*
 * Copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 *
 * See .c file for documentation.
 */

#ifndef _ALLPASS_FILTER_H
#define _ALLPASS_FILTER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "audio_elements_common.h"

// Result enumerations
typedef enum {
	ALLPASS_OK,
	ALLPASS_INVALID_INSTANCE_POINTER,
	ALLPASS_INVALID_DELAY_POINTER,
	ALLPASS_ERR_LENGTH_EXCEEDS_BUF_SIZE
} RESULT_ALLPASS_FILTER;

// Instance struct with parameters and state information
typedef struct {
	bool initialized;
	float * delay_line;
	uint32_t delay_line_size;
	uint32_t index;
	int length;
	float gain;
} ALLPASS_FILTER;

// Wrapper allows C code to be called from C++ files
#if __cplusplus
extern "C" {
#endif

// Function prototypes
RESULT_ALLPASS_FILTER allpass_setup(ALLPASS_FILTER * c, float * delay_buffer,
		uint32_t delay_buffer_size, float gain);

void allpass_read(ALLPASS_FILTER * C, float * audio_in, float * audio_out,
		uint32_t audio_block_size);

// Wrapper allows C code to be called from C++ files
#if __cplusplus
}
#endif

#endif  //_ALLPASS_FILTER_H
