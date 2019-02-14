/*
 * Copyright (c) 2018 Analog Devices, Inc.  All rights reserved.
 */

#include <stdio.h>

#include "startup_code_core2.h"

#include "adi_initialize.h"

// Contains system configuration defines
#include "common/audio_system_config.h"

// Multicore data sharing
#include "common/multicore_shared_memory.h"

// Device drivers
#include "drivers/bm_event_logging_driver/bm_event_logging.h"    // Simple event logging / error handling functionality
#include "drivers/bm_sysctrl_driver/bm_system_control.h"   // Simple system functionality (clocks, etc.)

#include "audio_framework_selector.h"

#include "callback_audio_processing.h"

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
        if (multicore_data->sharc_core2_dropped_audio_frames != dropped_audio_frames) {
            sprintf(message, "SHARC core 2 dropped %d audio frame(s) in the last second",
                    multicore_data->sharc_core2_dropped_audio_frames - dropped_audio_frames);
            log_event(EVENT_WARN, message);
            dropped_audio_frames = multicore_data->sharc_core2_dropped_audio_frames;
        }
    }
    if (second_counter % 5000 == 0) {
        sprintf(message, "SHARC core 2 processing peak load: %.2f MHz of %.1f MHz", multicore_data->sharc_core2_cpu_load_mhz_peak, cpu_speed);
        multicore_data->sharc_core2_cpu_load_mhz_peak = 0.0;
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
                            2      // This core will use Timer1 for its tick resource
                            ) != SYSCTRL_SUCCESS) {
        return -1;
    }

    // set a callback to the 1ms event
    simple_sysctrl_set_1ms_callback(timer_tick_callback);

    // Set up event logging
    event_logging_initialize_sharc_core(     (char *)multicore_data->sharc_core2_event_message,
                                             (uint32_t *)&multicore_data->sharc_core2_event_emuclk,
                                             (uint32_t *)&multicore_data->sharc_core2_event_emuclk2,
                                             (uint32_t *)&multicore_data->sharc_core2_event_level,
                                             (uint32_t *)&multicore_data->sharc_core2_new_message_ready);

    // If we're using a multicore framework, get audio going over here.
    #if (USE_BOTH_CORES_TO_PROCESS_AUDIO)

		log_event(EVENT_INFO, "SHARC Core 2 is running");

		// Set up our audio framework
		audioframework_initialize();
		log_event(EVENT_INFO, "Audio framework has been initialized");

		// Set up our audio processing algorithms in our audio processing callback
		processaudio_setup();

		// Kick off audio processing
		audioframework_start();
		log_event(EVENT_INFO, "Starting audio DMAs");

		log_event(EVENT_INFO, "...waiting for confirmation that audio DMA is running");
		while (!multicore_data->sharc_core2_processing_audio)
			continue;
		log_event(EVENT_INFO, "Audio DMA is running!");

		// If nothing else, wait here for interrupts
		while (1) {

			// Call our optional background audio processing loop
			processaudio_background_loop();
		}

    #else

		// And if we're not doing dual core processing, hang out in a while loop
		while (1) {}

    #endif
}
