/*
 * Copyright (c) 2018 Analog Devices, Inc.  All rights reserved.
 *
 * This framework directly connects the ADAU1761 to the AD2425W (A2B controller).
 *
 * This framework should be used while designing and debugging A2B topologies with
 * SigmaStudio since the AD2425W will need a set of I2S clocks before it can be
 * initialized.
 *
 * The ADAU1761 is initialized by SigmaStudio via the USBi connector.
 *
 * In all audio frameworks, the ARM core has the following responsibilities:
 *
 *   - Initializing any external components (ADCs, DACS, codecs, SigmaDSPs, A2B controllers)
 *   - Selecting the right SRU / DAI configuration to route data from these components
 *     to the right SPORTs within this chip.
 *   - Managing the audio sampling rate.
 *
 * This framework can be selected in common/audio_system_config.h.
 *
 * Revision history:
 *   1.0 - Initial release
 */

// Set audio system parameters in this file
#include "common/audio_system_config.h"

#if defined(AUDIO_FRAMEWORK_A2B_BYPASS_SC589) && AUDIO_FRAMEWORK_A2B_BYPASS_SC589

#include "audio_framework_a2b_bypass_sc589_arm.h"

// Drivers for quick configuration of the SRU on the SHARC Audio Module board
#include "drivers/bm_sru_driver/bm_sru.h"

// Delay function and system management
#include "drivers/bm_sysctrl_driver/bm_system_control.h"

/**
 * @brief      ARM audio framework initialization function
 *
 * This function initializes any external components, selects the right
 * SRU / DAI configuration, and sets the sample rate.
 *
 */
void audioframework_initialize(void) {

    // Set up LEDs on SHARC Audio Module board
    gpio_setup(GPIO_SHARC_SAM_LED10, GPIO_OUTPUT);    // Used for sign of life for ARM
    gpio_setup(GPIO_SHARC_SAM_LED11, GPIO_OUTPUT);    // used for sign of life for SHARC Core 1
    gpio_setup(GPIO_SHARC_SAM_LED12, GPIO_OUTPUT);    // Used for sign of life for SHARC Core 2

#if A2B_ROLE_MASTER
    sru_config_sharc_sam_a2b_passthrough_master();
#else
    sru_config_sharc_sam_a2b_passthrough_slave();
#endif // A2B_ROLE_MASTER
}

/**
 * @brief background loop for providing any framework processing
 */
void audioframework_background_loop(void) {

    // Toggle the ARM core LED
    gpio_toggle(GPIO_SHARC_SAM_LED10);

    delay(1000);
}

/**
 * @brief waits for SHARC processors to start running
 *
 * In the bypass framework, we don't use the SHARCs so this is a dummy function
 */
void audioframework_wait_for_sharcs(void) {

}


#endif    // FRAMEWORK_BYPASS_SC589_A2B
