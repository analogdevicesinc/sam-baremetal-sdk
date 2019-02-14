/*
 * Copyright (c) 2018 Analog Devices, Inc.  All rights reserved.
 */

#include <stdio.h>
#include <sys/adi_core.h>
#include <sys/platform.h>

#include "startup_code_core1.h"

#include "adi_initialize.h"

// Define your audio system parameters in this file
#include "common/audio_system_config.h"

// Simple multi-core data sharing scheme
#include "common/multicore_shared_memory.h"

// Simple system functionality (clocks, etc.)
#include "drivers/bm_sysctrl_driver/bm_system_control.h"

// Drivers for GPIO functionality
#include "drivers/bm_gpio_driver/bm_gpio.h"

// Drivers for uart / midi functionality
#include "drivers/bm_uart_driver/bm_uart.h"

// Event logging / error handling functionality
#include "drivers/bm_event_logging_driver/bm_event_logging.h"

// Include the audio framework
#include "audio_framework_selector.h"

// And our call backs from processing audio blocks and MIDI messages
#include "callback_audio_processing.h"
#include "callback_midi_message.h"

/**
 * If you want to use command program arguments, then place them in the following string.
 */
char __argv_string[] = "";

/**
 * @brief callback for 1ms timer event
 * @details Use 1ms timer event to manage the event logging system
 */
void timer_tick_callback(void) {

    char message[128];
    static uint32_t dropped_audio_frames = 0;
    static uint32_t second_counter = 1;
    float cpu_speed = CORE_CLOCK_FREQ_HZ/1000000;

    // If we have any messages queued up, send them
    event_logging_process_queue_sharc_core();

    // This is also a good place to alert us if we're dropping audio frames because our
    // callback processing is taking too long.
    if (second_counter % 1000 == 0) {
        if (multicore_data->sharc_core1_dropped_audio_frames != dropped_audio_frames) {
            sprintf(message, "SHARC core 1 dropped %d audio frame(s) in the last second",
                    multicore_data->sharc_core1_dropped_audio_frames - dropped_audio_frames);
            log_event(EVENT_WARN, message);
            dropped_audio_frames = multicore_data->sharc_core1_dropped_audio_frames;
        }
    }

    if (second_counter % 5000 == 0) {
        sprintf(message, "SHARC core 1 processing peak load: %.2f MHz of %.1f MHz", multicore_data->sharc_core1_cpu_load_mhz_peak, cpu_speed);
        multicore_data->sharc_core1_cpu_load_mhz_peak = 0.0;
        log_event(EVENT_INFO, message);
    }

    second_counter++;
}


int main(void){

    adi_initComponents();

    // Initialize 1ms housekeeping tick
    if (simple_sysctrl_init(EXT_OSCILLATOR_FREQ_HZ,
                            CORE_CLOCK_FREQ_HZ,
                            SYSTEM_CLOCK_FREQ_HZ,
                            SCK0_CLOCK_FREQ_HZ,
                            false, // This core not will initialize the system clocks
                            false, // This core not will control the HADC
                            true,  // This core will have a 1ms timer tick event (supports delay and millis functions)
                            1      // This core will use Timer1 for its tick resource
                            ) != SYSCTRL_SUCCESS) {
        return -1;
    }

    // set a callback to the 1ms event
    simple_sysctrl_set_1ms_callback(timer_tick_callback);

    // Set up event logging
    event_logging_initialize_sharc_core(     (char *)multicore_data->sharc_core1_event_message,
                                             (uint32_t *)&multicore_data->sharc_core1_event_emuclk,
                                             (uint32_t *)&multicore_data->sharc_core1_event_emuclk2,
                                             (uint32_t *)&multicore_data->sharc_core1_event_level,
                                             (uint32_t *)&multicore_data->sharc_core1_new_message_ready);

    log_event(EVENT_INFO, "SHARC Core 1 is running");

    // Initialize the audio framework
    audioframework_initialize();
    log_event(EVENT_INFO, "Audio framework has been initialized");

    // If we're routing MIDI events to the SHARC Core (done in audio_system_config.h), set up our MIDI interface
    #if defined(MIDI_UART_MANAGED_BY_SHARC1_CORE) && (MIDI_UART_MANAGED_BY_SHARC1_CORE)

		if (midi_setup_sharc1()) {
			log_event(EVENT_INFO, "SHARC Core 1 is configured to process MIDI");
		}
		else {
			log_event(EVENT_FATAL, "Error initializing the MIDI interface for SHARC Core 1!");
		}

    #endif

    // Set up our audio processing algorithms in our audio processing callback
    processaudio_setup();

    // Start Audio Framework
    audioframework_start();
    log_event(EVENT_INFO, "Starting audio DMAs");

    log_event(EVENT_INFO, "...waiting for confirmation that audio DMA is running");
    while (!multicore_data->sharc_core1_processing_audio)
        continue;
    log_event(EVENT_INFO, "Audio DMA is running!");

    // Wait for audio block interrupts
    while (1) {

        // Call our optional background audio processing loop
        processaudio_background_loop();
    }
}
