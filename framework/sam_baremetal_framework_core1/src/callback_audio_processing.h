/*
 * Copyright (c) 2018 Analog Devices, Inc.  All rights reserved.
 */

#ifndef _CALLBACK_AUDIO_PROCESSING
#define _CALLBACK_AUDIO_PROCESSING

#ifdef __cplusplus
extern "C" {
#endif

void processaudio_setup(void);
void processaudio_callback(void);
void processaudio_background_loop(void);
void processaudio_output_routing(void);
void processaudio_mips_overflow(void);

#ifdef __cplusplus
}
#endif

#endif     // _CALLBACK_AUDIO_PROCESSING
