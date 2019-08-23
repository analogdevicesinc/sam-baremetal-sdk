/*
 * Copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 */

#include <stdio.h>
#include <sys/adi_core.h>
#include <sys/platform.h>

#include "startup_code_core0.h"

#include "adi_initialize.h"

// Define your audio system parameters in this file
#include "common/audio_system_config.h"

// Structure containing shared variables between the three cores
#include "common/multicore_shared_memory.h"

// Device drivers
#include "drivers/bm_sysctrl_driver/bm_system_control.h"    // Basic system management functionality

// Simple event logging / error handling functionality
#include "drivers/bm_event_logging_driver/bm_event_logging.h"

// Audio processing framework support
#include "audio_framework_selector.h"

// And our call backs from processing UART / MIDI messages
#include "callback_midi_message.h"

/**
 * @brief If any FATAL or ERROR events are reported, this function gets called as an interrupt
 */
void event_logging_error_callback(uint32_t event, void *obj) {

    // Create a visual indication that things have gone terribly wrong
    gpio_setup(GPIO_SHARC_SAM_LED10, GPIO_OUTPUT);    // Used for sign of life for ARM
    gpio_setup(GPIO_SHARC_SAM_LED11, GPIO_OUTPUT);    // used for sign of life for SHARC Core 1
    gpio_setup(GPIO_SHARC_SAM_LED12, GPIO_OUTPUT);    // Used for sign of life for SHARC Core 2

    gpio_write(GPIO_SHARC_SAM_LED10, GPIO_HIGH);
    gpio_write(GPIO_SHARC_SAM_LED11, GPIO_LOW);
    gpio_write(GPIO_SHARC_SAM_LED12, GPIO_HIGH);

    // Strobe 4 times / second
    while (1) {
        gpio_toggle(GPIO_SHARC_SAM_LED10);
        gpio_toggle(GPIO_SHARC_SAM_LED11);
        gpio_toggle(GPIO_SHARC_SAM_LED12);
        delay(250);
    }
}

int main(int argc, const char *argv[]) {

    /**
     * Initialize managed drivers and/or services that have been added to
     * the project.
     * @return zero on success
     */
    adi_initComponents();

    /**
     * The default startup code does not include any functionality to allow
     * core 0 to enable core 1 and core 2. A convenient way to enable
     * core 1 and core 2 is to use the adi_core_enable function.
     */

    /*
     * ARM is responsible for setting up system clocks and managing HADC.  Also, timer tick
     * will use timer 0;
     */
    if (simple_sysctrl_init(EXT_OSCILLATOR_FREQ_HZ,
                            CORE_CLOCK_FREQ_HZ,
                            SYSTEM_CLOCK_FREQ_HZ,
                            SCK0_CLOCK_FREQ_HZ,
                            true, // This core will initialize the system clocks
                            true, // This core will control the HADC
                            true, // This core will have a 1ms timer tick event (supports delay and millis functions)
                            0     // This core will use Timer0 for its tick resource
                            ) != SYSCTRL_SUCCESS) {
        return -1;
    }

    // Initialize event log
    event_logging_initialize_arm((char *)&multicore_data->sharc_core1_event_message,
                                 (char *)&multicore_data->sharc_core2_event_message,
                                 (uint32_t *)&multicore_data->sharc_core1_event_emuclk,
                                 (uint32_t *)&multicore_data->sharc_core2_event_emuclk,
                                 (uint32_t *)&multicore_data->sharc_core1_event_emuclk2,
                                 (uint32_t *)&multicore_data->sharc_core2_event_emuclk2,
                                 (uint32_t *)&multicore_data->sharc_core1_event_level,
                                 (uint32_t *)&multicore_data->sharc_core2_event_level,
                                 (uint32_t *)&multicore_data->sharc_core1_new_message_ready,
                                 (uint32_t *)&multicore_data->sharc_core2_new_message_ready,
                                 (float)CORE_CLOCK_FREQ_HZ);

    // Send logged events to UART0 (p8 connector on the SHARC Audio Module)
    event_logging_connect_uart(UART_BAUD_RATE_115200,
                               UART_SERIAL_8N1,
                               UART_SAM_DEVICE_FTDI);

    // Set a callback for our fatal and error messages
    event_logging_set_error_callback(event_logging_error_callback);

    log_event(EVENT_INFO, "Baremetal Framework (version 2.0.0) for the ADI SHARC Audio Module");

    /*
     * Ensure that any shared memory structures we have using the
     * multicore_audio_simple code has not grown beyond the segments in which
     * it is supposed to live
     */

    if (!check_shared_memory_structure_sizes()) {
        // Multicore_shared_memory - structure defined in multicore_shared_memory.h file has grown too big
        log_event(EVENT_FATAL, "Structure defined in multicore_shared_memory.h file is too big");
    }

    // Initialize our selected the audio framework
    audioframework_initialize();

    // Initialize the effects presets
    multicore_data->total_effects_presets = 10;
    multicore_data->effects_preset = 0;
    multicore_data->reverb_preset = 0;

    #if defined(MIDI_UART_MANAGED_BY_ARM_CORE) && (MIDI_UART_MANAGED_BY_ARM_CORE)
    if (midi_setup_arm()) {
        log_event(EVENT_INFO, "SHARC Core 1 is configured to process MIDI");
    }
    else {
        log_event(EVENT_FATAL, "Error initializing the MIDI interface for SHARC Core 1!");
    }
    #endif

    // Start the cores
    log_event(EVENT_INFO, "Starting the SHARC cores...");
    adi_core_enable(ADI_CORE_SHARC0);
    adi_core_enable(ADI_CORE_SHARC1);
    log_event(EVENT_INFO, "Complete.");

    // Wait for both SHARC cores to init and start processing audio
    audioframework_wait_for_sharcs();

    // Call any background housekeeping functions here
    while (1) {
        audioframework_background_loop();
    }
}
