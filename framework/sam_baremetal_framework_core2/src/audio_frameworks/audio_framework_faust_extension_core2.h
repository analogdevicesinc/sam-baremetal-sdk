/*
 * Copyright (c) 2018 Analog Devices, Inc.  All rights reserved.
 */

#ifndef _AUDIO_FRAMEWORK_FAUST_EXTENSION_H
#define _AUDIO_FRAMEWORK_FAUST_EXTENSION_H

#ifdef __cplusplus
extern "C" {
#endif

// Input and output buffers for Faust
extern float audioChannel_faust_0_left_in[];
extern float audioChannel_faust_0_right_in[];
extern float audioChannel_faust_1_left_in[];
extern float audioChannel_faust_1_right_in[];
extern float audioChannel_faust_2_left_in[];
extern float audioChannel_faust_2_right_in[];
extern float audioChannel_faust_3_left_in[];
extern float audioChannel_faust_3_right_in[];

extern float audioChannel_faust_0_left_out[];
extern float audioChannel_faust_0_right_out[];
extern float audioChannel_faust_1_left_out[];
extern float audioChannel_faust_1_right_out[];
extern float audioChannel_faust_2_left_out[];
extern float audioChannel_faust_2_right_out[];
extern float audioChannel_faust_3_left_out[];
extern float audioChannel_faust_3_right_out[];

// Initializes the Faust framework
void faust_initialize(void);

// Performs block-based processing
void Faust_audio_processing(void);

#ifdef __cplusplus
}
#endif

#endif
