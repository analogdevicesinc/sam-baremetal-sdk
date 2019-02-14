/*
 * Copyright (c) 2018 Analog Devices, Inc.  All rights reserved.
 *
 * This is a dual-core audio processing framework for use with the automotive expander board.
 * This board has 16 DAC channels and 8 ADC channels.  It also has A2B and SPDIF.
 * Routing of the various signals on this board is done via the ADAU1452 SigmaDSP.
 *
 *       The data flow for this framework is
 *    ADC -> SHARC 1 -> SHARC 2 -> DAC
 *
 */

#ifndef _AUDIO_FRAMEWORK_16CH_SAM_AND_AUTOMOTIVE_FIN_CORE2_H
#define _AUDIO_FRAMEWORK_16CH_SAM_AND_AUTOMOTIVE_FIN_CORE2_H

// Driver for GPIO functionality
#include "drivers/bm_gpio_driver/bm_gpio.h"

// LEDs on SHARC Audio Module
#define    GPIO_SHARC_SAM_LED10     BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_D, 1)
#define    GPIO_SHARC_SAM_LED11     BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_D, 2)
#define    GPIO_SHARC_SAM_LED12     BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_D, 3)

// Push buttons (PBs) on SHARC Audio Module
#define    GPIO_SHARC_SAM_PB1       BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_F, 0)
#define    GPIO_SHARC_SAM_PB2       BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_F, 1)

#ifdef __cplusplus
extern "C" {
#endif

// Floating point audio buffers
extern float *audiochannel_0_left_in;
extern float *audiochannel_0_right_in;
extern float *audiochannel_1_left_in;
extern float *audiochannel_1_right_in;
extern float *audiochannel_2_left_in;
extern float *audiochannel_2_right_in;
extern float *audiochannel_3_left_in;
extern float *audiochannel_3_right_in;
extern float *audiochannel_4_left_in;
extern float *audiochannel_4_right_in;
extern float *audiochannel_5_left_in;
extern float *audiochannel_5_right_in;
extern float *audiochannel_6_left_in;
extern float *audiochannel_6_right_in;
extern float *audiochannel_7_left_in;
extern float *audiochannel_7_right_in;

extern float *audiochannel_0_left_out;
extern float *audiochannel_0_right_out;
extern float *audiochannel_1_left_out;
extern float *audiochannel_1_right_out;
extern float *audiochannel_2_left_out;
extern float *audiochannel_2_right_out;
extern float *audiochannel_3_left_out;
extern float *audiochannel_3_right_out;
extern float *audiochannel_4_left_out;
extern float *audiochannel_4_right_out;
extern float *audiochannel_5_left_out;
extern float *audiochannel_5_right_out;
extern float *audiochannel_6_left_out;
extern float *audiochannel_6_right_out;
extern float *audiochannel_7_left_out;
extern float *audiochannel_7_right_out;

// Function prototypes
void audioframework_initialize(void);
void audioframework_start(void);

#ifdef __cplusplus
}
#endif

#endif // _AUDIO_FRAMEWORK_16CH_SAM_AND_AUTOMOTIVE_FIN_CORE2_H
