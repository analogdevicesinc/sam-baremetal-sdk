/*
 * Copyright (c) 2018 Analog Devices, Inc.  All rights reserved.
 *
 * This is a single-core audio processing framework that operates on audio data from
 * both the local ADAU1761 CODEC as well as the A2B bus.  It can be configured to be
 * a master or a slave node (in common/audio_system_config.h).
 *
 *
 * See additional comments in the AD2425 driver (within drivers) to understand
 * how to use this as part of a SigmaStudio A2B layout.
 *
 * In all audio frameworks, SHARC core 1 has the following responsibilities:
 *
 *    - Setting up the DMA to move audio data between memory and the SPORTs
 *    - Managing the flow of data between core 1 and core 2 (shared memory or DMA)
 *    - Setting up the interrupts needed to let core 2 know audio data is ready
 *    - Calling the user's audio callback function
 */

// Define your audio system parameters in this file
#include "common/audio_system_config.h"
#include "../callback_audio_processing.h"

#if (defined(AUDIO_FRAMEWORK_8CH_SAM_AND_AUDIOPROJ_FIN) && AUDIO_FRAMEWORK_8CH_SAM_AND_AUDIOPROJ_FIN) || (defined(AUDIO_FRAMEWORK_A2B_BYPASS_SC589) && AUDIO_FRAMEWORK_A2B_BYPASS_SC589)

#include "audio_framework_8ch_sam_and_audioproj_fin_core1.h"

#include <services/dma/adi_dma.h>
#include <services/int/adi_int.h>
#include <services/int/adi_sec.h>
#include <sys/adi_core.h>
#include <sys/platform.h>

// Simple gpio functionality
#include "drivers/bm_gpio_driver/bm_gpio.h"

// Audio plumbing functionality
#include "drivers/bm_audio_flow_driver/bm_audio_flow.h"

// Simple event logging / error handling functionality
#include "drivers/bm_event_logging_driver/bm_event_logging.h"

// Structure containing shared variables between the three cores
#include "common/multicore_shared_memory.h"

// Hooks into user processing functions
#include "../callback_audio_processing.h"

#if (USE_FAUST_ALGORITHM_CORE1)
#include "audio_framework_faust_extension_core1.h"
#endif

// Local function prototypes for our interrupt handlers
void audioframework_dma_handler(uint32_t iid, void *arg);
void audioframework_audiocallback_handler(uint32_t iid);

// Definitions for this specific framework
#define     AUDIO_CHANNELS             (8)
#define     AUDIO_CHANNELS_MASK        (0xFF)
#define     SPDIF_DMA_CHANNELS         (2)
#define     SPDIF_DMA_CHANNEL_MASK     (0x3)

// ADAU1761 Fixed-point (raw ADC/DAC data) DMA buffers for ping-pong / double-buffered DMA
int section("seg_dmda_nw") sport0_dma_rx_0_buffer[AUDIO_CHANNELS * AUDIO_BLOCK_SIZE] = {
    0
};
int section("seg_dmda_nw") sport0_dma_rx_1_buffer[AUDIO_CHANNELS * AUDIO_BLOCK_SIZE] = {
    0
};
int section("seg_dmda_nw") sport0_dma_tx_0_buffer[AUDIO_CHANNELS * AUDIO_BLOCK_SIZE] = {
    0
};
int section("seg_dmda_nw") sport0_dma_tx_1_buffer[AUDIO_CHANNELS * AUDIO_BLOCK_SIZE] = {
    0
};

// A2B Fixed-point (raw ADC/DAC data) DMA buffers for ping-pong / double-buffered DMA
int section("seg_dmda_nw") sport1_dma_rx_0_buffer[AUDIO_CHANNELS * AUDIO_BLOCK_SIZE] = {
    0
};
int section("seg_dmda_nw") sport1_dma_rx_1_buffer[AUDIO_CHANNELS * AUDIO_BLOCK_SIZE] = {
    0
};
int section("seg_dmda_nw") sport1_dma_tx_0_buffer[AUDIO_CHANNELS * AUDIO_BLOCK_SIZE] = {
    0
};
int section("seg_dmda_nw") sport1_dma_tx_1_buffer[AUDIO_CHANNELS * AUDIO_BLOCK_SIZE] = {
    0
};

// SPDIF Fixed-point (raw ADC/DAC data) DMA buffers for ping-pong / double-buffered DMA
int section("seg_dmda_nw") sport2_dma_rx_0_buffer[SPDIF_DMA_CHANNELS * AUDIO_BLOCK_SIZE] = {
    0
};
int section("seg_dmda_nw") sport2_dma_rx_1_buffer[SPDIF_DMA_CHANNELS * AUDIO_BLOCK_SIZE] = {
    0
};
int section("seg_dmda_nw") sport2_dma_tx_0_buffer[SPDIF_DMA_CHANNELS * AUDIO_BLOCK_SIZE] = {
    0
};
int section("seg_dmda_nw") sport2_dma_tx_1_buffer[SPDIF_DMA_CHANNELS * AUDIO_BLOCK_SIZE] = {
    0
};

// Floating-point buffers that we will process / operate on
// These are aligned to 32-byte boundaries so we can use fast DMAs to move them around
#pragma align 32
float adau1761_audiochannels_out[AUDIO_CHANNELS * AUDIO_BLOCK_SIZE] = {0};    // Audio to DACs
#pragma align 32
float adau1761_audiochannels_in[AUDIO_CHANNELS * AUDIO_BLOCK_SIZE] = {0};     // Audio from ADCs

#pragma align 32
float a2b_audiochannels_out[AUDIO_CHANNELS * AUDIO_BLOCK_SIZE] = {0};        // Audio to A2B bus
#pragma align 32
float a2b_audiochannels_in[AUDIO_CHANNELS * AUDIO_BLOCK_SIZE] = {0};         // Audio from A2B bus

#pragma align 32
float spdif_audiochannels_out[SPDIF_DMA_CHANNELS * AUDIO_BLOCK_SIZE] = {0};    // Audio to SPDIF TX
#pragma align 32
float spdif_audiochannels_in[SPDIF_DMA_CHANNELS * AUDIO_BLOCK_SIZE] = {0};      // Audio from SPDIF RX

#if (USE_BOTH_CORES_TO_PROCESS_AUDIO)
#pragma align 32
float audiochannels_from_sharc_core2[AUDIO_CHANNELS * AUDIO_BLOCK_SIZE] = {0};      // Audio from SHARC Core 2
#pragma align 32
float audiochannels_to_sharc_core2[AUDIO_CHANNELS * AUDIO_BLOCK_SIZE] = {0};          // Audio from SHARC Core 2
#endif

/*
 * This framework sets up an 8 channel TDM link to the ADAU1761.  While the
 * ADAU1761 has a stereo ADC and a stereo DAC, the remaining 6 channels can
 * be used for pre-processed / post-processing audio channels.
 */

// These first two channels contain the audio from the ADCs
float *audiochannel_adau1761_0_left_in  = adau1761_audiochannels_in + AUDIO_BLOCK_SIZE * 0;
float *audiochannel_adau1761_0_right_in = adau1761_audiochannels_in + AUDIO_BLOCK_SIZE * 1;

// These remaining channels can be used for other pre-processed audio on the '1761
float *audiochannel_adau1761_1_left_in  = adau1761_audiochannels_in + AUDIO_BLOCK_SIZE * 2;
float *audiochannel_adau1761_1_right_in = adau1761_audiochannels_in + AUDIO_BLOCK_SIZE * 3;
float *audiochannel_adau1761_2_left_in  = adau1761_audiochannels_in + AUDIO_BLOCK_SIZE * 4;
float *audiochannel_adau1761_2_right_in = adau1761_audiochannels_in + AUDIO_BLOCK_SIZE * 5;
float *audiochannel_adau1761_3_left_in  = adau1761_audiochannels_in + AUDIO_BLOCK_SIZE * 6;
float *audiochannel_adau1761_3_right_in = adau1761_audiochannels_in + AUDIO_BLOCK_SIZE * 7;

// These first two channels contain the audio for the DACs
float *audiochannel_adau1761_0_left_out  = adau1761_audiochannels_out + AUDIO_BLOCK_SIZE * 0;
float *audiochannel_adau1761_0_right_out = adau1761_audiochannels_out + AUDIO_BLOCK_SIZE * 1;

// These remaining channels can be used to send audio for post processing on ADAU1761
float *audiochannel_adau1761_1_left_out  = adau1761_audiochannels_out + AUDIO_BLOCK_SIZE * 2;
float *audiochannel_adau1761_1_right_out = adau1761_audiochannels_out + AUDIO_BLOCK_SIZE * 3;
float *audiochannel_adau1761_2_left_out  = adau1761_audiochannels_out + AUDIO_BLOCK_SIZE * 4;
float *audiochannel_adau1761_2_right_out = adau1761_audiochannels_out + AUDIO_BLOCK_SIZE * 5;
float *audiochannel_adau1761_3_left_out  = adau1761_audiochannels_out + AUDIO_BLOCK_SIZE * 6;
float *audiochannel_adau1761_3_right_out = adau1761_audiochannels_out + AUDIO_BLOCK_SIZE * 7;

// SPDIF digital audio in buffers
float *audiochannel_spdif_0_left_in  = spdif_audiochannels_in + AUDIO_BLOCK_SIZE * 0;
float *audiochannel_spdif_0_right_in = spdif_audiochannels_in + AUDIO_BLOCK_SIZE * 1;

// SPDIF digital audio out buffers
float *audiochannel_spdif_0_left_out  = spdif_audiochannels_out + AUDIO_BLOCK_SIZE * 0;
float *audiochannel_spdif_0_right_out = spdif_audiochannels_out + AUDIO_BLOCK_SIZE * 1;

// A2B Audio In (from the A2B bus)
float *audiochannel_a2b_0_left_in  = a2b_audiochannels_in + AUDIO_BLOCK_SIZE * 0;
float *audiochannel_a2b_0_right_in = a2b_audiochannels_in + AUDIO_BLOCK_SIZE * 1;
float *audiochannel_a2b_1_left_in  = a2b_audiochannels_in + AUDIO_BLOCK_SIZE * 2;
float *audiochannel_a2b_1_right_in = a2b_audiochannels_in + AUDIO_BLOCK_SIZE * 3;
float *audiochannel_a2b_2_left_in  = a2b_audiochannels_in + AUDIO_BLOCK_SIZE * 4;
float *audiochannel_a2b_2_right_in = a2b_audiochannels_in + AUDIO_BLOCK_SIZE * 5;
float *audiochannel_a2b_3_left_in  = a2b_audiochannels_in + AUDIO_BLOCK_SIZE * 6;
float *audiochannel_a2b_3_right_in = a2b_audiochannels_in + AUDIO_BLOCK_SIZE * 7;

// A2B Audio Out (to the A2B bus)
float *audiochannel_a2b_0_left_out  = a2b_audiochannels_out + AUDIO_BLOCK_SIZE * 0;
float *audiochannel_a2b_0_right_out = a2b_audiochannels_out + AUDIO_BLOCK_SIZE * 1;
float *audiochannel_a2b_1_left_out  = a2b_audiochannels_out + AUDIO_BLOCK_SIZE * 2;
float *audiochannel_a2b_1_right_out = a2b_audiochannels_out + AUDIO_BLOCK_SIZE * 3;
float *audiochannel_a2b_2_left_out  = a2b_audiochannels_out + AUDIO_BLOCK_SIZE * 4;
float *audiochannel_a2b_2_right_out = a2b_audiochannels_out + AUDIO_BLOCK_SIZE * 5;
float *audiochannel_a2b_3_left_out  = a2b_audiochannels_out + AUDIO_BLOCK_SIZE * 6;
float *audiochannel_a2b_3_right_out = a2b_audiochannels_out + AUDIO_BLOCK_SIZE * 7;

#if (USE_BOTH_CORES_TO_PROCESS_AUDIO)

// Processed audio data from SHARC Core
float *audiochannel_from_sharc_core2_0_left  = audiochannels_from_sharc_core2 + AUDIO_BLOCK_SIZE * 0;
float *audiochannel_from_sharc_core2_0_right = audiochannels_from_sharc_core2 + AUDIO_BLOCK_SIZE * 1;
float *audiochannel_from_sharc_core2_1_left  = audiochannels_from_sharc_core2 + AUDIO_BLOCK_SIZE * 2;
float *audiochannel_from_sharc_core2_1_right = audiochannels_from_sharc_core2 + AUDIO_BLOCK_SIZE * 3;
float *audiochannel_from_sharc_core2_2_left  = audiochannels_from_sharc_core2 + AUDIO_BLOCK_SIZE * 4;
float *audiochannel_from_sharc_core2_2_right = audiochannels_from_sharc_core2 + AUDIO_BLOCK_SIZE * 5;
float *audiochannel_from_sharc_core2_3_left  = audiochannels_from_sharc_core2 + AUDIO_BLOCK_SIZE * 6;
float *audiochannel_from_sharc_core2_3_right = audiochannels_from_sharc_core2 + AUDIO_BLOCK_SIZE * 7;

float *audiochannel_to_sharc_core2_0_left  = audiochannels_to_sharc_core2 + AUDIO_BLOCK_SIZE * 0;
float *audiochannel_to_sharc_core2_0_right = audiochannels_to_sharc_core2 + AUDIO_BLOCK_SIZE * 1;
float *audiochannel_to_sharc_core2_1_left  = audiochannels_to_sharc_core2 + AUDIO_BLOCK_SIZE * 2;
float *audiochannel_to_sharc_core2_1_right = audiochannels_to_sharc_core2 + AUDIO_BLOCK_SIZE * 3;
float *audiochannel_to_sharc_core2_2_left  = audiochannels_to_sharc_core2 + AUDIO_BLOCK_SIZE * 4;
float *audiochannel_to_sharc_core2_2_right = audiochannels_to_sharc_core2 + AUDIO_BLOCK_SIZE * 5;
float *audiochannel_to_sharc_core2_3_left  = audiochannels_to_sharc_core2 + AUDIO_BLOCK_SIZE * 6;
float *audiochannel_to_sharc_core2_3_right = audiochannels_to_sharc_core2 + AUDIO_BLOCK_SIZE * 7;

#endif

// Define alias pointers (that are common across frameworks)
float *audiochannel_0_left_in  = adau1761_audiochannels_in + AUDIO_BLOCK_SIZE * 0;
float *audiochannel_0_right_in = adau1761_audiochannels_in + AUDIO_BLOCK_SIZE * 1;
float *audiochannel_1_left_in  = adau1761_audiochannels_in + AUDIO_BLOCK_SIZE * 2;
float *audiochannel_1_right_in = adau1761_audiochannels_in + AUDIO_BLOCK_SIZE * 3;
float *audiochannel_2_left_in  = adau1761_audiochannels_in + AUDIO_BLOCK_SIZE * 4;
float *audiochannel_2_right_in = adau1761_audiochannels_in + AUDIO_BLOCK_SIZE * 5;
float *audiochannel_3_left_in  = adau1761_audiochannels_in + AUDIO_BLOCK_SIZE * 6;
float *audiochannel_3_right_in = adau1761_audiochannels_in + AUDIO_BLOCK_SIZE * 7;

#if (USE_BOTH_CORES_TO_PROCESS_AUDIO)
// If we're in dual core, point our alias to the buffers heading to SHARC 2.
float *audiochannel_0_left_out  = audiochannels_to_sharc_core2 + AUDIO_BLOCK_SIZE * 0;
float *audiochannel_0_right_out = audiochannels_to_sharc_core2 + AUDIO_BLOCK_SIZE * 1;
float *audiochannel_1_left_out  = audiochannels_to_sharc_core2 + AUDIO_BLOCK_SIZE * 2;
float *audiochannel_1_right_out = audiochannels_to_sharc_core2 + AUDIO_BLOCK_SIZE * 3;
float *audiochannel_2_left_out  = audiochannels_to_sharc_core2 + AUDIO_BLOCK_SIZE * 4;
float *audiochannel_2_right_out = audiochannels_to_sharc_core2 + AUDIO_BLOCK_SIZE * 5;
float *audiochannel_3_left_out  = audiochannels_to_sharc_core2 + AUDIO_BLOCK_SIZE * 6;
float *audiochannel_3_right_out = audiochannels_to_sharc_core2 + AUDIO_BLOCK_SIZE * 7;

#else
// Otherwise, point our alias buffers back out to the ADAU1761
float *audiochannel_0_left_out  = adau1761_audiochannels_out + AUDIO_BLOCK_SIZE * 0;
float *audiochannel_0_right_out = adau1761_audiochannels_out + AUDIO_BLOCK_SIZE * 1;
float *audiochannel_1_left_out  = adau1761_audiochannels_out + AUDIO_BLOCK_SIZE * 2;
float *audiochannel_1_right_out = adau1761_audiochannels_out + AUDIO_BLOCK_SIZE * 3;
float *audiochannel_2_left_out  = adau1761_audiochannels_out + AUDIO_BLOCK_SIZE * 4;
float *audiochannel_2_right_out = adau1761_audiochannels_out + AUDIO_BLOCK_SIZE * 5;
float *audiochannel_3_left_out  = adau1761_audiochannels_out + AUDIO_BLOCK_SIZE * 6;
float *audiochannel_3_right_out = adau1761_audiochannels_out + AUDIO_BLOCK_SIZE * 7;
#endif

/* This variable is used to detect if we enter the DMA interrupt service routine while the
 * previous block is still being processed.  This indicates that we've overrun the available MIPS
 */
volatile bool last_audio_frame_completed = true;

// Keep track of the number of the number of audio blocks arriving and the number we've processed
uint32_t audio_blocks_processed_count = 0;
uint32_t audio_blocks_new_events_count = 0;

// Cycle counter used for benchmarking our code
uint64_t cycle_cntr;

// DMA & SPORT Configuration for SPORT 0 (ADAU1761 connection)
SPORT_DMA_CONFIG SPR0_ADAU1761_8CH_Config = {

    .sport_number           = SPORT0,

    .dma_audio_channels   = AUDIO_CHANNELS,
    .dma_audio_block_size = AUDIO_BLOCK_SIZE,

    .dma_tx_buffer_0     = sport0_dma_tx_0_buffer,
    .dma_tx_buffer_1     = sport0_dma_tx_1_buffer,
    .dma_rx_buffer_0     = sport0_dma_rx_0_buffer,
    .dma_rx_buffer_1     = sport0_dma_rx_1_buffer,

    // SPORT Transmit
    .pREG_SPORT_CTL_A    = (0x1 << BITP_SPORT_CTL_A_DTYPE) |        // Right justify, sign extend
                           (0x1F << BITP_SPORT_CTL_A_SLEN) |    // 32-bit transfers
                           BITM_SPORT_CTL_A_CKRE  |               // Sample on rising edge
                           BITM_SPORT_CTL_A_FSR |               // Frame sync required
                           BITM_SPORT_CTL_A_DIFS |                // Data independent FS
                           BITM_SPORT_CTL_A_LFS |                // Active low FS / LR CLK
                           BITM_SPORT_CTL_A_SPTRAN |               // SPORT is transmitter
                           0,

    .pREG_SPORT_MCTL_A  = BITM_SPORT_MCTL_A_MCE |                // Multi-channel enable
                          (0x1 << BITP_SPORT_MCTL_A_MFD) |        // Frame delay = 1
                          ((8 - 1) << BITP_SPORT_MCTL_A_WSIZE) |  // 8 words / frame
                          0,

    .pREG_SPORT_CS0_A   = AUDIO_CHANNELS_MASK,   // 8 channels

    // SPORT Receive
    .pREG_SPORT_CTL_B    = (0x1 << BITP_SPORT_CTL_B_DTYPE) |        // Right justify, sign extend
                           (0x1F << BITP_SPORT_CTL_B_SLEN) |    // 32-bit transfers
                           BITM_SPORT_CTL_A_CKRE  |               // Sample on rising edge
                           BITM_SPORT_CTL_B_FSR |               // Frame sync required
                           BITM_SPORT_CTL_B_DIFS |                // Data independent FS
                           BITM_SPORT_CTL_B_LFS |                // Active low FS / LR CLK
                           0,

    .pREG_SPORT_MCTL_B  = BITM_SPORT_MCTL_B_MCE |                // Multi-channel enable
                          (0x1 << BITP_SPORT_MCTL_B_MFD) |        // Frame delay = 1
                          ((8 - 1) << BITP_SPORT_MCTL_B_WSIZE) |  // 8 words / frame
                          0,

    .pREG_SPORT_CS0_B   = AUDIO_CHANNELS_MASK,   // 8 channels

    .generates_interrupts = true,
    .dma_interrupt_routine = audioframework_dma_handler
};

// DMA & SPORT Configuration for SPORT 1 (A2B connection)
SPORT_DMA_CONFIG SPR1_A2B_8CH_Config = {

    .sport_number           = SPORT1,

    .dma_audio_channels   = AUDIO_CHANNELS,
    .dma_audio_block_size = AUDIO_BLOCK_SIZE,

    .dma_tx_buffer_0     = sport1_dma_tx_0_buffer,
    .dma_tx_buffer_1     = sport1_dma_tx_1_buffer,
    .dma_rx_buffer_0     = sport1_dma_rx_0_buffer,
    .dma_rx_buffer_1     = sport1_dma_rx_1_buffer,

    // SPORT Transmit
    .pREG_SPORT_CTL_A    = (0x1 << BITP_SPORT_CTL_A_DTYPE) |        // Right justify, sign extend
                           (0x1F << BITP_SPORT_CTL_A_SLEN) |    // 32-bit transfers
                           BITM_SPORT_CTL_A_CKRE  |               // Sample on rising edge
                           BITM_SPORT_CTL_A_FSR |               // Frame sync required
                           BITM_SPORT_CTL_A_DIFS |                // Data independent FS
                           BITM_SPORT_CTL_A_LFS |                // Active low FS / LR CLK
                           BITM_SPORT_CTL_A_SPTRAN |               // SPORT is transmitter
                           0,

    .pREG_SPORT_MCTL_A  = BITM_SPORT_MCTL_A_MCE |                // Multi-channel enable
                          (0x1 << BITP_SPORT_MCTL_A_MFD) |        // Frame delay = 1
                          ((8 - 1) << BITP_SPORT_MCTL_A_WSIZE) |  // 8 words / frame
                          0,

    .pREG_SPORT_CS0_A   = AUDIO_CHANNELS_MASK,   // 8 channels

    // SPORT Receive
    .pREG_SPORT_CTL_B    = (0x1 << BITP_SPORT_CTL_B_DTYPE) |        // Right justify, sign extend
                           (0x1F << BITP_SPORT_CTL_B_SLEN) |    // 32-bit transfers
                           BITM_SPORT_CTL_A_CKRE  |               // Sample on rising edge
                           BITM_SPORT_CTL_B_FSR |               // Frame sync required
                           BITM_SPORT_CTL_B_DIFS |                // Data independent FS
                           BITM_SPORT_CTL_B_LFS |                // Active low FS / LR CLK
                           0,

    .pREG_SPORT_MCTL_B  = BITM_SPORT_MCTL_B_MCE |                // Multi-channel enable
                          (0x1 << BITP_SPORT_MCTL_B_MFD) |        // Frame delay = 1
                          ((8 - 1) << BITP_SPORT_MCTL_B_WSIZE) |  // 8 words / frame
                          0,

    .pREG_SPORT_CS0_B   = AUDIO_CHANNELS_MASK,   // 8 channels

    .generates_interrupts = false
};

// DMA & SPORT Configuration for SPORT 2 (S/PDIF connection)
SPORT_DMA_CONFIG SPR2_spdif_2CH_Config = {

    .sport_number           = SPORT2,

    .dma_audio_channels   = SPDIF_DMA_CHANNELS,
    .dma_audio_block_size = AUDIO_BLOCK_SIZE,

    .dma_tx_buffer_0     = sport2_dma_tx_0_buffer,
    .dma_tx_buffer_1     = sport2_dma_tx_1_buffer,
    .dma_rx_buffer_0     = sport2_dma_rx_0_buffer,
    .dma_rx_buffer_1     = sport2_dma_rx_1_buffer,

    // SPORT Transmit
    .pREG_SPORT_CTL_A    = (0x1 << BITP_SPORT_CTL_A_DTYPE) |        // Right justify, sign extend
                           (0x1F << BITP_SPORT_CTL_A_SLEN) |    // 32-bit transfers
                           BITM_SPORT_CTL_A_CKRE  |               // Sample on rising edge
                           BITM_SPORT_CTL_A_FSR |               // Frame sync required
                           BITM_SPORT_CTL_A_DIFS |                // Data independent FS
                           BITM_SPORT_CTL_A_LFS |                // Active low FS / LR CLK
                           BITM_SPORT_CTL_A_SPTRAN |               // SPORT is transmitter
                           0,

    .pREG_SPORT_MCTL_A  = BITM_SPORT_MCTL_A_MCE |                // Multi-channel enable
                          (0x1 << BITP_SPORT_MCTL_A_MFD) |        // Frame delay = 1
                          ((2 - 1) << BITP_SPORT_MCTL_A_WSIZE) |  // 2 words / frame
                          0,

    .pREG_SPORT_CS0_A   = SPDIF_DMA_CHANNEL_MASK,   // 2 channels

    // SPORT Receive
    .pREG_SPORT_CTL_B    = (0x1 << BITP_SPORT_CTL_B_DTYPE) |        // Right justify, sign extend
                           (0x1F << BITP_SPORT_CTL_B_SLEN) |    // 32-bit transfers
                           BITM_SPORT_CTL_A_CKRE  |               // Sample on rising edge
                           BITM_SPORT_CTL_B_FSR |               // Frame sync required
                           BITM_SPORT_CTL_B_DIFS |                // Data independent FS
                           BITM_SPORT_CTL_B_LFS |                // Active low FS / LR CLK
                           0,

    .pREG_SPORT_MCTL_B  = BITM_SPORT_MCTL_B_MCE |                // Multi-channel enable
                          (0x1 << BITP_SPORT_MCTL_B_MFD) |        // Frame delay = 1
                          ((2 - 1) << BITP_SPORT_MCTL_B_WSIZE) |  // 2 words / frame
                          0,

    .pREG_SPORT_CS0_B   = SPDIF_DMA_CHANNEL_MASK,   // 2 channels

    .generates_interrupts = false
};

/**
 * @brief      SHARC Core 1 handler for DMA interrupts
 *
 * This function is called every time a SPORT DMA moves a block of audio
 * data to / from external converters.  The DMA engine is set up to automatically
 * ping-pong between two buffers so this function determines which of the two
 * sets of buffers contains the new data.  This function manages the conversion
 * of audio data (which is typically 24-bit fixed point) to floating point using the
 * built-in functions.
 *
 * When using a dual-core framework, this routine also sets up the memory DMA
 * to move blocks of data from core 1 to core 2, and then from core 2 back to core 1.
 * This operation is pipelined.
 *
 * Finally, this routine raises a software interrupt which in turn calls the user's
 * audio processing callback function.  The reason for doing this is to keep the audio
 * processing at a lower priority that these interrupts.  If the audio processing is
 * done within this interrupt service routine, we may miss new blocks of audio if processing
 * takes too long.  In this model, one can optionally check in this service routine
 * if audio processing from the last block is still happening.
 *
 */

void audioframework_dma_handler(uint32_t iid, void *arg){
    int i;

    // Clear DMA interrupt
    *pREG_DMA1_STAT |= BITM_DMA_STAT_IRQDONE;

    // Capture a processor cycle count for benchmarking purposes.
    cycle_cntr = audioflow_get_cpu_cycle_counter();

    // Get the configuration of the SPORT / DMA combo driving interrupts
    SPORT_DMA_CONFIG *sport_dma_cfg = (SPORT_DMA_CONFIG *)arg;

    // Toggle LED11 on the SHARC Audio Module board to show that the audio is running and we're getting interrupts
    static uint16_t tglCntr = 0;
    if (tglCntr++ > (AUDIO_SAMPLE_RATE / AUDIO_BLOCK_SIZE) / 2) {
        tglCntr = 0;
        gpio_toggle(GPIO_SHARC_SAM_LED11);
        multicore_data->sharc_core1_led_strobed = true;
    }

    // Increment the counter for new audio blocks
    audio_blocks_new_events_count++;

    // Set flag that we are now getting audio interrupts and processing audio
    multicore_data->sharc_core1_processing_audio = true;

    #if (USE_BOTH_CORES_TO_PROCESS_AUDIO)

    /*
     ********************************************************************************
     * STEP 1:
     * Kick off transfer of output audio from SHARC 1 from last block to input audio
     * for SHARC 2
     ********************************************************************************
     */

    // Be sure the MDMA we kicked off at the end of the last block has now completed
    while (!*pREG_DMA19_STAT & 0x1) {

        // We should never get here
    }

    // Translate addresses from local to global
    void *sharc_core2_dest_addr = (void *)((uint32_t)multicore_data->sharc_core2_audio_in  + 0x28800000);
    void *sharc_core1_src_addr =  (void *)((uint32_t)multicore_data->sharc_core1_audio_out + 0x28000000);

    // Source
    *pREG_DMA8_ADDRSTART = sharc_core1_src_addr;
    *pREG_DMA8_XCNT = AUDIO_BLOCK_SIZE * AUDIO_CHANNELS;
    *pREG_DMA8_XMOD = 4;

    // Dest
    *pREG_DMA9_ADDRSTART = sharc_core2_dest_addr;
    *pREG_DMA9_XCNT = AUDIO_BLOCK_SIZE * AUDIO_CHANNELS;
    *pREG_DMA9_XMOD = 4;

    // Kick off transfer
    *pREG_DMA8_CFG     = BITM_DMA_CFG_EN |
                         (0x2 << BITP_DMA_CFG_MSIZE) |
                         0;

    *pREG_DMA9_CFG = BITM_DMA_CFG_EN |                        // Enable DMA
                     BITM_DMA_CFG_WNR |                      // Write mode
                     (0x2 << BITP_DMA_CFG_MSIZE) |
                     0;

    // And then route the audio we received from core 2 to the right output buffers
    processaudio_output_routing();

    #endif    // USE_BOTH_CORES_TO_PROCESS_AUDIO

    /*
     ********************************************************************************
     * Step 2:
     * Copy / convert new fixed point ADC DMA buffers to SHARC 1 input
     * Copy / convert DAC buffer to fixed point DAC DMA output buffers
     ********************************************************************************
     */

    /*
     * Copy audio data to and from our fixed-point DMA buffers.  Convert to and from floating
     * point in the process and clip audio if needed.  Use the current DMA pointers to determine
     * which pair of buffers is not presently being transmitted / received.
     */
    if (    (uint32_t)sport_dma_cfg->dma_descriptor_rx_0_list.Next_Desc !=
            (*sport_dma_cfg->pREG_DMA_RX_DSCPTR_NXT)
            )  {

        audioflow_float_to_fixed(adau1761_audiochannels_out, sport0_dma_tx_0_buffer, AUDIO_CHANNELS * AUDIO_BLOCK_SIZE);
        audioflow_fixed_to_float(sport0_dma_rx_0_buffer, adau1761_audiochannels_in,  AUDIO_CHANNELS * AUDIO_BLOCK_SIZE);

        #if (ENABLE_A2B)
        audioflow_float_to_fixed(a2b_audiochannels_out, sport1_dma_tx_0_buffer, AUDIO_CHANNELS * AUDIO_BLOCK_SIZE);
        audioflow_fixed_to_float(sport1_dma_rx_0_buffer, a2b_audiochannels_in,  AUDIO_CHANNELS * AUDIO_BLOCK_SIZE);
        #endif

        // Audio data to/from SPDIF
        audioflow_float_to_fixed(spdif_audiochannels_out, sport2_dma_tx_0_buffer, SPDIF_DMA_CHANNELS * AUDIO_BLOCK_SIZE);
        audioflow_fixed_to_float(sport2_dma_rx_0_buffer, spdif_audiochannels_in, SPDIF_DMA_CHANNELS * AUDIO_BLOCK_SIZE);
    }
    else
    {
        audioflow_float_to_fixed(adau1761_audiochannels_out, sport0_dma_tx_1_buffer, AUDIO_CHANNELS * AUDIO_BLOCK_SIZE);
        audioflow_fixed_to_float(sport0_dma_rx_1_buffer, adau1761_audiochannels_in,  AUDIO_CHANNELS * AUDIO_BLOCK_SIZE);

        #if (ENABLE_A2B)
        audioflow_float_to_fixed(a2b_audiochannels_out, sport1_dma_tx_1_buffer, AUDIO_CHANNELS * AUDIO_BLOCK_SIZE);
        audioflow_fixed_to_float(sport1_dma_rx_1_buffer, a2b_audiochannels_in,  AUDIO_CHANNELS * AUDIO_BLOCK_SIZE);
        #endif

        // Audio data to/from SPDIF
        audioflow_float_to_fixed(spdif_audiochannels_out, sport2_dma_tx_1_buffer, SPDIF_DMA_CHANNELS * AUDIO_BLOCK_SIZE);
        audioflow_fixed_to_float(sport2_dma_rx_1_buffer, spdif_audiochannels_in, SPDIF_DMA_CHANNELS * AUDIO_BLOCK_SIZE);
    }

    #if (USE_BOTH_CORES_TO_PROCESS_AUDIO)
    /*
     ********************************************************************************
     * STEP 3:
     * Kick off transfer of output audio from SHARC 2 to buffer on SHARC 1.
     * When this DMA completes, it will kick of an interrupt on SHARC 2.
     ********************************************************************************
     */

    // DMA Transfer from SH2 Out to SH1 In
    void *sharc_core2_src_addr  = (void *)((uint32_t)multicore_data->sharc_core2_audio_out + 0x28800000);
    void *sharc_core1_dest_addr = (void *)((uint32_t)multicore_data->sharc_core1_audio_in  + 0x28000000);

    // Source
    *pREG_DMA18_ADDRSTART = sharc_core2_src_addr;
    *pREG_DMA18_XCNT      = AUDIO_BLOCK_SIZE * AUDIO_CHANNELS;
    *pREG_DMA18_XMOD      = 4;

    // Dest
    *pREG_DMA19_ADDRSTART = sharc_core1_dest_addr;
    *pREG_DMA19_XCNT      = AUDIO_BLOCK_SIZE * AUDIO_CHANNELS;
    *pREG_DMA19_XMOD      = 4;

    // Kick off transfer
    // TODO - further optimize these settings
    *pREG_DMA18_CFG     // = 0x201;
        = BITM_DMA_CFG_EN |
          (0x2 << BITP_DMA_CFG_MSIZE) |
          0;

    *pREG_DMA19_CFG     // = 0x100203;
        = BITM_DMA_CFG_EN |                                  // Enable DMA
          BITM_DMA_CFG_WNR |                                 // Write mode
          (0x2 << BITP_DMA_CFG_MSIZE) |
          (0x1 << BITP_DMA_CFG_INT) |                        // Generate an interrupt when complete
          0;

    /*
     ********************************************************************************
     * STEP 4:
     * Before we kick off audio processing on SHARC 1, make sure we have copied
     * the full output buffer from SHARC 1 to SHARC 2 (from step 1).  We don't want
     * to begin processing data in SHARC 1 until all of the processed data has been
     * moved out.
     ********************************************************************************
     */
    while (!*pREG_DMA9_STAT & 0x1) { }
    #endif    // USE_BOTH_CORES_TO_PROCESS_AUDIO

    // Detect and handle the "frame dropped" event
    if (!last_audio_frame_completed) {

        // Make a call to the callback
        processaudio_mips_overflow();

        // Zero output buffers so we get silence instead of repeated audio
        for (i = 0; i < AUDIO_CHANNELS * AUDIO_BLOCK_SIZE; i++) {
			#if (USE_BOTH_CORES_TO_PROCESS_AUDIO)
            	audiochannels_to_sharc_core2[i] = 0;
		    #endif
            adau1761_audiochannels_out[i] = 0;
        }

        // Update dropped audio frame counter
        multicore_data->sharc_core1_dropped_audio_frames++;

        // Don't trigger the software interrupt for audio processing on this block
        return;
    }
    else {
        // Set to false as we begin processing this new audio frame
        last_audio_frame_completed = false;

        // Raise lower priority interrupt to kick off AudioFramework_AudioCallback_Handler
        *pREG_SEC0_RAISE = INTR_TRU0_INT4;
    }
}

/**
 * @brief      SHARC Core 1 Audio callback handler
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
    //*pREG_SEC0_END = INTR_SOFT7;
    *pREG_SEC0_END = INTR_TRU0_INT4;

    // If we're using Faust, run the Faust audio processing before our callback
    #if (defined(USE_FAUST_ALGORITHM_CORE1) && USE_FAUST_ALGORITHM_CORE1)
    Faust_audio_processing();
    #endif

    // Call user audio processing
    processaudio_callback();

    // Calculate our CPU load for this SHARC core based on our cycle counter
    multicore_data->sharc_core1_cpu_load_mhz = audioflow_get_cpu_load(cycle_cntr,
                                                                      AUDIO_BLOCK_SIZE,
                                                                      CORE_CLOCK_FREQ_HZ,
                                                                      AUDIO_SAMPLE_RATE);

    if (multicore_data->sharc_core1_cpu_load_mhz > multicore_data->sharc_core1_cpu_load_mhz_peak) {
        multicore_data->sharc_core1_cpu_load_mhz_peak = multicore_data->sharc_core1_cpu_load_mhz;
    }


	#if (SAM_AUDIOPROJ_FIN_BOARD_PRESENT)
		float amplitude = 0;
		for (int i=0;i<AUDIO_BLOCK_SIZE;i++) {
			amplitude += fabs(audiochannel_0_left_in[i]);
			amplitude += fabs(audiochannel_0_right_in[i]);
		}

		amplitude *= (1.0/AUDIO_BLOCK_SIZE);
		multicore_data->audio_in_amplitude = 20.0*log10(amplitude);
	#endif


    // Increment our counter containing number of blocks processed
    audio_blocks_processed_count++;

    // Set flag that last audio frame has completed
    last_audio_frame_completed = true;
}

/**
 * @brief      SHARC Core 1 audio framework initialization
 *
 * As noted in the comments at the top of this file, SHARC Core 1 is
 * responsible for getting the DMA configured to move audio data
 * to and from the SPORTs.
 *
 * If we're using a multi-core framework, this routine also waits for
 * SHARC Core 2 to report that it has initialized everything so
 * we don't start passing audio to core 2 prematurely.
 *
 */
void audioframework_initialize() {

    // Initialize multicore struct data

    // Clear peak MIPS
    multicore_data->sharc_core1_cpu_load_mhz_peak = 0;

    // Clear dropped frame counter
    multicore_data->sharc_core1_dropped_audio_frames = 0;

    // If we're using Faust on either core, initialize the Faust engine
    #if (USE_FAUST_ALGORITHM_CORE1)
    	faust_initialize();
    #endif

    // Initialize peripherals and DMA to configure audio data I/O flow
    audioflow_init_sport_dma(&SPR0_ADAU1761_8CH_Config);
    audioflow_init_sport_dma(&SPR1_A2B_8CH_Config);
    audioflow_init_sport_dma(&SPR2_spdif_2CH_Config);

    // Set up interrupt handler for our audio callback (set at a lower interrupt priority)
    adi_int_InstallHandler(INTR_TRU0_INT4, (ADI_INT_HANDLER_PTR)audioframework_audiocallback_handler, NULL, true);

    #if (USE_BOTH_CORES_TO_PROCESS_AUDIO)
		// Set pointers in our shared memory structure
		multicore_data->sharc_core1_audio_out = audiochannels_to_sharc_core2;
		multicore_data->sharc_core1_audio_in  = audiochannels_from_sharc_core2;
    #endif

    // Let everyone know that SHARC Core 1 is ready
    multicore_data->sharc_core1_ready_for_audio = true;

    #if (USE_BOTH_CORES_TO_PROCESS_AUDIO)
		// Wait for SHARC Core 2 to let us know it's ready
		while (!multicore_data->sharc_core2_ready_for_audio) {}
    #endif
}

/**
 * @brief      SHARC Core 1 kick off audio processing
 *
 * This routine enables the DMA and SPORTs (which were configured as part)
 * of AudioFramework_Initialize().
 *
 */
void audioframework_start() {

    // Enable RX and TX DMAs for SPORT0, SPORT1 and SPORT2
    SPORT_DMA_ENABLE(0, 1);
    SPORT_DMA_ENABLE(1, 1);
    SPORT_DMA_ENABLE(2, 1);
    SPORT_DMA_ENABLE(3, 1);
    SPORT_DMA_ENABLE(4, 1);
    SPORT_DMA_ENABLE(5, 1);

    // Enable SPORT0, SPORT1 and SPORT2
    SPORT_ENABLE(2, A, 0, 1);
    SPORT_ENABLE(2, B, 0, 1);
    SPORT_ENABLE(1, A, 0, 1);
    SPORT_ENABLE(1, B, 0, 1);
    SPORT_ENABLE(0, A, 0, 1);
    SPORT_ENABLE(0, B, 0, 1);
}

#endif  // AUDIO_FRAMEWORK_8CH_SAM_AND_AUDIOPROJ_FIN
int audio_framework_8ch_sam_and_audioproj_fin = 1;
