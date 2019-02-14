/*
 * Copyright (c) 2018 Analog Devices, Inc.  All rights reserved.
 *
 * This is a single-core audio processing of 2 channels of audio from the ADAU1761
 *    The data flow within this framework is
 *    ADC -> SHARC 1 -> DAC
 *
 * Enable this framework in the project settings.  See Audio_Framework .h file for more
 * information.
 *
 */

// Define your audio system parameters in this file
#include "common/audio_system_config.h"

#if (defined(AUDIO_FRAMEWORK_8CH_SAM_AND_AUDIOPROJ_FIN) && AUDIO_FRAMEWORK_8CH_SAM_AND_AUDIOPROJ_FIN) || (defined(AUDIO_FRAMEWORK_A2B_BYPASS_SC589) && AUDIO_FRAMEWORK_A2B_BYPASS_SC589)

#include "audio_framework_8ch_sam_and_audioproj_fin_core2.h"

/*
 * If we are using both cores for audio processing, set up a set of input and output
 * buffers that we can use to DMA our floating point data back and forth to core 1.
 */
#if    (USE_BOTH_CORES_TO_PROCESS_AUDIO)

#include <services/int/adi_int.h>

// Simple multi-core data sharing scheme
#include "common/multicore_shared_memory.h"

// Simple gpio functionality
#include "drivers/bm_gpio_driver/bm_gpio.h"

// Audio plumbing functionality
#include "drivers/bm_audio_flow_driver/bm_audio_flow.h"

// Hooks into user processing functions
#include "../callback_audio_processing.h"

#if defined(USE_FAUST_ALGORITHM_CORE2) && USE_FAUST_ALGORITHM_CORE2
#include "audio_framework_faust_extension_core2.h"
#endif

// Definitions for this specific framework
#define    AUDIO_CHANNELS            (8)
#define    AUDIO_CHANNELS_MASK         (0xFF)
#define    SPDIF_DMA_CHANNELS         (2)
#define    SPDIF_DMA_CHANNEL_MASK     (0x3)

float AudioChannels_From_SHARC_Core1[AUDIO_CHANNELS * AUDIO_BLOCK_SIZE] = {0}; // Audio to SHARC 2
float AudioChannels_To_SHARC_Core1[AUDIO_CHANNELS * AUDIO_BLOCK_SIZE] = {0}; // Audio to SHARC 2

// 8 channels of audio from SHARC Core 1
float *audiochannel_0_left_in  = AudioChannels_From_SHARC_Core1 + AUDIO_BLOCK_SIZE * 0;
float *audiochannel_0_right_in = AudioChannels_From_SHARC_Core1 + AUDIO_BLOCK_SIZE * 1;
float *audiochannel_1_left_in  = AudioChannels_From_SHARC_Core1 + AUDIO_BLOCK_SIZE * 2;
float *audiochannel_1_right_in = AudioChannels_From_SHARC_Core1 + AUDIO_BLOCK_SIZE * 3;
float *audiochannel_2_left_in  = AudioChannels_From_SHARC_Core1 + AUDIO_BLOCK_SIZE * 4;
float *audiochannel_2_right_in = AudioChannels_From_SHARC_Core1 + AUDIO_BLOCK_SIZE * 5;
float *audiochannel_3_left_in  = AudioChannels_From_SHARC_Core1 + AUDIO_BLOCK_SIZE * 6;
float *audiochannel_3_right_in = AudioChannels_From_SHARC_Core1 + AUDIO_BLOCK_SIZE * 7;

// 8 channels of processed audio to be sent back to DACs via SHARC Core 1
float *audiochannel_0_left_out  = AudioChannels_To_SHARC_Core1 + AUDIO_BLOCK_SIZE * 0;
float *audiochannel_0_right_out = AudioChannels_To_SHARC_Core1 + AUDIO_BLOCK_SIZE * 1;
float *audiochannel_1_left_out  = AudioChannels_To_SHARC_Core1 + AUDIO_BLOCK_SIZE * 2;
float *audiochannel_1_right_out = AudioChannels_To_SHARC_Core1 + AUDIO_BLOCK_SIZE * 3;
float *audiochannel_2_left_out  = AudioChannels_To_SHARC_Core1 + AUDIO_BLOCK_SIZE * 4;
float *audiochannel_2_right_out = AudioChannels_To_SHARC_Core1 + AUDIO_BLOCK_SIZE * 5;
float *audiochannel_3_left_out  = AudioChannels_To_SHARC_Core1 + AUDIO_BLOCK_SIZE * 6;
float *audiochannel_3_right_out = AudioChannels_To_SHARC_Core1 + AUDIO_BLOCK_SIZE * 7;

/* This variable is used to detect if we enter the DMA interrupt service routine while the
 * previous block is still being processed.  This indicates that we've overrun the available MIPS
 */
volatile bool last_audio_frame_completed = true;

// Keep track of the number of the number of audio blocks arriving and the number we've processed
uint32_t audio_blocks_processed_count = 0;
uint32_t audio_blocks_new_events_count = 0;

// Cycle counter used for benchmarking our code
uint64_t cycle_cntr;

//#pragma optimize_for_speed
void audioframework_dma_handler(void) {

    int i;

    // Clear DMA transfer status
    *pREG_DMA19_STAT |= BITM_DMA_STAT_IRQDONE;

    // Capture a processor cycle count for benchmarking purposes.
    cycle_cntr = audioflow_get_cpu_cycle_counter();

    // Toggle LED12 on the SHARC Audio Module board to show that the audio is running and we're getting interrupts
    static uint16_t tglCntr = 0;
    if (tglCntr++ > (AUDIO_SAMPLE_RATE / AUDIO_BLOCK_SIZE) / 2) {
        tglCntr = 0;
        gpio_toggle(GPIO_SHARC_SAM_LED12);
    }

    // Increment the counter for new audio blocks
    audio_blocks_new_events_count++;

    // Set flag that we are now getting audio interrupts and processing audio
    multicore_data->sharc_core2_processing_audio = true;

    // Detect and handle the "frame dropped" event
    if (!last_audio_frame_completed) {

        // Make a call to the callback
        processaudio_mips_overflow();

        // Zero output buffers so we get silence instead of repeated audio
        for (i = 0; i < AUDIO_CHANNELS * AUDIO_BLOCK_SIZE; i++) {
            AudioChannels_To_SHARC_Core1[i] = 0;
        }

        // Update dropped audio frame counter
        multicore_data->sharc_core2_dropped_audio_frames++;

        // Don't trigger the software interrupt for audio processing on this block
        return;
    }
    else {

        // Set to false as we begin processing this new audio frame
        last_audio_frame_completed = false;

        // Raise lower priority interrupt to kick off AudioFramework_AudioCallback_Handler
        *pREG_SEC0_RAISE = INTR_SOFT6;
    }
}

/**
 * @brief      SHARC Core 2 Audio callback handler
 *
 * As mentioned in the comments for AudioFramework_DMA_Handler, a lower
 * priority interrupt is 'raised' at the end of AudioFramework_DMA_Handler
 * which is where we kick off the user's audio processing.
 *
 * This function also calculates the CPU utilization using the DMA pointers
 * for the DMA happening in the background.  The advantage of this approach
 * over raw cycle counts is that it is independent of core clock frequency
 * and sample rate and thus a simpler calculation.
 *
 * The current and peak CPU utilization is stored in the shared memory
 * structure.
 *
 * Finally, this routine strobes LED11 once per second.  If LED11 is strobing
 * faster or slower than once per second, it indicates that there is a mismatch
 * between the reported and actual sample rates!  The ARM core strobes LED10
 * roughly every second so it provides a quick visual test to ensure audio is
 * working the way it should.
 */
void audioframework_audiocallback_handler(uint32_t iid) {

    // Clear the pending software interrupt
    *pREG_SEC0_END = INTR_SOFT6;

    // If we're using Faust, run the Faust audio processing before our callback
    #if defined(USE_FAUST_ALGORITHM_CORE2) && USE_FAUST_ALGORITHM_CORE2
    Faust_audio_processing();
    #endif

    // Call our audio callback function
    processaudio_callback();

    // Calculate our CPU load for this SHARC core based on our cycle counter
    multicore_data->sharc_core2_cpu_load_mhz = audioflow_get_cpu_load(cycle_cntr,
                                                                      AUDIO_BLOCK_SIZE,
                                                                      CORE_CLOCK_FREQ_HZ,
                                                                      AUDIO_SAMPLE_RATE);

    if (multicore_data->sharc_core2_cpu_load_mhz > multicore_data->sharc_core2_cpu_load_mhz_peak) {
        multicore_data->sharc_core2_cpu_load_mhz_peak = multicore_data->sharc_core2_cpu_load_mhz;
    }

    // Increment our counter containing number of blocks processed
    audio_blocks_processed_count++;

    // Set flag that last audio frame has completed
    last_audio_frame_completed = true;
}

/*
 * Initialize the audio framework.  For core 2, this entails setting up an interrupt to
 * let us know when a new block of audio is ready.  Core 1 will raise this interrupt after
 * data is copied into the input buffer.
 */
void audioframework_initialize() {

    // Clear peak MIPS
    multicore_data->sharc_core2_cpu_load_mhz_peak = 0;

    // Clear dropped audio frame counter
    multicore_data->sharc_core2_dropped_audio_frames = 0;

    // Set pointers in shared memory structure so SHARC Core 1 knows where to MDMA data to / from
    multicore_data->sharc_core2_audio_in  = AudioChannels_From_SHARC_Core1;
    multicore_data->sharc_core2_audio_out = AudioChannels_To_SHARC_Core1;

    // If we're using Faust, initialize the Faust engine
    #if defined(USE_FAUST_ALGORITHM_CORE2) && USE_FAUST_ALGORITHM_CORE2
    faust_initialize();
    #endif

    /*
     * Setup our software interrupt that we'll trigger from Core 1 when a new
     * audio block is ready
     */
    adi_int_InstallHandler(INTR_MDMA1_DST,
                           (ADI_INT_HANDLER_PTR)audioframework_dma_handler,
                           NULL,
                           true);

    adi_int_InstallHandler(INTR_SOFT6,
                           (ADI_INT_HANDLER_PTR)audioframework_audiocallback_handler,
                           NULL,
                           true);
}

/*
 * Kick off audio processing.  For core 2, this means setting a flag in shared memory
 * that we are ready for audio.  Once core 1 finishes initializing the DMA, etc. it will
 * poll this flag and once it is set to true, it will begin passing data into our buffers
 * and raising the interrupt indicating that new data is ready.
 */
void audioframework_start() {

    multicore_data->sharc_core2_ready_for_audio = true;
}

#endif    // __DUAL_CORE_AUDIO_PROCESSING__
#endif    // __AUDIO_FRAMEWORK_SINGLEDUAL_CORE_8CH_A2B__
int audio_framework_8ch_sam_and_audioproj_fin = 1;
