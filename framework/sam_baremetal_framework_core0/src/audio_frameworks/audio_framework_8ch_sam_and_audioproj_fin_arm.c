/*
 * Copyright (c) 2018 Analog Devices, Inc.  All rights reserved.
 *
 * This is a single-core audio processing framework that operates on audio data from
 * both the local ADAU1761 CODEC as well as the A2B bus.  It can be configured to be
 * a master or a slave node (in common/audio_system_config.h).
 *
 *    ADC -> SHARC 1 -> DAC
 *    A2B -> SHARC 1 -> A2B
 *
 * In all audio frameworks, the ARM core has the following responsibilities:
 *
 *   - Initializing any external components (ADCs, DACS, codecs, SigmaDSPs, A2B controllers)
 *   - Selecting the right SRU / DAI configuration to route data from these components
 *     to the right SPORTs within this chip.
 *   - Managing the audio sampling rate.
 *
 * This framework can be selected in common/audio_system_config.h.
 */

#include <stdio.h>

// Define your audio system parameters in this file
#include "common/audio_system_config.h"

#if defined(AUDIO_FRAMEWORK_8CH_SAM_AND_AUDIOPROJ_FIN) && AUDIO_FRAMEWORK_8CH_SAM_AND_AUDIOPROJ_FIN

#include "audio_framework_8ch_sam_and_audioproj_fin_arm.h"

// Structure containing shared variables between the three cores
#include "common/multicore_shared_memory.h"

#include "drivers/bm_sysctrl_driver/bm_system_control.h"

// Drivers for GPIO support
#include "drivers/bm_gpio_driver/bm_gpio.h"

// Drivers for A2B support
#include "drivers/bm_a2b_driver/bm_ad2425w.h"

// Drivers for serial EEPOM support (on A2B boards)
#include "drivers/bm_serial_eeprom_driver/bm_serial_eeprom.h"

// Drivers for ADAU / SigmaDSP Converters
#include "drivers/bm_adau_driver/bm_adau_device.h"

// Drivers for quick configuration of the SRU on the SHARC Audio Module board
#include "drivers/bm_sru_driver/bm_sru.h"

// Simple event logging / error handling functionality
#include "drivers/bm_event_logging_driver/bm_event_logging.h"

#include "../callback_midi_message.h"
#include "../callback_pushbuttons.h"

//*****************************************************************************
// A2B Configuration
//*****************************************************************************

// Enable A2B and select a topology in audio_system_config.h

#if defined(A2B_TOPOLOGY_TDM8_SAM_to_SAM_2up_2down) && (A2B_TOPOLOGY_TDM8_SAM_to_SAM_2up_2down && AUDIO_FRAMEWORK_8CH_SAM_AND_AUDIOPROJ_FIN)
#include "drivers/bm_a2b_driver/a2b_topologies/adi_a2b_i2c_commandlist-tdm8-SAM-SAM-peripheral-init.h"
#define A2B_PERIPHERAL_INIT_INCLUDED    TRUE

#elif defined(A2B_TOPOLOGY_TDM8_SAM_to_SAM_to_SAM_4up_4down) && (A2B_TOPOLOGY_TDM8_SAM_to_SAM_to_SAM_4up_4down && AUDIO_FRAMEWORK_8CH_SAM_AND_AUDIOPROJ_FIN)
#include "drivers/bm_a2b_driver/a2b_topologies/adi_a2b_i2c_commandlist-tdm8-sam-sam-sam-peripheral-init.h"
#define A2B_PERIPHERAL_INIT_INCLUDED    TRUE

#elif defined(A2B_TOPOLOGY_TDM8_SAM_to_CLASSD_4down) && (A2B_TOPOLOGY_TDM8_SAM_to_CLASSD_4down && AUDIO_FRAMEWORK_8CH_SAM_AND_AUDIOPROJ_FIN)
#include "drivers/bm_a2b_driver/a2b_topologies/adi_a2b_i2c_commandlist-tdm8-sam-classd-peripheral-init.h"
#define A2B_PERIPHERAL_INIT_INCLUDED    TRUE

// Add your own A2B configurations here

#endif

#if !defined(A2B_PERIPHERAL_INIT_INCLUDED)
#define A2B_PERIPHERAL_INIT_INCLUDED     FALSE
#endif

// Instance of our ADAU1761 driver for the ADAU1761 on this SHARC Audio Module board
BM_ADAU_DEVICE adau1761_local;

// Driver for the A2B controller on the SHARC Audio Module board
BM_AD2425W_CONTROLLER ad2425w;

#if ENABLE_A2B
/**
 * @brief      Callback for GPIO-over-disance
 *
 * A2B allows GPIO pins on remote nodes to be mapped back to the A2B controller on the
 * SHARC Audio Module.
 *
 */
void a2b_gpiod_callback(void *data_object) {

    // If using GPIOD (with A2B), respond to input flag change here
}
#endif

/**
 * @brief      Configures GPIO for the current platform
 *
 * This function sets the various GPIO pins available on the SHARC Audio Module as
 * well as the Audio Project fin.
 *
 */
void gpio_initialize(void) {

    // Set up LEDs on SHARC Audio Module board
    gpio_setup(GPIO_SHARC_SAM_LED10, GPIO_OUTPUT);    // Used for sign of life for ARM
    gpio_setup(GPIO_SHARC_SAM_LED11, GPIO_OUTPUT);    // used for sign of life for SHARC Core 1
    gpio_setup(GPIO_SHARC_SAM_LED12, GPIO_OUTPUT);    // Used for sign of life for SHARC Core 2

    // Set up the PBs as inputs on SHARC Audio Module board
    gpio_setup(GPIO_SHARC_SAM_PB1, GPIO_INPUT);
    gpio_setup(GPIO_SHARC_SAM_PB2, GPIO_INPUT);

    // Set PB08 low to route TWI0 to ADAU1761 and AD2425W
    gpio_setup(GPIO_SAM_TWI_MUX, GPIO_OUTPUT);
    gpio_write(GPIO_SAM_TWI_MUX, GPIO_LOW);

    // Attach a call back to each of these interrupts on the falling edge
    gpio_attach_interrupt(GPIO_SHARC_SAM_PB1, pushbutton_callback_sam_pb1, GPIO_FALLING, 0);
    gpio_attach_interrupt(GPIO_SHARC_SAM_PB2, pushbutton_callback_sam_pb1, GPIO_FALLING, 0);

    #if (SAM_AUDIOPROJ_FIN_BOARD_PRESENT)

		// Set Audio Project Fin PBs as inputs and attach handlers
		gpio_setup(GPIO_AUDIOPROJ_FIN_SW_1, GPIO_INPUT);
		gpio_setup(GPIO_AUDIOPROJ_FIN_SW_2, GPIO_INPUT);
		gpio_setup(GPIO_AUDIOPROJ_FIN_SW_3, GPIO_INPUT);
		gpio_setup(GPIO_AUDIOPROJ_FIN_SW_4, GPIO_INPUT);

		// Attach call back for these
		gpio_attach_interrupt(GPIO_AUDIOPROJ_FIN_SW_1, pushbutton_callback_external_1, GPIO_FALLING, 0);
		gpio_attach_interrupt(GPIO_AUDIOPROJ_FIN_SW_2, pushbutton_callback_external_2, GPIO_FALLING, 0);
		gpio_attach_interrupt(GPIO_AUDIOPROJ_FIN_SW_3, pushbutton_callback_external_3, GPIO_FALLING, 0);
		gpio_attach_interrupt(GPIO_AUDIOPROJ_FIN_SW_4, pushbutton_callback_external_4, GPIO_FALLING, 0);

		// Set Audio Project Fin SW LED pins as outputs
		gpio_setup(GPIO_AUDIOPROJ_FIN_LED_SW1, GPIO_OUTPUT);
		gpio_setup(GPIO_AUDIOPROJ_FIN_LED_SW2, GPIO_OUTPUT);
		gpio_setup(GPIO_AUDIOPROJ_FIN_LED_SW3, GPIO_OUTPUT);
		gpio_setup(GPIO_AUDIOPROJ_FIN_LED_SW4, GPIO_OUTPUT);

		// Set Audio Project Fin VU LED pins as outputs
		gpio_setup(GPIO_AUDIOPROJ_FIN_LED_VU1, GPIO_OUTPUT);
		gpio_setup(GPIO_AUDIOPROJ_FIN_LED_VU2, GPIO_OUTPUT);
		gpio_setup(GPIO_AUDIOPROJ_FIN_LED_VU3, GPIO_OUTPUT);
		gpio_setup(GPIO_AUDIOPROJ_FIN_LED_VU4, GPIO_OUTPUT);

		// Set Audio Project Fin Aux LED pins as outputs
		gpio_setup(GPIO_AUDIOPROJ_FIN_EXT_LED_1, GPIO_OUTPUT);
		gpio_setup(GPIO_AUDIOPROJ_FIN_EXT_LED_2, GPIO_OUTPUT);
		gpio_setup(GPIO_AUDIOPROJ_FIN_EXT_LED_3, GPIO_OUTPUT);
		gpio_setup(GPIO_AUDIOPROJ_FIN_EXT_LED_4, GPIO_OUTPUT);

		// If switches will be used to toggle FX, set initial state to false
		multicore_data->audioproj_fin_sw_1_state = false;
		multicore_data->audioproj_fin_sw_2_state = false;
		multicore_data->audioproj_fin_sw_3_state = false;
		multicore_data->audioproj_fin_sw_4_state = false;

    #endif



    #if ENABLE_A2B
		gpio_attach_interrupt(BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_F, 13), a2b_gpiod_callback, GPIO_FALLING, 0);
	#endif
}

/**
 * @brief      1ms tick event callback
 *
 * The framework has an optional 1ms tick event which includes a callback
 * for additional light weight processing.
 *
 */
void ms_tick_event_callback(void) {

    #if (SAM_AUDIOPROJ_FIN_BOARD_PRESENT)
		// Copy the latest read vales from the HADC into our shared memory struct
		// so SHARC cores can access too
		multicore_data->audioproj_fin_pot_hadc0 = hadc_read_float(SAM_AUDIOPROJ_FIN_POT_HADC0);
		multicore_data->audioproj_fin_pot_hadc1 = hadc_read_float(SAM_AUDIOPROJ_FIN_POT_HADC1);
		multicore_data->audioproj_fin_pot_hadc2 = hadc_read_float(SAM_AUDIOPROJ_FIN_POT_HADC2);
		multicore_data->audioproj_fin_aux_hadc3 = hadc_read_float(SAM_AUDIOPROJ_FIN_AUX_HADC3);
		multicore_data->audioproj_fin_aux_hadc4 = hadc_read_float(SAM_AUDIOPROJ_FIN_AUX_HADC4);
		multicore_data->audioproj_fin_aux_hadc5 = hadc_read_float(SAM_AUDIOPROJ_FIN_AUX_HADC5);
		multicore_data->audioproj_fin_aux_hadc6 = hadc_read_float(SAM_AUDIOPROJ_FIN_AUX_HADC6);
    #endif

	// Check to see if there are any event messages from the SHARC cores
    event_logging_poll_sharc_cores_for_new_message();
}

/**
 * @brief      ARM audio framework initialization function
 *
 * This function initializes any external components, selects the right
 * SRU / DAI configuration, and sets the sample rate.
 *
 */
void audioframework_initialize(void) {

    char message[128];

    /**
     * Set system-wide audio parameters in our shared memory structure between cores.
     * While sample rate is set initially via pre-processor variables, there may be
     * situations where they are modified on the fly.
     */

    multicore_data->audio_sample_rate = AUDIO_SAMPLE_RATE;
    multicore_data->audio_block_size = AUDIO_BLOCK_SIZE;
    multicore_data->core_clock_frequency = CORE_CLOCK_FREQ_HZ;

    log_event(EVENT_INFO, "System Configuration:");
    sprintf(message, "  Processor cores running at %'.2f MHz", (double)CORE_CLOCK_FREQ_HZ / 1000000.0);
    log_event(EVENT_INFO, message);
    sprintf(message, "  Audio sample rate set to %'.2f KHz", (double)AUDIO_SAMPLE_RATE / 1000.0);
    log_event(EVENT_INFO, message);
    sprintf(message, "  Audio block size (per channel) set to %d samples / frame", AUDIO_BLOCK_SIZE);
    log_event(EVENT_INFO, message);

    // Initialize GPIO
    gpio_initialize();

    // Set to false while we initialize the external audio components in our system
    multicore_data->arm_audio_peripheral_initialization_complete = false;

    // Initialize state variables and pointers for the audio framework
    multicore_data->sharc_core1_ready_for_audio = false;
    multicore_data->sharc_core2_ready_for_audio = false;

    /*
     * Attach a function to copy HADC values into our multicore memory structure
     * once every second
     */
    simple_sysctrl_set_1ms_callback(ms_tick_event_callback);

	// Determine which version of the Audio Project Fin is present
    #if (SAM_AUDIOPROJ_FIN_BOARD_PRESENT)

        multicore_data->audio_project_fin_present = true;

        #if SAM_AUDIOPROJ_FIN_BOARD_V3_02
            multicore_data->audioproj_fin_rev_3_20_or_later = false;
            log_event(EVENT_INFO, "  Framework configured for an earlier version of the Audio Project Fin (version 3.02)");
        #else
            multicore_data->audioproj_fin_rev_3_20_or_later = true;
            log_event(EVENT_INFO, "  Framework configured for Audio Project Fin version 3.2 or later");
        #endif    // SAM_AUDIOPROJ_FIN_BOARD_V3_02

    #else
        multicore_data->audio_project_fin_present = false;
    #endif    // SAM_AUDIOPROJ_FIN_BOARD_PRESENT

    // Set up the ADAU1761 as a master
    #if (A2B_ROLE_MASTER) || (!ENABLE_A2B)

    BM_ADAU_RESULT adau1761_result;

    log_event(EVENT_INFO, "Configuring the SRU - ADAU1761 is the I2S master");

    // Configure the DAI / SRU to use the ADAU1761 as an I2S clock/FS master to SPORT 0 and A2B to SPORT 1
    sru_config_sharc_sam_a2b_master();

    // Configure SPDIF to connect to SPORT2.  Divide the fs
    sru_config_spdif(4);

    log_event(EVENT_INFO, "Configuring the ADAU1761");
    // Initialize the ADAU1761 which is connected to TWI0 as a master
    #if USE_ENHANCED_ADAU1761_DRIVER

		log_event(EVENT_INFO, "  Using the 'enhanced audio' ADAU1761 init sequence");
		// see notes about enhanced driver in audio_system_config.h
		if ((adau1761_result = adau_initialize(&adau1761_local,
											   TWI0,
											   SAM_ADAU1761_I2C_ADDR,
											   &adau1761_8ch_i2s_enhanced_master,
											   ADAU1761_ADDR_BYTES) != ADAU_SUCCESS)) {
			if (adau1761_result == ADAU_CORRUPT_INIT_FILE) {
				log_event(EVENT_FATAL, "ADAU1761 failed to initialize properly due to a corrupt I2C initialization file");
			}
			else if (adau1761_result == ADAU_TWI_TIMEOUT_ERROR) {
				log_event(EVENT_FATAL, "ADAU1761 failed to initialize due to an I2C timeout during initialization");
			}
			else if (adau1761_result == ADAU_PLL_LOCK_TIMEOUT_ERROR) {
				log_event(EVENT_FATAL, "ADAU1761 failed to initialize because its PLL failed to lock");
			}
			else if (adau1761_result == ADAU_SIMPLE_ERROR) {
				log_event(EVENT_FATAL, "ADAU1761 failed to initialize because an initialization error occurred");
			}
		}
		else {
			log_event(EVENT_INFO, "  ADAU1761 successfully initialized over I2C");
	}

    #else
    log_event(EVENT_INFO, "  Using the standard (non-enhanced audio) ADAU1761 init sequence");
    if ((adau1761_result = adau_initialize(&adau1761_local,
                                           TWI0,
                                           SAM_ADAU1761_I2C_ADDR,
                                           &adau1761_8ch_i2s_master,
                                           ADAU1761_ADDR_BYTES) != ADAU_SUCCESS)) {

        if (adau1761_result == ADAU_CORRUPT_INIT_FILE) {
            log_event(EVENT_FATAL, "ADAU1761 failed to initialize properly due to a corrupt I2C initialization file");
        }
        else if (adau1761_result == ADAU_TWI_TIMEOUT_ERROR) {
            log_event(EVENT_FATAL, "ADAU1761 failed to initialize due to an I2C timeout during initialization");
        }
        else if (adau1761_result == ADAU_PLL_LOCK_TIMEOUT_ERROR) {
            log_event(EVENT_FATAL, "ADAU1761 failed to initialize because its PLL failed to lock");
        }
        else if (adau1761_result == ADAU_SIMPLE_ERROR) {
            log_event(EVENT_FATAL, "ADAU1761 failed to initialize because an initialization error occurred");
        }
    }
    else {
        log_event(EVENT_INFO, "  ADAU1761 successfully initialized over I2C");
    }
        #endif

    // Set codec sample rate
    log_event(EVENT_INFO, "  Setting sample rate");
    if (!adau1761_set_samplerate(&adau1761_local, AUDIO_SAMPLE_RATE)) {
        log_event(EVENT_FATAL, "  Failed to update the ADAU1761 sample rate");
    }

    // Confirm that the ADAU1761 is running (a good indication that it has been initialized properly)
    uint8_t sigmadspRunning;
    adau_read_ctrl_reg(&adau1761_local, ADAU1761_REG_DSP_RUN, &sigmadspRunning);
    if (!(sigmadspRunning & 0x1)) {
        log_event(EVENT_FATAL, "  The SigmaDSP core inside the ADAU1761 is not running");
    }

    uint8_t auxGain = 0x0;        // default is muted

    if (multicore_data->audio_project_fin_present) {

        if (multicore_data->audioproj_fin_rev_3_20_or_later) {
            // If newer board (>3.02) set aux gain to 0dB
            auxGain = 0x5;
        }
        else {
            // if 3.02 board, set aux gain to -6dB
            auxGain = 0x3;
        }
    }

    // Set the AUX channel gain depending on whether a Audio Project Fin is present, and which version of the board
    uint8_t currentRegVal;
    // Left channel AUX gain
    adau_read_ctrl_reg(&adau1761_local, ADAU1761_REG_REC_MIXER_LEFT_1, &currentRegVal);
    currentRegVal &= 0xF8;
    currentRegVal |= auxGain;
    adau_write_ctrl_reg(&adau1761_local, ADAU1761_REG_REC_MIXER_LEFT_1, currentRegVal);

    // Right channel AUX gain
    adau_read_ctrl_reg(&adau1761_local, ADAU1761_REG_REC_MIXER_RIGHT_1, &currentRegVal);
    currentRegVal &= 0xF8;
    currentRegVal |= auxGain;
    adau_write_ctrl_reg(&adau1761_local, ADAU1761_REG_REC_MIXER_RIGHT_1, currentRegVal);

    log_event(EVENT_INFO, "  ADAU1761 updated gain settings for the Audio Project Fin being used");
    log_event(EVENT_INFO, "  Complete");
    // Set up SRU to bypass the processor and route I2S from A2B to the ADAU1761
    // In this config, the ADAU1761 will be initialized from a remote master node
    #elif (!A2B_ROLE_MASTER)
    // Configure the DAI / SRU to use the A2B AD2425W as an I2S clock/FS master
    // In this mode, the A2B master will initialize the local ADAU161 over the A2b bus
    sru_config_sharc_sam_a2b_slave();
    log_event(EVENT_INFO, "A2B: Configuring the SHARC Audio Module as an A2B slave node");
    #endif

    // Initialize the A2B bus
    #if (ENABLE_A2B)
    BM_AD2425W_RESULT ad2425_result;

    log_event(EVENT_INFO, "Configuring A2B Bus");

        #if (A2B_ROLE_MASTER)

    log_event(EVENT_INFO, "  Role: A2B Master");

            #if defined(A2B_TOPOLOGY_TDM8_SAM_to_SAM_2up_2down) && (A2B_TOPOLOGY_TDM8_SAM_to_SAM_2up_2down && FRAMEWORK_8CH_SINGLE_OR_DUAL_CORE_A2B)
    log_event(EVENT_INFO, "  Topology: SAM-SAM ( 2 channels upstream / 2 channels downstream )");

            #elif defined(A2B_TOPOLOGY_TDM8_SAM_to_SAM_to_SAM_4up_4down) && (A2B_TOPOLOGY_TDM8_SAM_to_SAM_to_SAM_4up_4down && FRAMEWORK_8CH_SINGLE_OR_DUAL_CORE_A2B)
    log_event(EVENT_INFO, "  Topology: SAM-SAM-SAM ( 4 channels upstream / 4 channels downstream )");

            #elif defined(A2B_TOPOLOGY_TDM8_SAM_to_CLASSD_4down) && (A2B_TOPOLOGY_TDM8_SAM_to_CLASSD_4down && FRAMEWORK_8CH_SINGLE_OR_DUAL_CORE_A2B)
    log_event(EVENT_INFO, "  Topology: SAM-Class-D ( 4 channels downstream )");
            #endif

    // Initialize the AD2425W driver and chip as the master node
    if ((ad2425_result = ad2425w_initialize(&ad2425w,
                                            AD2425W_SIMPLE_MASTER,
                                            AD2425W_SAM_I2C_ADDR,
                                            0) != AD2425W_SIMPLE_SUCCESS)) {

        if (ad2425_result == AD2425W_A2B_BUS_ERROR) {
            log_event(EVENT_FATAL, "  A2B - a bus error was encountered while initializing the bus");
        }
        else if (ad2425_result == AD2425W_A2B_BUS_TIMEOUT) {
            log_event(EVENT_FATAL, "  A2B - A timeout occurred while initializing the bus");
        }
        else if (ad2425_result == AD2425W_SIMPLE_GPIO_SVCS_ERROR) {
            log_event(EVENT_FATAL, "  A2B - Error initializing GPIO used for GPIOD");
        }
        else if (ad2425_result == AD2425W_SIMPLE_ODD_I2C_ADDRESS_ERROR) {
            log_event(EVENT_FATAL, "  A2B - Attempting to initialize a remote I2C node with an odd I2C address (needs to be even / bit shifted)");
        }
        else if (ad2425_result == AD2425W_CORRUPT_INIT_FILE) {
            log_event(EVENT_FATAL, "  A2B - Init file used to initialize A2B (via I2C) is corrupt");
        }
        else if (ad2425_result == AD2425W_UNSUPPORTED_READ_WIDTH) {
            log_event(EVENT_FATAL, "  A2B - Init file has a multi-byte read command which isn't yet supported in this driver");
        }
        else if (ad2425_result == AD2425W_UNSUPPORTED_DATA_WIDTH) {
            log_event(EVENT_FATAL, "  A2B - Init file has a multi-byte data format which isn't yet supported in this driver");
        }
        else if (ad2425_result == AD2425W_SIMPLE_ERROR) {
            log_event(EVENT_FATAL, "  A2B - An error has occurred while initializing the A2B bus");
        }
    }
    else {
        log_event(EVENT_INFO, "  A2B Driver has been instantiated");
    }

    log_event(EVENT_INFO, "  Sending init sequence to initialize bus");

    // Initiate A2B bus configuration sequence
    if ((ad2425_result = ad2425w_load_init_sequence(&ad2425w,
                                                    (void *)&gaA2BConfig,
                                                    sizeof(gaA2BConfig),
                                                    NULL,
                                                    A2B_PERIPHERAL_INIT_INCLUDED) != AD2425W_SIMPLE_SUCCESS) ) {
        if (ad2425_result == AD2425W_A2B_BUS_ERROR) {
            log_event(EVENT_FATAL, "  A2B - a bus error was encountered while initializing the bus");
        }
        else if (ad2425_result == AD2425W_A2B_BUS_TIMEOUT) {
            log_event(EVENT_FATAL, "  A2B - A timeout occurred while initializing the bus");
        }
        else if (ad2425_result == AD2425W_SIMPLE_GPIO_SVCS_ERROR) {
            log_event(EVENT_FATAL, "  A2B - Error initializing GPIO used for GPIOD");
        }
        else if (ad2425_result == AD2425W_SIMPLE_ODD_I2C_ADDRESS_ERROR) {
            log_event(EVENT_FATAL, "  A2B - Attempting to initialize a remote I2C node with an odd I2C address (needs to be even / bit shifted)");
        }
        else if (ad2425_result == AD2425W_CORRUPT_INIT_FILE) {
            log_event(EVENT_FATAL, "  A2B - Init file used to initialize A2B (via I2C) is corrupt");
        }
        else if (ad2425_result == AD2425W_UNSUPPORTED_READ_WIDTH) {
            log_event(EVENT_FATAL, "  A2B - Init file has a multi-byte read command which isn't yet supported in this driver");
        }
        else if (ad2425_result == AD2425W_UNSUPPORTED_DATA_WIDTH) {
            log_event(EVENT_FATAL, "  A2B - Init file has a multi-byte data format which isn't yet supported in this driver");
        }
        else if (ad2425_result == AD2425W_SIMPLE_ERROR) {
            log_event(EVENT_FATAL, "  A2B - An error has occurred while initializing the A2B bus");
        }
    }
    else {
        log_event(EVENT_INFO, "  A2B bus has been successfully initialized");
    }

        #else

    // Initialize the AD2425W driver and chip as a slave node
    if ((ad2425_result = ad2425w_initialize(&ad2425w,
                                            AD2425W_SIMPLE_SLAVE,
                                            AD2425W_SAM_I2C_ADDR,
                                            GPIO_SHARC_SAM_AD2425_IRQ,
                                            0) != AD2425W_SIMPLE_SUCCESS) ) {

        if (ad2425_result == AD2425W_A2B_BUS_ERROR) {
            log_event(EVENT_FATAL, "A2B: a bus error was encountered while initializing the bus");
        }
        else if (ad2425_result == AD2425W_A2B_BUS_TIMEOUT) {
            log_event(EVENT_FATAL, "A2B: A timeout occurred while initializing the bus");
        }
        else if (ad2425_result == AD2425W_SIMPLE_GPIO_SVCS_ERROR) {
            log_event(EVENT_FATAL, "A2B: Error initializing GPIO used for GPIOD");
        }
        else if (ad2425_result == AD2425W_SIMPLE_ODD_I2C_ADDRESS_ERROR) {
            log_event(EVENT_FATAL, "A2B: Attempting to initialize a remote I2C node with an odd I2C address (needs to be even / bit shifted)");
        }
        else if (ad2425_result == AD2425W_CORRUPT_INIT_FILE) {
            log_event(EVENT_FATAL, "A2B: Init file used to initialize A2B (via I2C) is corrupt");
        }
        else if (ad2425_result == AD2425W_UNSUPPORTED_READ_WIDTH) {
            log_event(EVENT_FATAL, "A2B: Init file has a multi-byte read command which isn't yet supported in this driver");
        }
        else if (ad2425_result == AD2425W_UNSUPPORTED_DATA_WIDTH) {
            log_event(EVENT_FATAL, "A2B: Init file has a multi-byte data format which isn't yet supported in this driver");
        }
        else if (ad2425_result == AD2425W_SIMPLE_ERROR) {
            log_event(EVENT_FATAL, "A2B: An error has occurred while initializing the A2B bus");
        }
    }
    else {
        log_event(EVENT_INFO, "A2B: Driver has been instantiated");
    }

        #endif // A2B_ROLE_MASTER

    #endif // ENABLE_A2B

    // Initialize the MIDI / UART interface if the ARM is to be used for MIDI
    #if defined(MIDI_UART_MANAGED_BY_ARM_CORE) && MIDI_UART_MANAGED_BY_ARM_CORE
    midi_setup_arm();
    #endif    // MIDI_RUNS_ON_ARM_CORE

    // The ARM is all ready to go!
    multicore_data->arm_audio_peripheral_initialization_complete = true;
}

void audioframework_wait_for_sharcs(void) {

    volatile uint32_t timeout_timer = 100000;

    // Ensure both cores get kicked off properly
    while (!multicore_data->sharc_core1_processing_audio && timeout_timer--);

    if (!timeout_timer) {
        log_event(EVENT_FATAL, "ARM core timed out while waiting for SHARC core 1 to start");
    }

    #if (USE_BOTH_CORES_TO_PROCESS_AUDIO)
    timeout_timer = 100000;
    while (!multicore_data->sharc_core2_processing_audio && timeout_timer--);

    if (!timeout_timer) {
        log_event(EVENT_FATAL, "ARM core timed out while waiting for SHARC core 2 to start");
    }

    #endif
}

void audioframework_background_loop(void) {

    /**
     * The ARM controls LED10 on the SHARC SAM board.  This LED will strobe (on/off) once
     * per second.  The two SHARC cores control LED11 and LED12 respectively. In a
     * dual-core audio framework, both LED11 and LED12 will strobe.  In a single-core
     * framework, only LED11 will strobe.  LED11 and LED12 should also strobe at roughly
     * once per second.  If not, it indicates that the audio is running at a different
     * sampling rate than what is defined in the audio_system_config.h file.
     *
     * At this point, the ARM doesn't need to do much.  There is a 1 ms timer loop that runs
     * as part of drivers/sysctrl_simple that enables the Arduino-style delay() function
     * and it also copies the values of the HADC (housekeeping ADC) into the shared memory
     * structure.  In the case of the Audio Project Fin, this ensures that the values of the 3
     * pots on that board are always reflected and current in our shared memory structure.
     *
     */

    // ARM sign of life LED (once per second) once SHARC is running
    if (multicore_data->sharc_core1_led_strobed) {
        multicore_data->sharc_core1_led_strobed = false;

        // Toggle the ARM core LED
        gpio_toggle(GPIO_SHARC_SAM_LED10);




        // If we have A2B boards connected and GPIOD is configured in topology, toggle those remote GPIOD flags
        #if ENABLE_A2B
			gpio_toggle(BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_F, 8));
			gpio_toggle(BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_F, 9));
        #endif
    }
    // If the Audio Project Fin is attached, make a basic VU meter
    #if (SAM_AUDIOPROJ_FIN_BOARD_PRESENT)
    	if (multicore_data->audio_in_amplitude > -20.0) {
    		gpio_write(GPIO_AUDIOPROJ_FIN_LED_VU4, GPIO_HIGH);
    	} else {
    		gpio_write(GPIO_AUDIOPROJ_FIN_LED_VU4, GPIO_LOW);
    	}

    	if (multicore_data->audio_in_amplitude > -30.0) {
    		gpio_write(GPIO_AUDIOPROJ_FIN_LED_VU3, GPIO_HIGH);
    	} else {
    		gpio_write(GPIO_AUDIOPROJ_FIN_LED_VU3, GPIO_LOW);
    	}

    	if (multicore_data->audio_in_amplitude > -40.0) {
    		gpio_write(GPIO_AUDIOPROJ_FIN_LED_VU2, GPIO_HIGH);
    	} else {
    		gpio_write(GPIO_AUDIOPROJ_FIN_LED_VU2, GPIO_LOW);
    	}

    	if (multicore_data->audio_in_amplitude > -50.0) {
    		gpio_write(GPIO_AUDIOPROJ_FIN_LED_VU1, GPIO_HIGH);
    	} else {
    		gpio_write(GPIO_AUDIOPROJ_FIN_LED_VU1, GPIO_LOW);
    	}
	#endif     // SAM_AUDIOPROJ_FIN_BOARD_PRESENT

}

#endif    // FRAMEWORK_8CH_SINGLE_OR_DUAL_CORE_A2B
