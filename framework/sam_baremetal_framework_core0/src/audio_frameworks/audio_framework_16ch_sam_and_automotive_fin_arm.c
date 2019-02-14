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
 * In all audio frameworks, the ARM core has the following responsibilities:
 *
 *   - Initializing any external components (ADCs, DACS, codecs, SigmaDSPs, A2B controllers)
 *   - Selecting the right SRU / DAI configuration to route data from these components
 *     to the right SPORTs within this chip.
 *   - Managing the audio sampling rate.
 *
 * This framework can be selected in common/audio_system_config.h.
 *
 */

#include <stdio.h>

// Define your audio system parameters in this file
#include "common/audio_system_config.h"

#if defined(AUDIO_FRAMEWORK_16CH_SAM_AND_AUTOMOTIVE_FIN) && AUDIO_FRAMEWORK_16CH_SAM_AND_AUTOMOTIVE_FIN

#include "audio_framework_16ch_sam_and_automotive_fin_arm.h"

#include "../callback_pushbuttons.h"

// Structure containing shared variables between the three cores
#include "common/multicore_shared_memory.h"

// Simple event logging / error handling functionality
#include "drivers/bm_event_logging_driver/bm_event_logging.h"

// Simple system functionality (clocks, delays, etc.)
#include "drivers/bm_sysctrl_driver/bm_system_control.h"

// GPIO functionality
#include "drivers/bm_gpio_driver/bm_gpio.h"

// Drivers for ADAU / SigmaDSP Converters
#include "drivers/bm_adau_driver/bm_adau_device.h"

// Drivers for quick configuration of the SRU on the SHARC Audio Module board
#include "drivers/bm_sru_driver/bm_sru.h"

// ADAU I2C driver
BM_ADAU_DEVICE adau1966_instance;
BM_ADAU_DEVICE adau1977_instance;
BM_ADAU_DEVICE adau1979_instance;

// The ADAU1542 uses a SPI interface and requires a slightly different type of driver
BM_ADAU1452_DEVICE adau1452_instance;

/**
 * @brief      Configures GPIO for the current platform
 *
 * This function sets the various GPIO pins available on the SHARC Audio Module
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
}

/**
 * @brief      1ms tick event callback
 *
 * The framework has an optional 1ms tick event which includes a callback
 * for additional light weight processing.
 *
 */
void ms_tick_event_callback() {

    // Check to see if there are any event messages from the SHARC cores
    event_logging_poll_sharc_cores_for_new_message();
}

/**
 * @brief      ARM audio framework initialization function
 *
 * This function initializes any external components, selects the right
 * SRU / DAI configuraiton, and sets the samping rate.
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

    // Configure the DAI / SRU to route the ADAU1452 to SPORT4 A/B
    sru_config_sharc_sam_adau1452_master();

    // Initialize the ADAU1452 as a 16 channel TDM master
    log_event(EVENT_INFO, "Initializing the ADAU1452");
    if (adau1452_initialize(&adau1452_instance,
                            BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_C, 12),
                            SPI0,
                            &adau1452_16ch_master) != ADAU_SUCCESS) {
        log_event(EVENT_ERROR, "Failed to initialize the ADAU1452");
    }

    // Initialize the ADAU1966 as a 16 channel I2S slave
    log_event(EVENT_INFO, "Initializing the ADAU1966");
    if (adau_initialize(&adau1966_instance,
                        TWI1,
                        AUTO_ADAU1966_I2C_ADDR,
                        &adau1966_8ch_dual_i2s_slave,
                        ADAU1966_ADDR_BYTES) != ADAU_SUCCESS) {

        log_event(EVENT_ERROR, "Failed to initialize the ADAU1966");
    }

    // Initialize the ADAU1977 as a 4 channel I2S slave
    log_event(EVENT_INFO, "Initializing the ADAU1977");

    if (adau_initialize(&adau1977_instance,
                        TWI1,
                        AUTO_ADAU1977_I2C_ADDR,
                        &adau1977_4ch_i2s_slave,
                        ADAU1977_ADDR_BYTES) != ADAU_SUCCESS) {
        log_event(EVENT_ERROR, "Failed to initialize the ADAU1977");
    }

    // Initialize the ADAU1979 as a 4 channel I2S slave (if it's there)

    /**
     * The ADAU1977 and the ADAU1979 are wired by default to exist at the same I2C address (0x11).
     * It's possible to hand-modify the automotive board to shift the address of the ADAU1979 to
     * 0x51.  The code below reads a known value from a reserved registers in the ADAU1979 using
     * an I2C address of 0x51.  If this read completes successfully, we know there the automotive
     * board has been modified and we'll initialize the ADAU1979 at that address.
     */

    BM_TWI adau1979test;

    log_event(EVENT_INFO, "Checking to see if the ADAU1979 I2C address has been modified");
    if (twi_initialize(&adau1979test, AUTO_ADAU1979_I2C_ADDR_MODIFIED, TWI_TYPICAL_SCLK0_FREQ, TWI1) != TWI_SIMPLE_SUCCESS) {
        log_event(EVENT_INFO, "Failed to initialize TWI");
    }
    uint8_t val = 0x0;
    // The control register at address 0x10 is reserved and has a reset value of 0x0F
    twi_write_r(&adau1979test, 0x10, true);
    twi_read(&adau1979test, &val);

    // If the read returned a value of 0x0F, it means the ADAU1979 I2C address is 0x51.
    if (val == 0x0F) {
        log_event(EVENT_INFO, "Found ADAU1979 with modified I2C address");

        if (adau_initialize(&adau1979_instance,
                            TWI1,
                            AUTO_ADAU1979_I2C_ADDR_MODIFIED,
                            &adau1979_4ch_i2s_slave,
                            ADAU1979_ADDR_BYTES) != ADAU_SUCCESS) {
            log_event(EVENT_ERROR, "Failed to initialize the ADAU1979");
        }
    }

    /**
     * Set system-wide audio parameters in our shared memory structure between cores.
     * While sample rate is set initially via pre-processor variables, there may be
     * situations where they are modified on the fly.
     */

    // The ARM is all ready to go!
    multicore_data->arm_audio_peripheral_initialization_complete = true;
}

void audioframework_wait_for_sharcs(void) {

    volatile uint32_t timeout_timer = 100000;

    // Ensure both cores get started properly
    while (!multicore_data->sharc_core1_processing_audio && timeout_timer--)
        continue;

    if (!timeout_timer) {
        log_event(EVENT_FATAL, "ARM core timed out while waiting for SHARC core 1 to start");
    }

    #if (USE_BOTH_CORES_TO_PROCESS_AUDIO)
    timeout_timer = 100000;
    while (!multicore_data->sharc_core2_processing_audio && timeout_timer--)
        continue;

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

        // If the Audio Project Fin is attached, strobe those LEDs too
        #if (SAM_AUDIOPROJ_FIN_BOARD_PRESENT)

        // And toggle all VU LEDs on the Audio Project Fin if connected
        gpio_toggle(GPIO_AUDIOPROJ_FIN_LED_VU1);
        gpio_toggle(GPIO_AUDIOPROJ_FIN_LED_VU2);
        gpio_toggle(GPIO_AUDIOPROJ_FIN_LED_VU3);
        gpio_toggle(GPIO_AUDIOPROJ_FIN_LED_VU4);

        #endif     // SAM_AUDIOPROJ_FIN_BOARD_PRESENT

        // If we have A2B boards connected and GPIOD is configured in topology, toggle those remote GPIOD flags
        #if ENABLE_A2B
        gpio_toggle(BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_F, 8));
        gpio_toggle(BM_GPIO_PORTPIN_MAKE(ADI_GPIO_PORT_F, 9));
        #endif
    }
}

#endif     //FRAMEWORK_16CH_SINGLE_OR_DUAL_CORE_AUTOMOTIVE
