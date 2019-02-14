/*
 * Copyright (c) 2018 Analog Devices, Inc.  All rights reserved.
 *
 * See .c file for more information.
 */

#ifndef _AUDIO_FRAMEWORK_16CH_SAM_AND_AUTOMOTIVE_FIN_ARM_H
#define _AUDIO_FRAMEWORK_16CH_SAM_AND_AUTOMOTIVE_FIN_ARM_H

// Driver for GPIO functionality
#include "drivers/bm_gpio_driver/bm_gpio.h"

// LEDs on SHARC Audio Module
#define    GPIO_SHARC_SAM_LED10     BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_D, 1)
#define    GPIO_SHARC_SAM_LED11     BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_D, 2)
#define    GPIO_SHARC_SAM_LED12     BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_D, 3)

// Push buttons (PBs) on SHARC Audio Module
#define    GPIO_SHARC_SAM_PB1       BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_F, 0)
#define    GPIO_SHARC_SAM_PB2       BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_F, 1)

// TWI mux on v1.4 SAM boards and later
#define    GPIO_SAM_TWI_MUX         BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_B, 8)

void audioframework_initialize(void);
void audioframework_wait_for_sharcs(void);
void audioframework_background_loop(void);

#endif    //_AUDIO_FRAMEWORK_16CH_SAM_AND_AUTOMOTIVE_FIN_ARM_H
