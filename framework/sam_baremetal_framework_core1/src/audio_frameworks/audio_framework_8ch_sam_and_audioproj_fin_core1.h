/*
 * Copyright (c) 2018 Analog Devices, Inc.  All rights reserved.
 */
#ifndef _AUDIO_FRAMEWORK_8CH_SAM_AND_AUDIOPROJ_FIN_H
#define _AUDIO_FRAMEWORK_8CH_SAM_AND_AUDIOPROJ_FIN_H

#if defined(USE_FAUST_ALGORITHM_CORE1) && USE_FAUST_ALGORITHM_CORE1
#include "audio_framework_faust_extension_core1.h"
#endif

#include <math.h>
#include <stddef.h>
#include <stdint.h>

// Driver for GPIO functionality
#include "drivers/bm_gpio_driver/bm_gpio.h"

#if defined(SAM_AUDIOPROJ_FIN_BOARD_PRESENT) && SAM_AUDIOPROJ_FIN_BOARD_PRESENT

// LEDs by PCB designator
#define    GPIO_AUDIOPROJ_FIN_LED_3       BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_E, 0)
#define    GPIO_AUDIOPROJ_FIN_LED_4       BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_E, 1)
#define    GPIO_AUDIOPROJ_FIN_LED_5       BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_E, 2)
#define    GPIO_AUDIOPROJ_FIN_LED_6       BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_E, 3)
#define    GPIO_AUDIOPROJ_FIN_LED_7       BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_E, 4)
#define    GPIO_AUDIOPROJ_FIN_LED_8       BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_E, 5)
#define    GPIO_AUDIOPROJ_FIN_LED_9       BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_E, 6)
#define    GPIO_AUDIOPROJ_FIN_LED_10      BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_E, 7)

// LEDs beneath the Switches / push buttons
#define    GPIO_AUDIOPROJ_FIN_LED_SW1     BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_E, 3)
#define    GPIO_AUDIOPROJ_FIN_LED_SW2     BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_E, 2)
#define    GPIO_AUDIOPROJ_FIN_LED_SW3     BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_E, 1)
#define    GPIO_AUDIOPROJ_FIN_LED_SW4     BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_E, 0)

// Four VU (volume unit) LEDs in center of Audio Project Fin
#define    GPIO_AUDIOPROJ_FIN_LED_VU1     BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_E, 4)
#define    GPIO_AUDIOPROJ_FIN_LED_VU2     BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_E, 5)
#define    GPIO_AUDIOPROJ_FIN_LED_VU3     BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_E, 6)
#define    GPIO_AUDIOPROJ_FIN_LED_VU4     BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_E, 7)

// External LED 100mil header (P15-P12)
#define    GPIO_AUDIOPROJ_FIN_EXT_LED_1   BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_D, 4)
#define    GPIO_AUDIOPROJ_FIN_EXT_LED_2   BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_D, 5)
#define    GPIO_AUDIOPROJ_FIN_EXT_LED_3   BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_D, 6)
#define    GPIO_AUDIOPROJ_FIN_EXT_LED_4   BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_D, 7)

// Switches / push buttons
#define    GPIO_AUDIOPROJ_FIN_SW_1        BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_E, 8)
#define    GPIO_AUDIOPROJ_FIN_SW_2        BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_E, 9)
#define    GPIO_AUDIOPROJ_FIN_SW_3        BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_E, 10)
#define    GPIO_AUDIOPROJ_FIN_SW_4        BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_E, 12)

#define    GPIO_AUDIOPROJ_FIN_PB_1        BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_E, 8)
#define    GPIO_AUDIOPROJ_FIN_PB_2        BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_E, 9)
#define    GPIO_AUDIOPROJ_FIN_PB_3        BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_E, 10)
#define    GPIO_AUDIOPROJ_FIN_PB_4        BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_E, 12)

#endif // __AUDIOPROJ_DAUGHTER_BOARD__

// LEDs on SHARC Audio Module
#define    GPIO_SHARC_SAM_LED10     BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_D, 1)
#define    GPIO_SHARC_SAM_LED11     BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_D, 2)
#define    GPIO_SHARC_SAM_LED12     BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_D, 3)

// Push buttons (PBs) on SHARC Audio Module
#define    GPIO_SHARC_SAM_PB1       BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_F, 0)
#define    GPIO_SHARC_SAM_PB2       BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_F, 1)

// TWI mux on v1.4 SAM boards and later
#define    GPIO_SAM_TWI_MUX         BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_B, 8)

#ifdef __cplusplus
extern "C" {
#endif

// ADAU1761 Floating point audio buffers
extern float *audiochannel_adau1761_0_left_in;
extern float *audiochannel_adau1761_0_right_in;
extern float *audiochannel_adau1761_1_left_in;
extern float *audiochannel_adau1761_1_right_in;
extern float *audiochannel_adau1761_2_left_in;
extern float *audiochannel_adau1761_2_right_in;
extern float *audiochannel_adau1761_3_left_in;
extern float *audiochannel_adau1761_3_right_in;

extern float *audiochannel_adau1761_0_left_out;
extern float *audiochannel_adau1761_0_right_out;
extern float *audiochannel_adau1761_1_left_out;
extern float *audiochannel_adau1761_1_right_out;
extern float *audiochannel_adau1761_2_left_out;
extern float *audiochannel_adau1761_2_right_out;
extern float *audiochannel_adau1761_3_left_out;
extern float *audiochannel_adau1761_3_right_out;

// A2B Floating point audio buffers
extern float *audiochannel_a2b_0_left_in;
extern float *audiochannel_a2b_0_right_in;
extern float *audiochannel_a2b_1_left_in;
extern float *audiochannel_a2b_1_right_in;
extern float *audiochannel_a2b_2_left_in;
extern float *audiochannel_a2b_2_right_in;
extern float *audiochannel_a2b_3_left_in;
extern float *audiochannel_a2b_3_right_in;

extern float *audiochannel_a2b_0_left_out;
extern float *audiochannel_a2b_0_right_out;
extern float *audiochannel_a2b_1_left_out;
extern float *audiochannel_a2b_1_right_out;
extern float *audiochannel_a2b_2_left_out;
extern float *audiochannel_a2b_2_right_out;
extern float *audiochannel_a2b_3_left_out;
extern float *audiochannel_a2b_3_right_out;

// SPDIF digital audio in buffers
extern float *audiochannel_spdif_0_left_in;
extern float *audiochannel_spdif_0_right_in;

// SPDIF digital audio out buffers
extern float *audiochannel_spdif_0_left_out;
extern float *audiochannel_spdif_0_right_out;

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

// Processed audio data from SHARC Core 1 for SHARC Core 2
extern float *audiochannel_to_sharc_core2_0_left;
extern float *audiochannel_to_sharc_core2_0_right;
extern float *audiochannel_to_sharc_core2_1_left;
extern float *audiochannel_to_sharc_core2_1_right;
extern float *audiochannel_to_sharc_core2_2_left;
extern float *audiochannel_to_sharc_core2_2_right;
extern float *audiochannel_to_sharc_core2_3_left;
extern float *audiochannel_to_sharc_core2_3_right;

#endif    // USE_BOTH_CORES_TO_PROCESS_AUDIO

// Alias pointers
extern float *audiochannel_0_left_in;
extern float *audiochannel_0_right_in;
extern float *audiochannel_1_left_in;
extern float *audiochannel_1_right_in;
extern float *audiochannel_2_left_in;
extern float *audiochannel_2_right_in;
extern float *audiochannel_3_left_in;
extern float *audiochannel_3_right_in;

extern float *audiochannel_0_left_out;
extern float *audiochannel_0_right_out;
extern float *audiochannel_1_left_out;
extern float *audiochannel_1_right_out;
extern float *audiochannel_2_left_out;
extern float *audiochannel_2_right_out;
extern float *audiochannel_3_left_out;
extern float *audiochannel_3_right_out;

// Total number of audio blocks processed
extern uint32_t audio_blocks_processed_count;

void audioframework_initialize(void);
void audioframework_start(void);

#ifdef __cplusplus
}
#endif

#endif     // _AUDIO_FRAMEWORK_8CH_SAM_AND_AUDIOPROJ_FIN_H
