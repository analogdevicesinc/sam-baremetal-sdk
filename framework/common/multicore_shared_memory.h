/*
 * Copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 */

#ifndef _MULTICORE_AUDIO_SIMPLE_H
#define _MULTICORE_AUDIO_SIMPLE_H

#include <stdbool.h>
#include <stdint.h>

#include "audio_system_config.h"
#include "drivers/bm_event_logging_driver/bm_event_logging.h"

/*
 * This structure lives in L2 memory where the MCAPI memory normally live
 * It's important to ensure that MCAPI is not enabled if you are using this
 * simpler approach for multi-core communications.  See the description in
 * the .c file for more information.
 *
 * This file is included in each of the three projects.  If a new variable /
 * field is added to this structure, it will be available on all three cores.
 *
 * Aligning on 32-bit boundaries (32-bit data types) helps ensure values are
 * updated properly for the other cores during read-modify-write operations.
 *
 */
typedef struct
{

    // Status of system initialization
    uint32_t arm_audio_peripheral_initialization_complete;
    uint32_t sharc_core1_ready_for_audio;
    uint32_t sharc_core1_processing_audio;
    uint32_t sharc_core2_ready_for_audio;
    uint32_t sharc_core2_processing_audio;

    // Synchronize LEDs
    uint32_t sharc_core1_led_strobed;

    // System status variables
    uint32_t audio_sample_rate;
    uint32_t audio_block_size;
    float core_clock_frequency;

    // Examine these variables to understand MHz loading for each core
    float sharc_core1_cpu_load_mhz;
    float sharc_core1_cpu_load_mhz_peak;
    float sharc_core2_cpu_load_mhz;
    float sharc_core2_cpu_load_mhz_peak;

    uint32_t sharc_core1_dropped_audio_frames;
    uint32_t sharc_core2_dropped_audio_frames;

    // ARM captures PB events and lets rest of system know
    uint32_t sharc_sam_pb_1_pressed;
    uint32_t sharc_sam_pb_2_pressed;

    /*
     * If the Audio Project Fin is installed on the SHARC Audio Module board, expose
     * additional functionality.
     **/
    #ifdef SAM_AUDIOPROJ_FIN_BOARD_PRESENT

        uint32_t audioproj_fin_sw_1_core1_pressed;
        uint32_t audioproj_fin_sw_2_core1_pressed;
        uint32_t audioproj_fin_sw_3_core1_pressed;
        uint32_t audioproj_fin_sw_4_core1_pressed;
        uint32_t audioproj_fin_sw_1_core2_pressed;
        uint32_t audioproj_fin_sw_2_core2_pressed;
        uint32_t audioproj_fin_sw_3_core2_pressed;
        uint32_t audioproj_fin_sw_4_core2_pressed;

        uint32_t audioproj_fin_sw_1_state;
        uint32_t audioproj_fin_sw_2_state;
        uint32_t audioproj_fin_sw_3_state;
        uint32_t audioproj_fin_sw_4_state;

        // These are the POTS on the Audio Project Fin
        float audioproj_fin_pot_hadc0;
        float audioproj_fin_pot_hadc1;
        float audioproj_fin_pot_hadc2;

        // And these are the additional HADC input channels available on the Audio Project Fin headers
        float audioproj_fin_aux_hadc3;
        float audioproj_fin_aux_hadc4;
        float audioproj_fin_aux_hadc5;
        float audioproj_fin_aux_hadc6;

        float audio_in_amplitude;

        uint32_t audioproj_fin_rev_3_20_or_later;
        
    #endif
    uint32_t audio_project_fin_present;

    // Effects processing presets
    uint32_t	effects_preset;
    uint32_t	reverb_preset;
    uint32_t	total_effects_presets;


    /**
     * We are using memory DMA (MDMA) to move audio data between cores in the background,
     * SHARC Core 1 needs the memory addresses of where the input and output buffers reside.
     */
    float *sharc_core1_audio_in;
    float *sharc_core1_audio_out;

    float *sharc_core2_audio_in;
    float *sharc_core2_audio_out;

    // Buffers for passing message data to ARM core
    uint32_t sharc_core1_new_message_ready;
    uint32_t sharc_core2_new_message_ready;

    uint32_t sharc_core1_event_emuclk;
    uint32_t sharc_core1_event_emuclk2;
    uint32_t sharc_core1_event_level;

    uint32_t sharc_core2_event_emuclk;
    uint32_t sharc_core2_event_emuclk2;
    uint32_t sharc_core2_event_level;

    char sharc_core1_event_message[EVENT_LOG_MESSAGE_LEN];
    char sharc_core2_event_message[EVENT_LOG_MESSAGE_LEN];

    // Add any parameters that you'd like all three cores to access here

    /*
     * If we're using Faust on both cores, use a simple FIFO to move MIDI notes
     * from SHARC Core 1 to SHARC Core 2
     */
    #if (USE_FAUST_ALGORITHM_CORE1) && (USE_FAUST_ALGORITHM_CORE2)

        uint8_t sh1_sh2_byte_fifo[SH1_SH2_BYTE_FIFO_SIZE];
        uint32_t sh1_sh2_fifo_read_ptr;
        uint32_t sh1_sh2_fifo_write_ptr;

    #endif
} MULTICORE_DATA;

extern volatile MULTICORE_DATA *multicore_data;
bool check_shared_memory_structure_sizes(void);

#endif  // _MULTICORE_AUDIO_SIMPLE_H
