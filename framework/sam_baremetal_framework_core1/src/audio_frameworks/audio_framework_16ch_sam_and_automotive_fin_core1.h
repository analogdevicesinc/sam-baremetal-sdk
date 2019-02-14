/*
 * Copyright (c) 2018 Analog Devices, Inc.  All rights reserved.
 *
 * See .c file for more information.
 *
 */

#ifndef _AUDIO_FRAMEWORK_16CH_SAM_AND_AUTOMOTIVE_FIN_CORE1_H
#define _AUDIO_FRAMEWORK_16CH_SAM_AND_AUTOMOTIVE_FIN_CORE1_H

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

extern float *adau1977_ch0_mic_in;
extern float *adau1977_ch1_mic_in;
extern float *adau1977_ch2_mic_in;
extern float *adau1977_ch3_mic_in;

extern float *adau1979_jack_j9_in_left;
extern float *adau1979_jack_j9_in_right;
extern float *adau1979_ch0_in;
extern float *adau1979_ch1_in;
extern float *adau1979_ch2_in;
extern float *adau1979_ch3_in;

extern float *adau1966_jack_j19_out_left;
extern float *adau1966_jack_j19_out_right;
extern float *adau1966_jack_j20_out_left;
extern float *adau1966_jack_j20_out_right;

#if (USE_BOTH_CORES_TO_PROCESS_AUDIO)

// Processed audio data from SHARC Core 2
extern float *audiochannel_from_sharc_core2_0_left;
extern float *audiochannel_from_sharc_core2_0_right;
extern float *audiochannel_from_sharc_core2_1_left;
extern float *audiochannel_from_sharc_core2_1_right;
extern float *audiochannel_from_sharc_core2_2_left;
extern float *audiochannel_from_sharc_core2_2_right;
extern float *audiochannel_from_sharc_core2_3_left;
extern float *audiochannel_from_sharc_core2_3_right;
extern float *audiochannel_from_sharc_core2_4_left;
extern float *audiochannel_from_sharc_core2_4_right;
extern float *audiochannel_from_sharc_core2_5_left;
extern float *audiochannel_from_sharc_core2_5_right;
extern float *audiochannel_from_sharc_core2_6_left;
extern float *audiochannel_from_sharc_core2_6_right;
extern float *audiochannel_from_sharc_core2_7_left;
extern float *audiochannel_from_sharc_core2_7_right;

// Processed audio data from SHARC Core 1 for SHARC Core 2
extern float *audiochannel_to_sharc_core2_0_left;
extern float *audiochannel_to_sharc_core2_0_right;
extern float *audiochannel_to_sharc_core2_1_left;
extern float *audiochannel_to_sharc_core2_1_right;
extern float *audiochannel_to_sharc_core2_2_left;
extern float *audiochannel_to_sharc_core2_2_right;
extern float *audiochannel_to_sharc_core2_3_left;
extern float *audiochannel_to_sharc_core2_3_right;
extern float *audiochannel_to_sharc_core2_4_left;
extern float *audiochannel_to_sharc_core2_4_right;
extern float *audiochannel_to_sharc_core2_5_left;
extern float *audiochannel_to_sharc_core2_5_right;
extern float *audiochannel_to_sharc_core2_6_left;
extern float *audiochannel_to_sharc_core2_6_right;
extern float *audiochannel_to_sharc_core2_7_left;
extern float *audiochannel_to_sharc_core2_7_right;

#endif

// Inputs from ADCs
extern float *audiochannel_automotive_0_left_in;
extern float *audiochannel_automotive_0_right_in;
extern float *audiochannel_automotive_1_left_in;
extern float *audiochannel_automotive_1_right_in;
extern float *audiochannel_automotive_2_left_in;
extern float *audiochannel_automotive_2_right_in;
extern float *audiochannel_automotive_3_left_in;
extern float *audiochannel_automotive_3_right_in;

// Inputs from other input channels
extern float *audiochannel_automotive_4_left_in;
extern float *audiochannel_automotive_4_right_in;
extern float *audiochannel_automotive_5_left_in;
extern float *audiochannel_automotive_5_right_in;
extern float *audiochannel_automotive_6_left_in;
extern float *audiochannel_automotive_6_right_in;
extern float *audiochannel_automotive_7_left_in;
extern float *audiochannel_automotive_7_right_in;

// Output to DACs
extern float *audiochannel_automotive_0_left_out;
extern float *audiochannel_automotive_0_right_out;
extern float *audiochannel_automotive_1_left_out;
extern float *audiochannel_automotive_1_right_out;
extern float *audiochannel_automotive_2_left_out;
extern float *audiochannel_automotive_2_right_out;
extern float *audiochannel_automotive_3_left_out;
extern float *audiochannel_automotive_3_right_out;
extern float *audiochannel_automotive_4_left_out;
extern float *audiochannel_automotive_4_right_out;
extern float *audiochannel_automotive_5_left_out;
extern float *audiochannel_automotive_5_right_out;
extern float *audiochannel_automotive_6_left_out;
extern float *audiochannel_automotive_6_right_out;
extern float *audiochannel_automotive_7_left_out;
extern float *audiochannel_automotive_7_right_out;

void audioframework_initialize(void);
void audioframework_start(void);

#ifdef __cplusplus
}
#endif

#endif     //_AUDIO_FRAMEWORK_16CH_SAM_AND_AUTOMOTIVE_FIN_CORE1_H
