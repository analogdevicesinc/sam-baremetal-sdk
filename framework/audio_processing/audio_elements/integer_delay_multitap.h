/*
 * Copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 *
 * See .c file for documentation.
 */

#ifndef _INTEGER_DELAY_MULTITAP_H
#define _INTEGER_DELAY_MULTITAP_H

#include <stdint.h>
#include <stdbool.h>
#include "audio_elements_common.h"

#define     MULTITAP_DELAY_MAX_TAPS (32)

// Result enumerations
typedef enum {
	MT_DELAY_OK,
	MT_DELAY_INVALID_INSTANCE_POINTER,
	MT_DELAY_INVALID_DELAY_LINE_POINTER,
	MT_DELAY_INVALID_TAPS_POINTER,
	MT_DELAY_TOO_MANY_TAPS,
	MT_DELAY_TAP_EXCEEDS_DELAY_LINE_LEN
} RESULT_MT_DELAY;

// C struct with parameters and state information
typedef struct {
	bool initialized;

	float * delay_line;
	uint32_t tap_offsets[MULTITAP_DELAY_MAX_TAPS];
	float tap_gains[MULTITAP_DELAY_MAX_TAPS];
	uint32_t delay_line_size;
	uint32_t index;
	uint32_t num_taps;
	float feedthrough;
} MULTITAP_DELAY;

#if __cplusplus
extern "C" {
#endif

RESULT_MT_DELAY multitap_delay_setup(MULTITAP_DELAY * c, float * delay_line,
		uint32_t delay_line_size, uint32_t num_taps, uint32_t * tap_offsets,
		float * tap_gains, float feedthrough);

RESULT_MT_DELAY multitap_delay_modify_taps(MULTITAP_DELAY * c,
		uint32_t * new_tap_offsets);

void multitap_delay_read(MULTITAP_DELAY * c, float * audio_in,
		float * audio_out, uint32_t audio_block_size);

void multitap_delay_read_isolated(MULTITAP_DELAY * c, float * audio_in,
		float ** audio_outs, uint32_t audio_block_size);

#if __cplusplus
}
#endif

#endif  // _INTEGER_DELAY_MULTITAP_H
